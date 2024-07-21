#include <iostream>
#include <Windows.h>
#include <shobjidl.h>
#include <vector>

using namespace std;

void CopyFile(HANDLE hSrc, HANDLE hDst) {
    //буферы для чтения/записи
    vector<byte> bufRead(4096);
    vector<byte> bufWrite(bufRead.size());
    //позиции текущего чтения/записи
    LONGLONG posRead = 0;
    LONGLONG posWrite = 0;
    //сколько байт было прочитано/записано
    DWORD byteRead, byteWritten;
    //объекты-события для оповещения о конце операции
    HANDLE hRead = CreateEventA(NULL, FALSE, FALSE, NULL);
    HANDLE hWrite = CreateEventA(NULL, FALSE, FALSE, NULL);
    HANDLE events[2] = { hRead, hWrite };
    //структуры OVERLAPPED для ассинхронных операций
    OVERLAPPED ovRead, ovWrite;
    //читаем первый блок данных из файла
    ZeroMemory(&ovRead, sizeof(ovRead));
    ovRead.Offset = posRead & 0xFFFFFFFF; //((1 << 32)-1) //младщие 4 байта
    ovRead.OffsetHigh = posRead >> 32; //старшие 4 байта
    ovRead.hEvent = hRead; //событие сигнал завершения операции
    //планируем чтение
    ReadFile(
        hSrc, //откуда читаем
        bufRead.data(), //куда читаем
        bufRead.size(), //сколько читаем
        NULL, //потом узнаем сколько байт прочитано
        &ovRead //используемая структура OVERLAPPED
    );
    //ReadFile() сразу вернет управление. надо ждать
    WaitForSingleObject(hRead, INFINITE); //ждем без таймаута
    //получаем результат операции
    GetOverlappedResult(
        hSrc, //над каким файлом выполнялась операция
        &ovRead, //о какой операции идет речь
        &byteRead, //сколько байт переслано по ходу операции
        FALSE //не ждем конца операции
    );
    posRead += byteRead;
    while (byteRead > 0) {
        //меняем буфферы ролями
        bufRead.swap(bufWrite);
        //планируем чтение
        ZeroMemory(&ovRead, sizeof(ovRead));
        ovRead.Offset = posRead & 0xFFFFFFFF; //((1 << 32)-1) //младщие 4 байта
        ovRead.OffsetHigh = posRead >> 32; //старшие 4 байта
        ovRead.hEvent = hRead; //событие сигнал завершения операции
        //планируем чтение
        ReadFile(
            hSrc, //откуда читаем
            bufRead.data(), //куда читаем
            bufRead.size(), //сколько читаем
            NULL, //потом узнаем сколько байт прочитано
            &ovRead //используемая структура OVERLAPPED
        );
        //планируем запись
        ZeroMemory(&ovWrite, sizeof(ovWrite));
        ovWrite.Offset = posWrite & 0xFFFFFFFF; //((1 << 32)-1) //младшие 4 байта
        ovWrite.OffsetHigh = posWrite >> 32; //старшие 4 байта
        ovWrite.hEvent = hWrite; //событие сигнал завершения операции
        WriteFile(
            hDst, //файл приемник (куда пишем)
            bufWrite.data(), //что пишем
            byteRead, //пишем сколько, сколько почитали
            NULL, //потом узнаем, сколько байт записали
            &ovWrite //используемая структура OVERLAPPED
        );
        WaitForMultipleObjects(
            2, //сколько событий ждет
            events, //адрес массива событий
            TRUE, //ждем ВСЕ события, а не любое
            INFINITE //без таймаута
        );
        GetOverlappedResult(hDst, &ovWrite, &byteWritten, FALSE);
        if (byteRead == byteWritten) {
            cout << "Переписано " << byteWritten << " байт.\n";
        }
        else {
            cout << "Ошибка записи -> переписано " << byteWritten << " из " << byteRead << " байт\n";
            break;
        }
        posWrite += byteWritten; //смещаем позицию записи
        GetOverlappedResult(hSrc, &ovRead, &byteRead, FALSE);
        posRead += byteRead; //смещаем позицию чтения
    }
    CloseHandle(hRead);
    CloseHandle(hWrite);
}

