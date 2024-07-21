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