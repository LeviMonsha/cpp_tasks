#include <iostream>
#include <vector>
#include <Windows.h>
#include <string>
#include <iterator>
#include <algorithm>
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

void find_extra(
    vector<string> larger, //вектор с большим числом элементов
    vector<string> smaller, //вектор с меньшим числом элементов
    vector<string>& diff //разница между векторами
)
{
    sort(larger.begin(), larger.end());
    sort(smaller.begin(), smaller.end());
    diff.clear();
    set_difference(
        //первый набор данных
        larger.begin(), larger.end(),
        //второй набор данных
        smaller.begin(), smaller.end(),
        //куда записывать разность
        back_inserter(diff)
    );
}

int main()
{
    setlocale(0, "RU-ru");

    vector<string> start;
    list_loaded_modules(start);
    cout << "--- Список модулей при запуске ---\n";
    print_list(start);

    HMODULE lib = LoadLibraryA("User32.dll");
    if (lib == NULL) {
        cout << "не удалось загрузить библиотеку\n";
    }
    else {
        vector<string> after_import, added;
        list_loaded_modules(after_import);
        find_extra(after_import, start, added);
        cout << "--- добавилось после импорта ---\n";
        print_list(added);

        typedef int (WINAPI* MyFuncType) (
            HWND hwnd,
            LPCSTR text,
            LPCSTR caption,
            UINT uType
        );
        MyFuncType func;
        //получаем адрес функции
        LPVOID addr = GetProcAddress(lib, "MessageBoxA");
        if (addr == NULL) {
            cout << "функция не найдена\n";
        }
        else {
            func = (MyFuncType)addr;
            func(//вызываем функция MessageBoxA
                NULL, //без привязки к окну
                "Hello world", //текст сообщения
                "test", //заголовок сообщения
                //иконка i и кнопка OK
                MB_OK | MB_ICONINFORMATION
            );
        }
        vector<string> after_call, diff_call;
        list_loaded_modules(after_call);
        find_extra(after_call, after_import, diff_call);
        cout << "--- добавилось после вызова ---\n";
        print_list(diff_call);
        //выгружаем библиотеку
        FreeLibrary(lib);
        func = NULL;
        vector<string> after_free, diff_free;
        list_loaded_modules(after_free);
        find_extra(after_call, after_free, diff_free);
        cout << "--- пропало при выгрузке ---\n";
        print_list(diff_free);
    }

    return 0;
}