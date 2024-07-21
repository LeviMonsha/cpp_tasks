#include <Windows.h>
#include <string>

using namespace std;

string get_error_message(DWORD error) {
	string result(4096, 0);
	DWORD len = FormatMessageA(//сообщение об ошибке
		FORMAT_MESSAGE_FROM_SYSTEM, //системные ошибки
		NULL,
		error, //код ошибки, для которой берем описание
		0, //описание на текущем языке системы
		&result[0], //куда записать описание ошибки
		result.size(), //размер буфера
		NULL
	);

	return result.substr(0, len);
}

HINSTANCE instance; //дескриптор экземпляра приложения

//оконная функция главного окна программы
LRESULT __stdcall MainWndProc(
	HWND hWnd, //дескриптор окна
	UINT uMsg, //код оконного сообщения
	//параметры оконного сообщения
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (uMsg) {
	case WM_DESTROY: { //окно уничтожается
		//по закрытию окна завершаем работу
		PostQuitMessage(0); //код завершения
	}; break;
	default:
		//вызывай DefWindowProc()
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
}

//точка входа в оконное приложение
int __stdcall WinMain(
	HINSTANCE hInstance, //экземпляр приложения 
	HINSTANCE prevInstance, //зарезервирован
	LPSTR lpCmdLine, //параметры командной строки
	int nShowCmd //желаемое состояние окна
) 
{
	instance = hInstance;
	WNDCLASSEXA cls; //класс окна
	ZeroMemory(&cls, sizeof(cls));

	cls.cbSize = sizeof(cls); //контроль целостности
	//имя класса - уникальное в пределах процесса
	cls.lpszClassName = "MainWindow";
	//оконная функция для данного класса окон
	cls.lpfnWndProc = MainWndProc;
	//дескриптор экземпляра приложения
	cls.hInstance = instance;
	//стиль класса окна
	//перерисовать при изменении размера
	cls.style = CS_HREDRAW | CS_VREDRAW;
	//используемый фон окна - цвет кнопок \ окон
	cls.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	//вид курсора внутри окна
	cls.hCursor = LoadCursor(NULL, IDC_IBEAM);
	//регистрируем класс окна
	if (RegisterClassExA(&cls) == NULL) {
		//регистрация не удалась
		string err = get_error_message(GetLastError());
		MessageBoxA(NULL, err.c_str(), "ошибка регистрации класса окна", MB_OK | MB_ICONERROR);
		return 1;
	}
	//создаем окно программы
	HWND hMainWindow = CreateWindowExA(
		//расширенный стиль окна
		WS_EX_APPWINDOW, //окно приложения
		cls.lpszClassName, //имя класса окна
		//текст окна (для главного окна - заголовок)
		"новое окно",
		//стиль окна
		//окно приложения с заголовком и системным меню
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		//положение окна на экране
		CW_USEDEFAULT, 0, //система выбирает сама
		//размер окна (внешний, вместе с рамкой и заголовком)
		300, 150, //ширина, высота в пикселях
		//дескриптор родительского окна
		NULL, //нет родителя - окно верхнего уровня
		//дескриптор главного меню окна
		NULL, //нет меню
		//дескриптор экземпляра приложения
		instance,
		NULL //зарезервировано
	);
	if (hMainWindow== NULL) {
		//регистрация не удалась
		string err = get_error_message(GetLastError());
		MessageBoxA(NULL, err.c_str(), "ошибка создания окна", MB_OK | MB_ICONERROR);
		return 1;
	}
	//показываем окно
	ShowWindow(hMainWindow, nShowCmd);
	//обновляем окно
	UpdateWindow(hMainWindow);
	//цикл обработки сообщений
	MSG msg;
	while (GetMessageA(
		&msg, 
		NULL, //для всех окон смотрим 
		0, 
		0
	)) 
	{
		//пока не получили WM_QUIT
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}