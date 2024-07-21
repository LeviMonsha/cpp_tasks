
#include <iostream>
#include <iomanip>
#include <Windows.h>

using namespace std;

void show_info(MEMORY_BASIC_INFORMATION info) {
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
    cout << " ";
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
    cout << " ";
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
    default: cout << setw(8) << "?";
    }
    cout << endl;
}

size_t query_address_info(void* addr) {
    MEMORY_BASIC_INFORMATION info;
    if (!VirtualQuery(addr, &info, sizeof(info))) return 0;
    show_info(info);
    return info.RegionSize;
}

int main()
{
    const uint32_t gb = 1024 * 1024 * 1024;
    uint32_t addr = 0; //null
    uint32_t addr_change = addr;
    MEMORY_BASIC_INFORMATION info;
    if (!VirtualQuery(&addr, &info, sizeof(info))) return 1;
    cout << setw(8) << "Address" << " "
        << setw(8) << "Size" << " "
        << setw(8) << "State" << " "
        << setw(8) << "Type" << " "
        << setw(8) << "Protect" << " "
        << endl;
    while (addr_change < gb) {
        addr_change += query_address_info((void*)addr_change);
    }
    cout << "success";
}
