#include <iostream>
#include <time.h> //работа с датой
#include <WS2tcpip.h> //строго до wh //для getaddrinfo()
#include <Windows.h>

using namespace std;

#pragma comment (lib, "Ws2_32.lib") //бинарная часть WinSock

//отключаем выравнивание полей структуры по кратным адресам, 
//но запоминаем предыдущую надстройку
#pragma pack(push, 1)

struct NTPPacket {//пакет протокола NTP версии 3
    BYTE info; //версия протокола и назначение пакета
    BYTE stratum; //эшелон сервера NTP
    BYTE poll;
    BYTE precision;
    //информация о сервере
    DWORD root_delay; //матожидание ошибки сервера
    DWORD root_dispertion; //дисперсия ошибки сервера
    DWORD reference_id;
    //когда сервер последний раз сверял всое время
    DWORD reference_seconds; //число секунд с 1 янв 1900 г
    DWORD reference_fraction; //число долей сек
    //когда запрос был отправлен (по часам клиента)
    DWORD origin_seconds;
    DWORD origin_fraction;
    //когда запрос был принт (по часам сервера)
    DWORD receive_seconds;
    DWORD receive_fraction;
    //когда ответ был отправлен (по часам сервера)
    DWORD transmit_seconds;
    DWORD transmit_fraction;
};

//востанавливаем предыдущее выравнивание
#pragma pack(pop)

void PrintTime(DWORD seconds) {
    const ULONGLONG NTP_TIME_DELTA = 2208988800ull;
    //пересчитываем время с эпохи NTP в эпоху Unix
    time_t current_time = (time_t)(seconds - NTP_TIME_DELTA);
    //переводим время в структуру
    tm local_time;
    localtime_s(&local_time, &current_time);
    //формируем читаемое представление времени
    wchar_t buffer[64];
    wcsftime(
        buffer, 64, //куда поместить строку и макс длину
        L"%d %B %Y %H:%M:%S", //шаблон даты
        &local_time //откуда берем
    );
    wcout << buffer << endl;
}

int main()
{
    //setlocale(0, "Ru-ru");

    WSADATA wsadata; //сведения о библиотеке WinSock
    if (WSAStartup(2, &wsadata) != 0) {
        cout << "cant ready WinSock\n";
        return 1;
    }
    cout << "WinSock is ready to work\n";

    PADDRINFOA addr; //список адресов узла 
    int err = getaddrinfo(
        "time.windows.com", //адрес узла
        "123", //порт NTP - 123 ("ntp")
        NULL,
        &addr //куда поместить адрес начала списка адресов
    );
    if (err != 0) {
        cout << "cant take addr lace\n";
    }
    else {
        cout << "addr was taken\n";

        SOCKET sock = socket( //создаем сокет для работы
            AF_INET, //семейство адресов IPv4
            SOCK_DGRAM, //датаграммный сокет
            IPPROTO_UDP //протокол UDP
        );
        if (sock == INVALID_SOCKET) {
            cout << "cant create socket\n";
        }
        else {
            cout << "socket was created\n";

            //готовим пакет-запрос
            NTPPacket packet;
            ZeroMemory(&packet, sizeof(packet));
            //запрос клиента к серверу по версии протокола 3
            packet.info = 0b00011011; //0x1B
            //отправка пакета
            int send = sendto(
                sock, //какой сокет используем для отправки
                (const char*)&packet, //что отправляем
                sizeof(packet), //сколько байт отправляем
                0, //флагов нет, поведение по умолчанию
                //куда отправляем данные
                addr->ai_addr, //адрес узла в машинной форме
                addr->ai_addrlen //длина адреса узла
            );
            if (send == SOCKET_ERROR) {
                cout << "sending cant\n";
            }
            else if (send != sizeof(packet)) {
                cout << "not all\n";
            }
            else {
                cout << "sending all\n";
                int rcvd = recvfrom(
                    sock, //на какой сокет придут данные
                    (char*)&packet, //куда их принять
                    sizeof(packet), //сколько данных ожидаем
                    0, //флагов нет
                    //игнорируем адрес отправителя
                    NULL,
                    NULL
                );
                if (rcvd == SOCKET_ERROR) {
                    cout << "taking cant\n";
                }
                else if (rcvd != sizeof(packet)) {
                    cout << "not all\n";
                }
                else {
                    cout << "taking all\n";
                    //порядок байт сеть -> хост
                    DWORD seconds = ntohl(
                        packet.transmit_seconds
                    );
                    PrintTime(seconds);
                }
            }

            closesocket(sock);
        }

        freeaddrinfo(addr); //освобождение списка адресов
    }

    WSACleanup();
    return 0;
}