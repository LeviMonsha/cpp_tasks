#include <iostream>
#include <vector>
#include <Windows.h>
#include <string>
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

void list_loaded_modules(vector<string>& list) {
    vector<HMODULE> modules;
    enumerate_modules(modules);
    list.clear();
    for (size_t i = 0; i < modules.size(); i++) {
        list.push_back(get_module_name(modules[i]));
    }
}

void print_list(const vector<string>& list) {
    for (size_t i = 0; i < list.size(); i++) {
        cout << "\t" << list[i].c_str() << endl;
    }
}

int main()
{
    setlocale(0, "RU-ru");
    
    vector<string> start;
    list_loaded_modules(start);
    cout << "----Список модулей при запуске----\n";
    print_list(start);

    return 0;
}