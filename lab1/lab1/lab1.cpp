
#include <iostream>
#include <iomanip>
using namespace std;

void sub_subproc(uint32_t& eip, uint32_t& esp) {
    uint32_t EIPSup2, ESPSup2;
    __asm {
    // помещаем адрес метки в регистр EAX
    label_sup2: //метка
        lea EAX, label_sup2
            // пересылаем значения в переменную
            mov EIPSup2, EAX
            // помещаем адрес вершины стека в переменную
            mov ESPSup2, ESP;
    };
    
    cout << "After sub_subproc()" << endl;
    cout << "EIP = " << hex << setw(8) << setfill('0') << EIPSup2 << endl;
    cout << "ESP = " << hex << setw(8) << setfill('0') << ESPSup2 << endl;
    cout << endl;

    // присваиваем полученные значения (EIPSup2, ESPSup2) входящим параметрам (eip, esp)
    eip = EIPSup2;
    esp = ESPSup2;
}

void subproc(uint32_t& eip, uint32_t& esp) {
    uint32_t EIPSup, ESPSup;
    __asm {
    // помещаем адрес метки в регистр EAX
    label_sup: //метка
        lea EAX, label_sup
            // пересылаем значения в переменную
            mov EIPSup, EAX
            // помещаем адрес вершины стека в переменную
            mov ESPSup, ESP;
    };

    cout << "Inside subproc()" << endl;
    cout << "EIP = " << hex << setw(8) << setfill('0') << EIPSup << endl;
    cout << "ESP = " << hex << setw(8) << setfill('0') << ESPSup << endl;

    cout << "ESP delta\nESP2 - ESP = " << dec << esp - ESPSup << endl;
    cout << endl;

    // присваиваем полученные значения (EIPSup, ESPSup) входящим параметрам (eip, esp)
    eip = EIPSup;
    esp = ESPSup;

    // вызов функции sub_subproc с параметрами eip, esp
    sub_subproc(eip, esp);
}

int main()
{
    uint32_t EIPM, ESPM;
    __asm {
    // помещаем адрес метки в регистр EAX
    label_main1: //метка 
        lea EAX, label_main1
            // пересылаем значения в переменную
            mov EIPM, EAX
            // помещаем адрес вершины стека в переменную
            mov ESPM, ESP;
    };

    cout << "Before subproc()" << endl;
    cout << "EIP = " << hex << setw(8) << setfill('0') << EIPM << endl;
    cout << "ESP = " << hex << setw(8) << setfill('0') << ESPM << endl;
    cout << endl;

    // вызов функции subproc с параметрами eip, esp
    subproc(EIPM, ESPM);

    __asm {
    // помещаем адрес метки в регистр EAX
    label_main2: //метка
        lea EAX, label_main2
            // пересылаем значения в переменную
            mov EIPM, EAX
            // помещаем адрес вершины стека в переменную
            mov ESPM, ESP;
    };

    cout << "After subproc()" << endl;
    cout << "EIP = " << hex << setw(8) << setfill('0') << EIPM << endl;
    cout << "ESP = " << hex << setw(8) << setfill('0') << ESPM << endl;
    cout << endl;
}
