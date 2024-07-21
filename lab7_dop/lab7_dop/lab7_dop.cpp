#include <iostream>
#include <vector>
#include <Windows.h>
#include <string>
#include <iomanip>
#include <Psapi.h>

using namespace std;

void enumerate_modules(vector<HMODULE>& list) {
    //дескриптор процесса (наш процесс)
    HANDLE me = GetCurrentProcess();
    //требуемый размер массива дескриптора
    DWORD needed_size = 0;
    //текущий размер массива
    DWORD actual_size = 0;
    do {
        actual_size = list.size() * sizeof(HMODULE);
        EnumProcessModules(
            me, //в каком процессе ищем модули
            list.data(), //куда записать дескрипторы
            actual_size, //размер буфера в памяти
            &needed_size //желаемый размер массива
        );
        list.resize(needed_size / sizeof(HMODULE));
    } while (needed_size > actual_size);
}

string get_module_name(HMODULE module) {
    HANDLE me = GetCurrentProcess();
    string buffer(FILENAME_MAX, 0);
    DWORD actual_len = GetModuleBaseNameA(
        me, //дескриптор процесса
        module, //дескриптор модуля
        &buffer[0], //куда поместить имя модуля
        buffer.size() //размер буфера для имени
    );
    if (actual_len == 0) return "";
    return buffer.substr(0, actual_len);
}

void show_info(HMODULE moduleName, MEMORY_BASIC_INFORMATION info) {
    cout << setw(20) << setfill(' ') << get_module_name(moduleName) << "\t";
    cout << hex << setw(8) << setfill('0') << info.BaseAddress << " ";
    cout << dec << setw(8) << setfill(' ') << info.RegionSize << " ";

    switch (info.State) {
    case MEM_FREE:
        cout << setw(8) << "free"; break;
    case MEM_COMMIT:
        cout << setw(8) << "commit"; break;
    case MEM_RESERVE:
        cout << setw(8) << "reserve"; break;
    default: cout << setw(8) << "?";
    }
    switch (info.Type) {
    case MEM_PRIVATE:
        cout << setw(8) << "PRIVATE"; break;
    case MEM_MAPPED:
        cout << setw(8) << "MAPPED"; break;
    case MEM_IMAGE:
        cout << setw(8) << "IMAGE"; break;
    case 0:
        cout << setw(8) << "FREE"; break;
    default: cout << setw(8) << "?";
    }
    switch (info.Protect) {
    case PAGE_EXECUTE:
        cout << setw(12) << "EXEC"; break;
    case PAGE_EXECUTE_READ:
        cout << setw(12) << "EXEC-READ"; break;
    case PAGE_EXECUTE_READWRITE:
        cout << setw(12) << "EXEC-READ-WRITE"; break;
    case PAGE_NOACCESS:
        cout << setw(12) << "NO ACCESS"; break;
    case PAGE_READONLY:
        cout << setw(12) << "READONLY"; break;
    case PAGE_READWRITE:
        cout << setw(12) << "READ-WRITE"; break;
    case PAGE_WRITECOPY:
        cout << setw(12) << "WRITECOPY"; break;
    default: cout << setw(12) << "?";
    }
    cout << endl;
}

void print_memory_map(MODULEINFO moduleInfo) {
    vector<HMODULE> modules;
    enumerate_modules(modules);
    MEMORY_BASIC_INFORMATION infoMemory;
    BYTE* addr = (BYTE*)moduleInfo.lpBaseOfDll;
    for (size_t i = 0; i < modules.size(); i++) {
        if (!VirtualQuery(addr, &infoMemory, sizeof(infoMemory))) return;
        show_info(modules[i], infoMemory);
        addr += infoMemory.RegionSize;
    }
}

int main()
{
    setlocale(0, "RU-ru");

    MODULEINFO moduleInfo;
    HMODULE hModule = GetModuleHandle(NULL);
    if (!hModule) return 1;
    else {
        GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo));
        cout << "lpBaseOfDll: " << moduleInfo.lpBaseOfDll << endl;
        cout << "SizeOfImage: " << moduleInfo.SizeOfImage << endl;

        cout << "---- Memory Map ----" << endl;
        cout << setw(16) << "Module" 
            << setw(16) << "Address" 
            << setw(16) << "Size" 
            << setw(8) << "State" 
            << setw(8) << "Type" 
            << setw(12) << "Protect" << endl;
        print_memory_map(moduleInfo);
    }

    return 0;
}