#include <iostream>
#include <Windows.h>
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
            cout << "Rewrite " << byteWritten << " byte.\n";
        }
        else {
            cout << "error writing. rewrite " << byteWritten << " out of " << byteRead << " byte\n";
            break;
        }
        posWrite += byteWritten; //смещаем позицию записи
        GetOverlappedResult(hSrc, &ovRead, &byteRead, FALSE);
        posRead += byteRead; //смещаем позицию чтения
    }
    CloseHandle(hRead);
    CloseHandle(hWrite);
}

int main(int argc, char* argv[])
{
    setlocale(0, "");

    if (argc != 2) {
        cout << "run example\n" << argv[0] << "filename.txt\n";
        return 0;
    }

    const char* SrcName = "lab4_2.cpp";
    const char* DstName = "copy.txt";

    HANDLE hSrc = CreateFileA(
        SrcName, //какой файл открываем
        GENERIC_READ, //для чтения
        FILE_SHARE_READ, //другие могут тоже читать
        NULL, //атрибуты безопасности по умолчанию
        OPEN_EXISTING, //открыть сущ. файл
        FILE_FLAG_OVERLAPPED, //ассинхронный режим
        NULL //игнорирование параметра
    );

    if (hSrc == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        cout << "cant open " << SrcName << endl;
        cout << "error code: " << err << endl;
    }
    else {
        cout << "open for read " << SrcName << endl;
        //работа с файлом
        HANDLE hDst = CreateFileA(
            DstName, //имя файла
            GENERIC_WRITE, //для записи
            0,
            NULL, //настройка безопасности файла по умолчанию
            CREATE_ALWAYS,
            FILE_FLAG_OVERLAPPED, //ассинхронный режим
            NULL //игнорирование параметра
        );
        if (hDst == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            cout << "cant open " << SrcName << endl;
            cout << "error code: " << err << endl;
        }
        else {
            cout << "open for rewrite " << DstName << endl;
            CopyFile(hSrc, hDst);
            CloseHandle(hDst);
        }
        CloseHandle(hSrc);
    }
    return 0;
}