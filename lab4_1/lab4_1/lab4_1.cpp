#include <iostream>
#include <Windows.h>

using namespace std;

DWORD countLines(const char* addr, LONGLONG size) {
    if (size == 0) return 0;
    DWORD count = 1;
    for (LONGLONG i = 0; i < size; i++) {
        if (addr[i] == '\n') count++;
    }
    return count;
}

int main(int argc, char* argv[])
{
    setlocale(0, ""); //RU-ru
    /*for (int i = 0; i < argc; i++) {
        cout << "argv[" << i << "] is " << argv[i] << endl;
    }
    return 0;*/

    if (argc != 2) {
        cout << "run example\n" << argv[0] << "filename.txt\n";
        return 0;
    }

    //однобайтовая ASNI-кодировка
    const char* filename = argv[1]; //"lab4.cpp"
    //двухбайтовая кодировка UCS-2
    //const wchar_t*
    HANDLE hFile = CreateFileA(//открываем файл
        filename, //путь и имя файла
        GENERIC_READ, //доступ на чтение
        FILE_SHARE_READ, //другие программы открывают файл только на чтение
        NULL, //настройка безопасности файла по умолчанию
        OPEN_EXISTING, //открыть сущ. файл
        0, //файловые флаги по умолчанию
        NULL //игнорирование параметра
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        cout << "Fallacy" << filename << endl;
        cout << "Error code" << err << endl;
    }
    else {
        //файл открыт успешно
        cout << "file opened" << endl;

        LARGE_INTEGER size;
        if (!GetFileSizeEx(hFile, &size)) {
            cout << "can`t get file`s size" << endl;
        }
        else if (size.QuadPart > 1 * 1024 * 1024) {
            cout << "too large file`s size" << endl;
        }
        else {
            cout << "size shown" << endl;
            //подготовка объекта отображения
            HANDLE hMap = CreateFileMappingA(
                hFile, //отображаемый файл
                NULL, //атрибут без - по умолч.
                PAGE_READONLY, //только для чтения
                size.HighPart, size.LowPart, //макс. размер
                NULL //объект безымянный (доступ только для нас)
            );
            if (hMap == NULL) cout << "presented object wasnt created" << endl;
            else {
                cout << "file created" << endl;

                void* addr = MapViewOfFile(
                    hMap, //объект отображения
                    FILE_MAP_READ, //доступ только для чтения
                    0, 0, //смещение от начала файла
                    (SIZE_T)size.QuadPart //размер области
                );
                if (addr == NULL) cout << "file can`t be present" << endl;
                else {
                    cout << "file present" << endl;
                    DWORD lines = countLines((const char*)addr, size.QuadPart);
                    cout << "file has " << lines << endl;
                    UnmapViewOfFile(addr);
                }
                CloseHandle(hMap);
            }
        }
        CloseHandle(hFile);
    }
}