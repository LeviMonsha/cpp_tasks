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

    HKEY uninstall;
    LSTATUS error = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        0,
        KEY_READ,
        &uninstall
    );
    if (error != ERROR_SUCCESS) {
        cout << "не удалось открыть ключ Uninstall\n";
    }
    else {
        cout << "\t~~~~~~ ключ Uninstall открыт ~~~~~~\n";

        vector<string> models;
        get_sub_keys(uninstall, models);
        for (size_t i = 0; i < models.size(); i++) {
            char model_name[255];
            DWORD model_name_size = 255;
            if (RegEnumKeyExA(uninstall,
                i,
                model_name,
                &model_name_size,
                NULL,
                NULL,
                NULL,
                NULL) == ERROR_SUCCESS) continue;
            HKEY model;
            error = RegOpenKeyExA(uninstall, model_name, 0, KEY_READ, &model);
            if (error == ERROR_SUCCESS) {
                char display_name[255];
                DWORD display_name_size = 255;
                error = RegGetValueA(
                    model, 
                    NULL, 
                    "DisplayName", 
                    RRF_RT_REG_SZ, 
                    NULL, 
                    (LPBYTE)display_name, 
                    &display_name_size);
                if (error == ERROR_SUCCESS) {
                    cout << display_name << endl;
                }
                else {
                    cout << models[i].c_str() << endl;
                }
                RegCloseKey(model);
            }
        }
    }
    return 0;
}
