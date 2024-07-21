#include <iostream>
#include <iomanip>

using namespace std;

void subproc(uint32_t& esp, uint32_t& eip)
{
    uint32_t ESPSub, EIPSub;
    __asm
    {
    label_sub: // метка
        // помещаем адрес метки в регистр EAX
        lea EAX, label_sub
            // пересылаем значения в переменную
            mov EIPSub, EBX
            // помещаем адрес вершины стка в переменную
            mov ESPSub, ESP;
    };
    cout << "Inside subproc()" << endl;
    cout << "EIPMain1 = " << hex << setw(8) << setfill('0') << EIPSub << endl;
    cout << "ESPMain1 = " << hex << setw(8) << setfill('0') << ESPSub << endl;
    cout << "ESPDelta = " << hex << dec << setfill('0') << esp - ESPSub << endl;

    esp = ESPSub;
    eip = EIPSub;
}

int main()
{
    uint32_t ESPMain1, EIPMain1;
    __asm
    {
    label_main1: // метка
        // помещаем адерс метки в регистр EAX
        lea EAX, label_main1
            // пересылаем значения в переменную
            mov EIPMain1, EAX
            // помещаем адрес вершины стка в переменную
            mov ESPMain1, ESP;
    };
    cout << "Before subproc()" << endl;
    cout << "EIPMain1 = " << hex << setw(8) << setfill('0') << EIPMain1 << endl;
    cout << "ESPMain1 = " << hex << setw(8) << setfill('0') << ESPMain1 << endl;
    // hex - переводит в 16 чную СС, setw(n) - дополняет число до n знаков, setfill(r) - дополняет r символами
    subproc(ESPMain1, EIPMain1);

    __asm
    {
    label_main2: // метка
        // помещаем адерс метки в регистр EAX
        lea EAX, label_main2
            // пересылаем значения в переменную
            mov EIPMain1, EAX
            // помещаем адрес вершины стка в переменную
            mov ESPMain1, ESP;
    };

    cout << "After subproc()" << endl;
    cout << "EIPMain1 = " << hex << setw(8) << setfill('0') << EIPMain1 << endl;
    cout << "ESPMain1 = " << hex << setw(8) << setfill('0') << ESPMain1 << endl;

    return 0;
}
