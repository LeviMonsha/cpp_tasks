#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <tlhelp32.h>

using namespace std;

vector<DWORD> get_processes_by_name(wstring exe_name) {
    vector<DWORD> found;

    //снимок процессов в сиистеме
    HANDLE hSnapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS, 0
    );
    PROCESSENTRY32W entry; //описание процесса
    ZeroMemory(&entry, sizeof(entry));
    entry.dwSize = sizeof(entry);
    //получаем первый процесс в списке
    bool success = Process32FirstW(hSnapshot, &entry);
    while (success) {
        //обрабатываем очередной процесс
        if (exe_name == entry.szExeFile) {
            found.push_back(entry.th32ProcessID);
        }
        //получаем следующий процесс
        success = Process32NextW(hSnapshot, &entry);
    }
    CloseHandle(hSnapshot);

    return found;
}

BOOL __stdcall process_window(HWND hWnd, LPARAM lParam) {
    vector<DWORD>* ids = (vector<DWORD>*)lParam;
    DWORD id;
    if (GetWindowThreadProcessId(hWnd, &id) != 0) {
        //удалось определить процесс, породивший окно
        auto found = find(ids->begin(), ids->end(), id);
        if (found != ids->end()) {
            //ID процесса владельца есть в списке
            wstring caption(4096, 0);
            GetWindowTextW(hWnd, &caption[0], caption.size());
            wcout << L"закрываем окно: " << caption.c_str() << endl;
            PostMessageW(hWnd, WM_CLOSE, 0, 0);
        }
    }

    return TRUE;
}

int main()
{
    setlocale(0, "RU-ru");

    //запрашиваем имя процесса
    wcout << L"введите имя exe-файла процесса: ";
    wstring name;
    getline(wcin, name);
    if (name.empty()) return 0;
    vector<DWORD> processes = get_processes_by_name(name);
    wcout << L"ID: ";
    for (size_t i = 0; i < processes.size(); i++) {
        wcout << processes[i] << L" ";
    }
    wcout << endl;
    EnumWindows(process_window, (LPARAM)&processes);

    return 0;
}