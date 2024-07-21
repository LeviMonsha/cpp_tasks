#include <iostream>
#include <vector>
#include <Windows.h>
#include <string>
#include <algorithm>

using namespace std;


void get_sub_keys(HKEY parent, vector<string>& subkeys) {
    DWORD subkeycount;
    DWORD namelen;
    subkeys.clear();
    LSTATUS error = RegQueryInfoKeyA(
        parent,
        NULL, NULL,
        NULL, //зарезервирован
        &subkeycount,
        &namelen,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    if (error != ERROR_SUCCESS) return;
    subkeys.reserve(subkeycount);
    for (DWORD idx = 0; idx < subkeycount; idx++) {
        string namebuf(namelen + 1, 0);
        error = RegEnumKeyA(
            parent,
            idx,
            &namebuf[0],
            namebuf.size()
        );
        if (error == ERROR_SUCCESS) {
            subkeys.push_back(namebuf);
        }
    }

}

int main()
{
    setlocale(0, "RU-ru");

    HKEY usbstor;
    LSTATUS error = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Enum\\USBSTOR",
        0,
        KEY_READ,
        &usbstor
    );
    if (error != ERROR_SUCCESS) {
        cout << "не удалось открыть ключ USBSTOR\n";
    }
    else {
        cout << "ключ USBSTOR открыт\n";

        vector<string> models, instances, names;
        get_sub_keys(usbstor, models);
        for (size_t i = 0; i < models.size(); i++) {
            cout << models[i].c_str() << endl;
            HKEY model;
            error = RegOpenKeyExA(
                usbstor,
                models[i].c_str(),
                0,
                KEY_READ,
                &model
            );
            if (error != ERROR_SUCCESS) continue;
            get_sub_keys(model, instances);
            for (size_t j = 0; j < instances.size(); j++) {
                cout << "\t" << instances[j].c_str() << endl;
                string friendlyname(2048, 0);
                DWORD namesize = friendlyname.size();
                error = RegGetValueA(
                    model,
                    instances[j].c_str(),
                    "FriendlyName",
                    RRF_RT_REG_SZ, //принимаем только строки
                    NULL,
                    &friendlyname[0],
                    &namesize
                );
                if (error == ERROR_SUCCESS) {
                    names.push_back(friendlyname.substr(0, namesize));
                }
            }

            RegCloseKey(model);
        }

        RegCloseKey(usbstor);
        cout << "накопители данных\n";

        sort(names.begin(), names.end());
        auto stop = unique(names.begin(), names.end());
        names.erase(stop, names.end());

        for (size_t i = 0; i < names.size(); i++) {
            cout << names[i].c_str() << endl;
        }
    }

    return 0;
}
