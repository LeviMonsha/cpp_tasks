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

string get_wnd_text(HWND wnd) {
	int size = GetWindowTextLengthA(wnd);
	string result = "";
	if (size > 0) {
		//в окне текст не пустой
		result.resize(size + 1, 0);
		GetWindowTextA(
			wnd, //окно откуда копируем текст
			&result[0], //куда копируем
			result.size() //размер буфера
		);
	}
	return result;
}


HINSTANCE instance; //дескриптор экземпляра приложения
HWND hText = NULL; //надписть в окне
HWND hEdit = NULL; //поле ввода
HWND hBtn = NULL; //кнопка

void make_child_controls(HWND hParent) {
	//создаем дочерние элементы главного окна
	RECT parentrect;
	GetClientRect(hParent, &parentrect);
	int width = parentrect.right - parentrect.left;
	//текстовая метка
	hText = CreateWindowA(
		"STATIC", //системный класс - просто текст
		"введите текст ниже:", //текст окна
		//стиль дочернее видимое текст по центру
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		//позиция и размеры окна
		5, 5, width - 10, 20, //x, y, width, height
		hParent, //родительское окно
		NULL, //меню нет
		instance,
		NULL
	);
	hEdit = CreateWindowExA(
		WS_EX_CLIENTEDGE, //3d рамка
		"EDIT", //имя класса - поле ввода
		"", //текст окна - содержимое поля ввода
		//стиль дочернее видимое с рамкой
		//текст по левому краю с прокруткой текста
		WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT | ES_AUTOHSCROLL,
		//позиция и размеры
		5, 30, width - 10, 30,
		hParent,
		NULL,
		instance,
		NULL
	);
	hBtn = CreateWindowA(
		"BUTTON", //имя класса кнопка
		"нажми на меня", //
		//стиль
		WS_CHILD | WS_VISIBLE | BS_CENTER | BS_VCENTER,
		//позиция и размеры
		5, 65, width - 10, 30,
		hParent,
		NULL,
		instance,
		NULL
	);
}

BOOL __stdcall check_window(HWND hWnd, LPARAM lParam) {
	//получаем заголовок окна
	string text = get_wnd_text(hWnd);
	if (text.empty()) return TRUE; //пусто - идем дальше
	const char* match = (const char*)lParam; //искомое
	if (text.find(match) == string::npos) return TRUE;

	RECT r;
	GetWindowRect(hWnd, &r);
	HRGN reg1 = CreateRectRgn(0, -30, r.right - r.left, r.bottom - r.top);
	HRGN reg2 = CreateRectRgn(100, 70, r.right - r.left - 100, r.bottom - r.top - 100);
	CombineRgn(reg1, reg2, reg1, RGN_DIFF);
	SetWindowRgn(hWnd, reg1, TRUE);
	DeleteObject(reg2);

	return FALSE;
	//return TRUE; //переходим к следующему окну
}


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
	case WM_CREATE: { //окно создано
		make_child_controls(hWnd);
	}; break;
	case WM_COMMAND: { //сигнал от дочернего элемента
		if ((wParam == BN_CLICKED) && ((HWND)lParam == hBtn)) {
			//щелчок по кнопке hBtn
			string text = get_wnd_text(hEdit);

			if (text.empty()) {

				MessageBoxA(
					hWnd, //модальность (запрет фокусировки на базовый элемент)
					"нужно ввести часть заголовка окна",
					"error",
					MB_OK
				);
			}
			else {
				//перечисляем окна приложений
				BOOL result = EnumWindows(
					check_window, //callback
					(LPARAM)text.c_str() //параметр
				);
				DWORD err = GetLastError();
				if (result) {
					MessageBoxA(
						hWnd, //модальность (запрет фокусировки на базовый элемент)
						"нет окна с таким именем",
						"error",
						MB_OK
					);
				}
				else if (err != 0) {
					MessageBoxA(
						hWnd, //модальность (запрет фокусировки на базовый элемент)
						"ошибка перечисления окон",
						"error",
						MB_OK
					);
				}

			}
		}
	}
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
	if (hMainWindow == NULL) {
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
		DispatchMessageA(&msg);
	}

	return (int)msg.wParam;
}