bool open_dlg(wstring& filepath) {
    bool success = false;

    filepath.clear();
    CLSID class_id = CLSID_FileOpenDialog;
    IID interface_id = IID_IFileOpenDialog;
    IFileDialog* pDialog; //обертка в виде класса c++
    HRESULT res = CoCreateInstance(
        class_id, //экземпляр какого класса
        NULL, //экземпляр будет сам по себе
        CLSCTX_ALL, //контекст использования
        interface_id, //какой интерфейс нужен
        (void**)&pDialog //куда поместили ссылку
    );
    if (SUCCEEDED(res)) {
        pDialog->SetTitle(L"исходный файл");
        if (SUCCEEDED(pDialog->Show(NULL))) {
            //пользователь выбрал файл
            IShellItem* pItem;
            res = pDialog->GetResult(&pItem);
            if (SUCCEEDED(res)) {
                //получили выбранный файл
                PWSTR path;
                //метод сам выделит память для строки
                res = pItem->GetDisplayName(
                    SIGDN_FILESYSPATH, //путь к файлу
                    &path //куда поместит адрес строки
                );
                if (SUCCEEDED(res)) {
                    //путь получен
                    //объект wstring копирует строку
                    filepath = path;
                    //освобождаем память
                    CoTaskMemFree(path);
                    success = true;
                }
                pItem->Release();
            }
        }
        pDialog->Release();
    }

    return success;
}

bool save_dlg(wstring& filepath) {
    bool success = false;

    filepath.clear();
    CLSID class_id = CLSID_FileSaveDialog;
    IID interface_id = IID_IFileSaveDialog;
    IFileDialog* pDialog; //обертка в виде класса c++
    HRESULT res = CoCreateInstance(
        class_id, //экземпляр какого класса
        NULL, //экземпляр будет сам по себе
        CLSCTX_ALL, //контекст использования
        interface_id, //какой интерфейс нужен
        (void**)&pDialog //куда поместили ссылку
    );
    if (SUCCEEDED(res)) {
        pDialog->SetTitle(L"текущий файл");
        if (SUCCEEDED(pDialog->Show(NULL))) {
            //пользователь выбрал файл
            IShellItem* pItem;
            res = pDialog->GetResult(&pItem);
            if (SUCCEEDED(res)) {
                //получили выбранный файл
                PWSTR path;
                //метод сам выделит память для строки
                res = pItem->GetDisplayName(
                    SIGDN_FILESYSPATH, //путь к файлу
                    &path //куда поместит адрес строки
                );
                if (SUCCEEDED(res)) {
                    //путь получен
                    //объект wstring копирует строку
                    filepath = path;

                    //освобождаем память
                    CoTaskMemFree(path);
                    success = true;
                }
                pItem->Release();
            }
        }
        pDialog->Release();
    }

    return success;
}

int main()
{
    setlocale(0, "RU-ru");

    HRESULT res = CoInitializeEx(
        NULL,
        COINIT_APARTMENTTHREADED
    );
    if (FAILED(res)) {
        wcout << L"подсистема COM недоступна\n";
        return 1;
    }
    wcout << L"подсистема COM готова к работе\n";

    wstring path;
    wstring save_path;
    HANDLE hSrc;
    HANDLE hDst;
    if (open_dlg(path)) {
        wcout << L"выбран файл: " << path.c_str() << endl;

        hSrc = CreateFileW(
            path.c_str(), //какой файл открываем
            GENERIC_READ, //для чтения
            FILE_SHARE_READ, //другие могут тоже читать
            NULL, //атрибуты безопасности по умолчанию
            OPEN_EXISTING, //открыть сущ. файл
            FILE_FLAG_OVERLAPPED, //ассинхронный режим
            NULL //игнорирование параметра
        );

        if (save_dlg(save_path)) {
            wcout << L"куда сохранить: " << save_path.c_str() << endl;
            hDst = CreateFileW(
                save_path.c_str(), //имя файла
                GENERIC_WRITE, //для записи
                0,
                NULL, //настройка безопасности файла по умолчанию
                CREATE_ALWAYS,
                FILE_FLAG_OVERLAPPED, //ассинхронный режим
                NULL //игнорирование параметра
            );
            if (hDst == INVALID_HANDLE_VALUE) {
                DWORD err = GetLastError();
                cout << "нельзя открыть файл " << save_path.c_str() << endl;
                cout << "Ошибка: " << err << endl;
            }
            else {
                CopyFile(hSrc, hDst);
            }
            CloseHandle(hDst);
        }
        CloseHandle(hSrc);
    }
    else {
        wcout << L"не удалось узнать путь\n";
    }
    CoUninitialize();

    return 0;
}