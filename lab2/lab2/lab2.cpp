#include <iostream>
#include <iomanip>
#include <Windows.h>

using namespace std;

bool isPress(int vkey) {
    SHORT response = GetKeyState(vkey);
    return (response & (1 << 15)) != 0;
}

int main()
{
    int a = 10;
    int *b = &a;
    cout << b;

    SYSTEMTIME now;
    bool change_time = true; // создание и объявление bool переменной change_time и присвоение true
    while (!isPress(VK_ESCAPE)) {
        // меняем bool значение пермененной change_time, 
        // если нажата кнопка пробела
        if (isPress(VK_SPACE)) { 
            change_time = !change_time; 
        }
        // если значение change_time = true задаётся локальное время (GetLocalTime(&now)),
        // change_time = false - задаётся время по Гринвичу (GetSystemTime(&now))
        if (change_time) {
            GetLocalTime(&now);            
        } else { 
            GetSystemTime(&now); 
        }
        cout << setw(2) << setfill('0')
            << now.wHour << ":"
            << setw(2) << setfill('0')
            << now.wMinute << ":"
            << setw(2) << setfill('0')
            << now.wSecond << "\x08\x08\x08\x08\x08\x08\x08\x08";
        Sleep(200); // задержка выполнения программы на 0.2 секунды
    }
}