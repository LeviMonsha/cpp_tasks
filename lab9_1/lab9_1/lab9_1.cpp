#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

using namespace std;

bool is(DWORD type, DWORD mask) {
    return (type & mask) == mask;
}

void show_service_info(ENUM_SERVICE_STATUS_PROCESSA info) {
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    cout << info.lpServiceName << endl;
    cout << info.lpDisplayName << endl;

    switch (info.ServiceStatusProcess.dwCurrentState) {
    case SERVICE_RUNNING:
        cout << "[RUNNING]";
        break;
    case SERVICE_STOPPED:
        cout << "[STOPPED]";
        break;
    case SERVICE_PAUSED:
        cout << "[PAUSED]";
        break;
    default:
        cout << "[UNKNOWN]";
    }
    cout << endl;

    DWORD type = info.ServiceStatusProcess.dwServiceType;
    if (is(type, SERVICE_WIN32)) {
        cout << "[SERVICE]";
    }
    else if (is(type, SERVICE_WIN32_OWN_PROCESS)) {
        cout << "[SERVICE][OWN PROCESS]";
    }
    else if (is(type, SERVICE_WIN32_SHARE_PROCESS)) {
        cout << "[SERVICE][SHARE PROCESS]";
    }
    cout << endl;

    if (is(type, SERVICE_DRIVER)) {
        cout << "[DRIVER]";
    }
    else if (type, SERVICE_KERNEL_DRIVER) {
        cout << "[KERNEL]";
    }
    else if (type, SERVICE_FILE_SYSTEM_DRIVER) {
        cout << "[DRIVER][FS]";
    }
    cout << endl;
}

int main()
{
    setlocale(0, "RU-ru");

    SC_HANDLE scm = OpenSCManagerA(
        NULL, //обращаемся к локальной базе
        NULL, // зарезервировано
        //желаемые права на доступ
        SC_MANAGER_ENUMERATE_SERVICE //перечисление
    );
    if (scm == NULL) {
        cout << "нет доступа к службам\n";
        return 1;
    }
    cout << "менеджер служб доступен\n";

    vector<byte> buffer(0); //буфер служб
    DWORD bytesNeeded = 0; //желаемый размер буфера
    DWORD servicesCount = 0; //число служб
    BOOL success = FALSE;
    DWORD error = 0;
    do { //пытаемся получить список служб
        
        buffer.resize(bytesNeeded);
        ZeroMemory(buffer.data(), buffer.size());
        //опрашиваем список служб
        success = EnumServicesStatusExA(
            scm, //какую базу служб опрашиваем
            SC_ENUM_PROCESS_INFO, //уровень детализации
            SERVICE_DRIVER | SERVICE_WIN32, //какую службу ищем
            SERVICE_ACTIVE | SERVICE_INACTIVE, //в каком состоянии должна быть служба
            buffer.data(), //куда записать результат
            buffer.size(), //размер буфера
            &bytesNeeded, //желаемый размер буфера
            &servicesCount, //сколько служб прочитали
            NULL, //дескриптор продолжения - не используется
            NULL //имя группы пустое - любая служба
        );
        error = GetLastError();
        //повторяем цикл пока проблема в маленьком буфере

    } while (!success && (error == ERROR_MORE_DATA));
    if (!success) {
        cout << "ошибка перечисления служб\n";
    }
    else {
        cout << servicesCount << " служб найдено\n";
        ENUM_SERVICE_STATUS_PROCESSA* records;
        records = (ENUM_SERVICE_STATUS_PROCESSA*)buffer.data();
        for (int i = 0; i < servicesCount; i++) {
            show_service_info(records[i]);
        }
    }

    CloseServiceHandle(scm);

    return 0;
}
