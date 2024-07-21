#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <iostream>
#include <WS2tcpip.h>
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <experimental/filesystem>

using namespace std;
#pragma comment(lib, "Ws2_32.lib")

bool working = true;
bool make_content(
    const string& path,
    stringstream& content,
    string& contenttype
)
{
    content << "<!DOCTYPE html>\n"
            << "<title>iconView</title>\n"
            << "<link rel=\"icon\" href=\"" << path << "\" type=\"image/x-icon\">";
    contenttype = "image/x-icon";
    return true;
}

void send_file(SOCKET* client, const char* nameIcon) {
    fstream file;
    file.open(nameIcon, ios_base::in | ios_base::binary);

    if (file.is_open()) {
        int sizeIcon = experimental::filesystem::file_size(nameIcon) + 1;
        char* bytes = new char[sizeIcon];
        file.read(bytes, sizeIcon);

        cout << "size: " << sizeIcon << endl;
        cout << "name: " << nameIcon << endl;
        cout << "data: " << bytes << endl;

        send(*client, to_string(sizeIcon).c_str(), 16, 0);
        send(*client, ((string)nameIcon).c_str(), 32, 0);
        send(*client, bytes, sizeIcon, 0);
    }
    else {
        cout << "error";
    }
    file.close();
}

void recv_file(SOCKET* client) {
    char file_size_str[16];
    char file_name[32];
    recv(*client, file_name, 16, 0);
    int file_size = atoi(file_size_str);
    char* bytes = new char[file_size];

    recv(*client, file_name, 32, 0);

    fstream file;
    file.open(file_name, ios_base::out | ios_base::binary);

    if (file.is_open()) {
        recv(*client, bytes, file_size, 0);
        cout << "data: " << bytes << endl;
        file.write(bytes, file_size);
    }
    else {
        cout << "error";
    }
    delete[] bytes;
    file.close();
}

void serve_client(SOCKET client) {
    string buffer(1024*8, 0);
    size_t pos = 0;
    do {
        int bytes_read = recv(
            client,
            &buffer[pos],
            buffer.size() - pos,
            0
        );
        if (bytes_read == SOCKET_ERROR) {
            cout << "ошибка приема данных\n";
            pos = 0;
            break;
        }
        else if (bytes_read == 0) {
            cout << "клиент неожиданно закрыл соединение\n";
            pos = 0;
            break;
        }
        else {
            const char* nameIcon = "favicon.ico";
            send_file(&client, nameIcon);
            cout << "принято " << bytes_read << "byte\n";
            pos += bytes_read;
        }
    } while ((pos < buffer.size()) && (buffer.find("\r\n\r\n") == string::npos));
    if (pos == 0) cout << "ошибка чтения запроса\n";
    else if (pos == buffer.size()) cout << "слишком большой запрос\n";
    else {
        cout << buffer.c_str();
        size_t path_start = buffer.find(" ", 0) + 1;
        size_t path_end = buffer.find(" ", path_start);
        string path = buffer.substr(
            path_start,
            path_end - path_start
        );
        cout << "запрошен путь " << path.c_str() << endl;
        stringstream content;
        string type;
        if (make_content(path, content, type)) {
            buffer = "HTTP/1.0 200 OK\r\n";
            buffer.append("Content-Type: ");
            buffer.append(type);
            buffer.append("\r\n");
            buffer.append("Content-Length: ");
            buffer.append(to_string(content.str().size()));
            buffer.append("\r\n");
            buffer.append("\r\n");
            buffer.append(content.str());
        }
        else {
            buffer = "HTTP/1.0 404 Not Found\r\n\r\n";
        }



        pos = 0;
        while (pos < buffer.size()) {
            int sent = send(
                client,
                &buffer[pos],
                buffer.size() - pos,
                0
            );
            if ((sent == SOCKET_ERROR) || (sent == 0)) {
                cout << "ошибка отправки\n";
                break;
            }
            else {
                cout << "отправлено " << sent << "byte\n";
                pos += sent;
            }
        }
    }
    shutdown(client, SD_BOTH);
    closesocket(client);
}

int main()
{
    setlocale(0, "Ru-ru");
    WSADATA wsa;
    if (WSAStartup(2, &wsa) != 0) {
        cout << "не удалось задействовать Winsock2\n";
        return 1;
    }
    PADDRINFOA addr;
    int err = getaddrinfo(
        "127.0.0.1",
        "8080",
        NULL,
        &addr
    );
    if (err != 0) cout << "адрес неизвестен\n";
    else {
        cout << "адрес получен\n";
        SOCKET listener = socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP
        );
        if (listener == INVALID_SOCKET) cout << "не удалось создать сокет\n";
        else {
            cout << "сокет создан\n";

            err = bind(
                listener,
                addr->ai_addr,
                addr->ai_addrlen
            );
            if (err != 0) cout << "не удалось занять адрес\n";
            else {
                cout << "адрес занят\n";
                listen(listener, 5);
                while (working) {
                    sockaddr_in clientaddr;
                    clientaddr.sin_family = AF_INET;
                    socklen_t addrlen = sizeof(clientaddr);

                    SOCKET client = accept(
                        listener,
                        (sockaddr*)&clientaddr,
                        &addrlen
                    );
                    wchar_t addrbuf[64];
                    DWORD buflen = 64;
                    WSAAddressToStringW(
                        (sockaddr*)&clientaddr,
                        addrlen,
                        NULL,
                        addrbuf,
                        &buflen
                    );
                    wcout << L"подключился клиент" << addrbuf << endl;



                    
                    serve_client(client);
                    recv_file(&client);
                }
            }

            closesocket(listener);
        }
        freeaddrinfo(addr);
    }
    WSACleanup();
    return 0;
}

/*
Запрос
GET /запрашиваемый/путь HTTP/1.1
Имя-заголовка: значение
Имя-заголовка: значение
Имя-заголовка: значение

Ответ 2-получение 3-перенаправление 4-ошибка обращения //20 ok
HTTP/1.0
Content-Type: text/html
Content-Length: 1234
Имя-заголовка: значение
Имя-заголовка: значение

тело ответа
*/