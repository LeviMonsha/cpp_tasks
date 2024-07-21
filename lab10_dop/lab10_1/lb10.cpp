#include <Windows.h>
#include <string>

using namespace std;

string get_error_message(DWORD error) {
	string result(4096, 0);
	DWORD len = FormatMessageA(//��������� �� ������
		FORMAT_MESSAGE_FROM_SYSTEM, //��������� ������
		NULL,
		error, //��� ������, ��� ������� ����� ��������
		0, //�������� �� ������� ����� �������
		&result[0], //���� �������� �������� ������
		result.size(), //������ ������
		NULL
	);

	return result.substr(0, len);
}

string get_wnd_text(HWND wnd) {
	int size = GetWindowTextLengthA(wnd);
	string result = "";
	if (size > 0) {
		//� ���� ����� �� ������
		result.resize(size + 1, 0);
		GetWindowTextA(
			wnd, //���� ������ �������� �����
			&result[0], //���� ��������
			result.size() //������ ������
		);
	}
	return result;
}


HINSTANCE instance; //���������� ���������� ����������
HWND hText = NULL; //�������� � ����
HWND hEdit = NULL; //���� �����
HWND hBtn = NULL; //������

void make_child_controls(HWND hParent) {
	//������� �������� �������� �������� ����
	RECT parentrect;
	GetClientRect(hParent, &parentrect);
	int width = parentrect.right - parentrect.left;
	//��������� �����
	hText = CreateWindowA(
		"STATIC", //��������� ����� - ������ �����
		"������� ����� ����:", //����� ����
		//����� �������� ������� ����� �� ������
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		//������� � ������� ����
		5, 5, width - 10, 20, //x, y, width, height
		hParent, //������������ ����
		NULL, //���� ���
		instance,
		NULL
	);
	hEdit = CreateWindowExA(
		WS_EX_CLIENTEDGE, //3d �����
		"EDIT", //��� ������ - ���� �����
		"", //����� ���� - ���������� ���� �����
		//����� �������� ������� � ������
		//����� �� ������ ���� � ���������� ������
		WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT | ES_AUTOHSCROLL,
		//������� � �������
		5, 30, width - 10, 30,
		hParent,
		NULL,
		instance,
		NULL
	);
	hBtn = CreateWindowA(
		"BUTTON", //��� ������ ������
		"����� �� ����", //
		//�����
		WS_CHILD | WS_VISIBLE | BS_CENTER | BS_VCENTER,
		//������� � �������
		5, 65, width - 10, 30,
		hParent,
		NULL,
		instance,
		NULL
	);
}

BOOL __stdcall check_window(HWND hWnd, LPARAM lParam) {
	//�������� ��������� ����
	string text = get_wnd_text(hWnd);
	if (text.empty()) return TRUE; //����� - ���� ������
	const char* match = (const char*)lParam; //�������
	if (text.find(match) == string::npos) return TRUE;

	RECT r;
	GetWindowRect(hWnd, &r);
	HRGN reg1 = CreateRectRgn(0, -30, r.right - r.left, r.bottom - r.top);
	HRGN reg2 = CreateRectRgn(100, 70, r.right - r.left - 100, r.bottom - r.top - 100);
	CombineRgn(reg1, reg2, reg1, RGN_DIFF);
	SetWindowRgn(hWnd, reg1, TRUE);
	DeleteObject(reg2);

	return FALSE;
	//return TRUE; //��������� � ���������� ����
}


