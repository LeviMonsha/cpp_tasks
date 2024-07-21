#include <iostream>
#include <vector>
#include <Windows.h>

using namespace std;

HANDLE cout_mutex;

int count_lines(const char* filename)
{
    // считает число строк в файле
    // ничего не выводит в cout
    // возвращает -1 при ошибке

    // буфер для чтения/записи
    vector<byte> buf(1024 * 1024 * 4);
    // сколько байт было прочитано или записано
    DWORD bytesRead, bytesWritten;

    HANDLE hSrc = CreateFileA(
        filename,	
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,			
        OPEN_EXISTING,	
        0,			
        NULL
    );

    if (hSrc != INVALID_HANDLE_VALUE)
    {
        int lines = 1;

        ReadFile(
            hSrc,	
            buf.data(),	
            buf.size(),		
            &bytesRead,		
            NULL			 
        );
        for (size_t i = 0; i < bytesRead; i++)
            if (buf[i] == '\n')
            {
                lines++;
                Sleep(100);
            }

        return lines;
    }

    return -1;
}

DWORD WINAPI ThreadBody(LPVOID param) {
    const char* filename = (const char*)param;
    WaitForSingleObject(cout_mutex, INFINITE);

    cout << "обрабатываем файл " << filename << endl;

    ReleaseMutex(cout_mutex);

    int lines = count_lines(filename);
    if (lines == -1)
        cout << filename << ": ошибка\n";
    else 
        cout << filename << ": " << lines << " строк\n";

    ReleaseMutex(cout_mutex);
    return 0;
}

int main(int argc, char*argv[])
{
    setlocale(0, "RU-ru");
    if (argc < 2) {
        cout << "usage: " << argv[0] << " file1.txt[file2.txt [...]]\n";
        return 1;

    }
    cout_mutex = CreateMutexA(
        NULL, //атрибут безопасности
        FALSE, //не захватываем мьютекс сразу
        NULL //мьютекс анонимный
    );
    vector<HANDLE> hThreads;
    for (int i = 1; i < argc; i++) {

        HANDLE hThread = CreateThread(
            NULL, //атрибут безопасности по умолч
            4096, //размер стека потока в байтах
            ThreadBody, //адрес тела потока
            argv[i], //параметр потока
            0, //флаги запуска по умолч
            NULL //не нужен общесистемный ID потока
        );
        hThreads.push_back(hThread);
        WaitForSingleObject(cout_mutex, INFINITE);

        cout << "поток #" << i << " запущен\n";

        ReleaseMutex(cout_mutex);
    }

    vector<const char*> spinner = {
        "\\", "|", "\"", "-"
    };
    size_t spinner_pos = 0;

    while (WaitForMultipleObjects(
        hThreads.size(), //сколько объектов ждем
        hThreads.data(), //адрес массива объектов
        TRUE, //ждем все объекты
        100 //таймаут
    ) == WAIT_TIMEOUT) {
        WaitForSingleObject(cout_mutex, INFINITE);
        cout << spinner[spinner_pos] << "\x08";
        flush(cout);
        ReleaseMutex(cout_mutex);
        spinner_pos = (spinner_pos + 1) % spinner.size();

    }
    for (int i = 0; i < hThreads.size(); i++) {
        CloseHandle(hThreads[i]);
    }

    cout << "завершение\n";
    CloseHandle(cout_mutex);
    return 0;
}