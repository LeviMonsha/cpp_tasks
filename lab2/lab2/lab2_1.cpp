//#include <Windows.h>
//
//int main()
//{
//    HANDLE stdout = GetStdHandle(STD_OUTPUT_HANDLE);
//    if (stdout == INVALID_HANDLE_VALUE) return 1;
//    const char* str = "Hello\n";
//    DWORD len = strlen(str);
//    DWORD writ = 0;
//    BOOL succ = WriteFile(stdout, //���� �����(���������� �����)
//        str, //��� �����(����� ������)
//        len, //������� �����(� ������)
//        &writ, //������� ��������
//        NULL);
//    if (!succ || writ != len) return 2;
//    return 0;
//}