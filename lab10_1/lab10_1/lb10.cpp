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
	if (hMainWindow== NULL) {
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
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}