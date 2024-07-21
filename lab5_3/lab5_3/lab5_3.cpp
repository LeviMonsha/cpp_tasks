#include <iostream>
#include <WS2tcpip.h>
#include <wininet.h>
#include <Windows.h>
#include <string>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wininet.lib")

bool split_url(
    const char* url, //разбираемая ссылка
    string& host, //адрес узла в ссылке
    string& port, //номер порта в ссылке
    string& path //запрашиваемый на сайте путь
)
{
    //http://узел[:port][way][?params]
    //выделяем память под части URL
    host.resize(4096, 0);
    port.resize(4096, 0);
    path.resize(4096, 0);
    string extra(4096, 0);
    //описываем структуру
    URL_COMPONENTSA parts;
    ZeroMemory(&parts, sizeof(parts));
    parts.dwStructSize = sizeof(parts);
    //хотим узнать адрес узла
    parts.lpszHostName = &host[0];
    parts.dwHostNameLength = host.size();
    // хотим узнать запрашиваемый путь
    parts.lpszUrlPath = &path[0];
    parts.dwUrlPathLength = path.size();
    //хотим узнать параметры запроса
    parts.lpszExtraInfo = &extra[0];
    parts.dwExtraInfoLength = extra.size();
    //разбираем ссылку
    int err = InternetCrackUrlA(
        url, //ссылка
        0, //строка ссылки - нульзавершенная
        0, //флагов нет
        &parts //куда поместить результат
    );
    if (err == 0) return false;
    host.resize(parts.dwHostNameLength);
    path.resize(parts.dwUrlPathLength);
    extra.resize(parts.dwExtraInfoLength);
    path.append(extra);
    if (path.empty()) path = "/";
    port = to_string(parts.nPort);
    return true;
}

void download_file(
    SOCKET server, // откуда качаем
    HANDLE hFile, //куда качаем
    const char* host, //имя сервера
    const char* path //запрашиваемый путь
)
{
    //формируем HTTP запрос
    string buffer = "GET ";
    buffer.append(path);
    buffer.append(" HTTP/1.0\r\n");
    //заголовок Host (какой сайт запрашиваем)
    buffer.append("Host: ");
    buffer.append(host);
    buffer.append("\r\n");
    //конец заголовков и запроса
    buffer.append("\r\n");
    size_t pos = 0;
    while (pos < buffer.size()) {
        int sent = send(
            server, //куда отправляем
            &buffer[pos], //отправляемые файлы
            buffer.size() - pos, //сколько отправляем байт
            0 //флагов нет
        );
        if ((sent == SOCKET_ERROR) || (sent == 0)) {
            cout << "ошибка при отправке запроса\n";
            return;
        }
        else {
            cout << sent << " byte sent\n";
            pos += sent;
        }
    }
    cout << "запрос отправлен, принимаем ответ...\n";
    buffer.resize(8 * 1024, 0);
    pos = 0;
    size_t end_of_headers = string::npos; //не найдена подстрока в строке
    do {
        int rcvd = recv(//принимаем данные
            server, // из какого сокета
            &buffer[pos], //куда
            buffer.size() - pos, // сколько макс байт
            0 // флагов нет
        );
        if ((rcvd == SOCKET_ERROR) || (rcvd == 0)) {
            cout << "ошибка при приеме ответа\n";
                return;
        }
        else {
            cout << rcvd << " byte принято\n";
            pos += rcvd;
            end_of_headers = buffer.find("\r\n\r\n");
        }
    } while ((end_of_headers == string::npos) && (pos < buffer.size()));
    if (end_of_headers == string::npos) {
        cout << "сервер прислал слишком много заголовков\n";
        return;
    }
    size_t code_start = buffer.find(" ", 0) + 1;
    size_t code_end = buffer.find(" ", code_start);
    string code = buffer.substr(
        code_start,
        code_end - code_start
    );
    cout << "сервер ответил кодом " << code.c_str() << endl;
    if (code != "200") return;
    //пишем на диск начало файла, лежащего в буфере
    size_t write_pos = end_of_headers + 4;
    DWORD written = 0;
    WriteFile(
        hFile, //пишем в файл
        &buffer[write_pos], //начиная с начала тела ответа
        pos - write_pos, //до конца занятой части буфера
        &written,
        NULL
    );
    if (written != (pos - write_pos)) {
        cout << "ошибка записи в файл\n";
        return;
    }
    //дочитываем остаток файла
    do {
        int rcvd = recv(
            server,
            &buffer[0],
            buffer.size(),
            0
        );
        if (rcvd == SOCKET_ERROR) {
            cout << "ошибка скачивания\n";
            break;
        }
        else if (rcvd == 0) {
            cout << "скачивание успешно завершено\n";
            break;
        }
        else {
            WriteFile(hFile,
                &buffer[0],
                rcvd,
                &written,
                NULL
            );
            if (written != rcvd) {
                cout << "ошибка при записи\n";
                break;
            }
            cout << "записано " << written << " byte\n";
        }
    } while (true);
}

int main()
{
    setlocale(0, "RU-ru");

    //const char* urln = "http://some.site:88/path?params";
    const char* url = "http://neverssl.com";
    const char* filename = "result.txt";
    string host, port, path;
    if (!split_url(url, host, port, path)) {
        cout << "incorrect link: " << url << endl;
        return 1;
    }
    cout << "server: " << host.c_str() << endl
        << "port: " << port.c_str() << endl
        << "path: " << path.c_str() << endl;

    WSADATA wsa;
    if (WSAStartup(2, &wsa)) {
        cout << "не удалось задействовать Winsock2\n";
        return 1;
    }
    PADDRINFOA addr;
    int err = getaddrinfo(
        host.c_str(), //адрес узла
        port.c_str(), //номер порта
        NULL,
        &addr
    );
    if (err != 0) {
        cout << "адрес неизвестен " << host.c_str() << ":" << port.c_str() << endl;;
    }
    else {
        cout << "адрес узла получен\n";

        SOCKET server = socket(
            AF_INET, //адрес IPv4
            SOCK_STREAM, //
            IPPROTO_TCP
        );
        if (server == INVALID_SOCKET) cout << " cant create socket\n";
        else {
            cout << "socket created\n";

            err = connect(//установить соединение
                server, //какой сокет используем
                addr -> ai_addr, //куда соединяемся
                addr -> ai_addrlen
            );
            if (err != 0) cout << "cant connect to server\n";
            else {
                cout << "connection\n";

                HANDLE hFile = CreateFileA(
                    filename,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    0,
                    NULL
                );
                if (hFile == INVALID_HANDLE_VALUE) cout << "cant open file " << filename << endl;
                else {
                    download_file(server, hFile, host.c_str(), path.c_str()); //V

                    CloseHandle(hFile);
                }
                shutdown(server, SD_BOTH);
            }

            closesocket(server);
        }

        freeaddrinfo(addr);
    }
    WSACleanup();
    return 0;
}
