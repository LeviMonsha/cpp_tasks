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

void show_service_config(QUERY_SERVICE_CONFIGA cfg) {
    cout << "путь: " << cfg.lpBinaryPathName << endl;
    if (cfg.dwServiceType & SERVICE_WIN32) {
        cout << "пользователь: " << cfg.lpServiceStartName << endl;
    }
    switch (cfg.dwStartType) {
    case SERVICE_BOOT_START:
        cout << "[BOOT]";
        break;
    case SERVICE_SYSTEM_START:
        cout << "[SYSTEM]";
        break;
    case SERVICE_AUTO_START:
        cout << "[AUTO]";
        break;
    case SERVICE_DEMAND_START:
        cout << "[ON DEMAND]";
        break;
    case SERVICE_DISABLED:
        cout << "[DISABLED]";
        break;
    }
    cout << endl;
    switch (cfg.dwErrorControl) {
    case SERVICE_ERROR_CRITICAL:
        cout << "[CRITICAL]";
        break;
    case SERVICE_ERROR_SEVERE:
        cout << "[SEVERE]";
        break;
    case SERVICE_ERROR_NORMAL:
        cout << "[NORMAL]";
        break;
    case SERVICE_ERROR_IGNORE:
        cout << "[IGNORE]";
        break;
    }
    cout << endl;
}

void show_description_config(vector<BYTE> description) {
    SERVICE_DESCRIPTIONA* serviceDescription = (SERVICE_DESCRIPTIONA*)description.data();
    if (serviceDescription->lpDescription != NULL) cout << "описание службы:\n" << serviceDescription->lpDescription << endl;
    else cout << "нет описания службы\n";
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
    BOOL success_description = FALSE;
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

            SC_HANDLE service = OpenServiceA(
                scm, //где прописана эта служба
                records[i].lpServiceName, //имя службы
                SERVICE_QUERY_CONFIG //запрос конфига
            );
            if (service == NULL) {
                cout << "нет данных\n";
                continue;
            }
            vector<byte> config;
            DWORD cfgBytesNeeded = 0;
            
            do {
                config.resize(cfgBytesNeeded);
                success = QueryServiceConfigA(
                    service, //дескриптор службы
                    (QUERY_SERVICE_CONFIGA*)config.data(),
                    config.size(),
                    &cfgBytesNeeded //желаемый размер
                );
                error = GetLastError();

            } while (!success && (error == ERROR_INSUFFICIENT_BUFFER));

            if (!success) {
                cout << "конфиг службы недоступен\n";
            }
            else {
                QUERY_SERVICE_CONFIGA* cfg;
                cfg = (QUERY_SERVICE_CONFIGA*)config.data();
                show_service_config(*cfg);
            }

            DWORD bytesNeeded_description = 0;
            QueryServiceConfig2A(
                service,
                SERVICE_CONFIG_DESCRIPTION,
                NULL,
                0,
                &bytesNeeded_description
            );

            error = GetLastError();
            if (error == ERROR_INSUFFICIENT_BUFFER) {
                vector<BYTE> buffer_description(bytesNeeded_description);
                success = QueryServiceConfig2A(
                    service,
                    SERVICE_CONFIG_DESCRIPTION,
                    buffer_description.data(),
                    bytesNeeded_description,
                    &bytesNeeded_description
                );
                if (success) {
                    show_description_config(buffer_description);
                }
            }

            CloseServiceHandle(service);
        }
    }

    CloseServiceHandle(scm);

    return 0;
}

//зачем нужны службы
//драйверы другие приложения которые не взаимодействуют с пользователем

//разница между драйверами на 32 и 64 использ 2 уровня кода (3 кольцо защиты)
//многие инструкции не доступны в ядре
//драйвер ядра работает в кольце защиты
