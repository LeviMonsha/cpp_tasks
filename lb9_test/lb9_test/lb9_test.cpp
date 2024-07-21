#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <vector>

using namespace std;

int main()
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

    if (NULL == schSCManager)
    {
        cout << "OpenSCManager failed (%d)\n" << GetLastError();
        return 1;
    }

    SC_HANDLE schService = OpenService(schSCManager, TEXT("ServiceName"), SERVICE_QUERY_CONFIG);

    if (schService == NULL)
    {
        cout << "OpenService failed (%d)\n" << GetLastError();
        CloseServiceHandle(schSCManager);
        return 1;
    }

    BYTE lpBuffer = NULL;
    DWORD dwBytesNeeded = 0;

    QueryServiceConfig2A(
        schService, // дескриптор службы
        SERVICE_CONFIG_DESCRIPTION, // идентификатор параметра (описание)
        NULL, // указатель на буфер для получения данных (первый вызов для получения размера)
        0, // размер буфера (первый вызов для получения размера)
        &dwBytesNeeded // количество байтов, требующихся для буфера
    );

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        // Выделяем достаточный размер для буфера
        std::vector<BYTE> buffer(dwBytesNeeded);

        // Получаем информацию о конфигурации с использованием буфера
        if (QueryServiceConfig2A(
            schService, // дескриптор службы
            SERVICE_CONFIG_DESCRIPTION, // идентификатор параметра (описание)
            buffer.data(), // указатель на буфер
            dwBytesNeeded, // размер буфера
            &dwBytesNeeded // количество байтов, требующихся для буфера
        )) {
            // Приводим буфер к типу структуры SERVICE_DESCRIPTIONA
            SERVICE_DESCRIPTIONA* serviceDescription = reinterpret_cast<SERVICE_DESCRIPTIONA*>(buffer.data());

            // Теперь у вас есть структура SERVICE_DESCRIPTIONA, содержащая описание службы
        }
    }

    if (lpBuffer)
    {
        LocalFree(&lpBuffer);
    }
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return 0;
}