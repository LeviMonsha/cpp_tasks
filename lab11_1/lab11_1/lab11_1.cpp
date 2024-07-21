#include <iostream>
#include <Windows.h>
#include <shobjidl.h>

using namespace std;

bool open_dlg(wstring& filepath) {
    bool success = false;

    filepath.clear();
    CLSID class_id = CLSID_FileOpenDialog;
    IID interface_id = IID_IFileOpenDialog;
    IFileDialog* pDialog; //обертка в виде класса c++
    HRESULT res = CoCreateInstance(
        class_id, //экземпляр какого класса
        NULL, //экземпляр будет сам по себе
        CLSCTX_ALL, //контекст использования
        interface_id, //какой интерфейс нужен
        (void**)&pDialog //куда поместили ссылку
    );
    if (SUCCEEDED(res)) {
        pDialog->SetTitle(L"исходный файл");
        if (SUCCEEDED(pDialog->Show(NULL))) {
            //пользователь выбрал файл
            IShellItem* pItem;
            res = pDialog->GetResult(&pItem);
            if (SUCCEEDED(res)) {
                //получили выбранный файл
                PWSTR path;
                //метод сам выделит память для строки
                res = pItem->GetDisplayName(
                    SIGDN_FILESYSPATH, //путь к файлу
                    &path //куда поместит адрес строки
                );
                if (SUCCEEDED(res)) {
                    //путь получен
                    //объект wstring копирует строку
                    filepath = path;
                    //освобождаем память
                    CoTaskMemFree(path);
                    success = true;
                }
                pItem->Release();
            }
        }
        pDialog->Release();
    }

    return success;
}

int main()
{
    setlocale(0, "RU-ru");

   HRESULT res = CoInitializeEx(
        NULL, 
        COINIT_APARTMENTTHREADED
    );
   if (FAILED(res)) {
       wcout << L"подсистема COM недоступна\n";
       return 1;
   }
   wcout << L"подсистема COM готова к работе\n";

   wstring path;
   if (open_dlg(path)) {
       wcout << L"выбран файл: " << path.c_str() << endl;
   }
   else {
       wcout << L"не удалось узнать путь\n";
   }

   CoUninitialize();

    return 0;
}