//������� ������� �������� ���� ���������
LRESULT __stdcall MainWndProc(
	HWND hWnd, //���������� ����
	UINT uMsg, //��� �������� ���������
	//��������� �������� ���������
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (uMsg) {
	case WM_DESTROY: { //���� ������������
		//�� �������� ���� ��������� ������
		PostQuitMessage(0); //��� ����������
	}; break;
	case WM_CREATE: { //���� �������
		make_child_controls(hWnd);
	}; break;
	case WM_COMMAND: { //������ �� ��������� ��������
		if ((wParam == BN_CLICKED) && ((HWND)lParam == hBtn)) {
			//������ �� ������ hBtn
			string text = get_wnd_text(hEdit);

			if (text.empty()) {

				MessageBoxA(
					hWnd, //����������� (������ ����������� �� ������� �������)
					"����� ������ ����� ��������� ����",
					"error",
					MB_OK
				);
			}
			else {
				//����������� ���� ����������
				BOOL result = EnumWindows(
					check_window, //callback
					(LPARAM)text.c_str() //��������
				);
				DWORD err = GetLastError();
				if (result) {
					MessageBoxA(
						hWnd, //����������� (������ ����������� �� ������� �������)
						"��� ���� � ����� ������",
						"error",
						MB_OK
					);
				}
				else if (err != 0) {
					MessageBoxA(
						hWnd, //����������� (������ ����������� �� ������� �������)
						"������ ������������ ����",
						"error",
						MB_OK
					);
				}

			}
		}
	}
	default:
		//������� DefWindowProc()
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
}

//����� ����� � ������� ����������
int __stdcall WinMain(
	HINSTANCE hInstance, //��������� ���������� 
	HINSTANCE prevInstance, //��������������
	LPSTR lpCmdLine, //��������� ��������� ������
	int nShowCmd //�������� ��������� ����
)
{
	instance = hInstance;
	WNDCLASSEXA cls; //����� ����
	ZeroMemory(&cls, sizeof(cls));

	cls.cbSize = sizeof(cls); //�������� �����������
	//��� ������ - ���������� � �������� ��������
	cls.lpszClassName = "MainWindow";
	//������� ������� ��� ������� ������ ����
	cls.lpfnWndProc = MainWndProc;
	//���������� ���������� ����������
	cls.hInstance = instance;
	//����� ������ ����
	//������������ ��� ��������� �������
	cls.style = CS_HREDRAW | CS_VREDRAW;
	//������������ ��� ���� - ���� ������ \ ����
	cls.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	//��� ������� ������ ����
	cls.hCursor = LoadCursor(NULL, IDC_IBEAM);
	//������������ ����� ����
	if (RegisterClassExA(&cls) == NULL) {
		//����������� �� �������
		string err = get_error_message(GetLastError());
		MessageBoxA(NULL, err.c_str(), "������ ����������� ������ ����", MB_OK | MB_ICONERROR);
		return 1;
	}
	//������� ���� ���������
	HWND hMainWindow = CreateWindowExA(
		//����������� ����� ����
		WS_EX_APPWINDOW, //���� ����������
		cls.lpszClassName, //��� ������ ����
		//����� ���� (��� �������� ���� - ���������)
		"����� ����",
		//����� ����
		//���� ���������� � ���������� � ��������� ����
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		//��������� ���� �� ������
		CW_USEDEFAULT, 0, //������� �������� ����
		//������ ���� (�������, ������ � ������ � ����������)
		300, 150, //������, ������ � ��������
		//���������� ������������� ����
		NULL, //��� �������� - ���� �������� ������
		//���������� �������� ���� ����
		NULL, //��� ����
		//���������� ���������� ����������
		instance,
		NULL //���������������
	);
	if (hMainWindow == NULL) {
		//����������� �� �������
		string err = get_error_message(GetLastError());
		MessageBoxA(NULL, err.c_str(), "������ �������� ����", MB_OK | MB_ICONERROR);
		return 1;
	}
	//���������� ����
	ShowWindow(hMainWindow, nShowCmd);
	//��������� ����
	UpdateWindow(hMainWindow);
	//���� ��������� ���������
	MSG msg;
	while (GetMessageA(
		&msg,
		NULL, //��� ���� ���� ������� 
		0,
		0
	))
	{
		//���� �� �������� WM_QUIT
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return (int)msg.wParam;
}