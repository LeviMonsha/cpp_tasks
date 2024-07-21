#include <iostream>
#include <Windows.h>
#include <vector>

using namespace std;

void CopyFile(HANDLE hSrc, HANDLE hDst) {
	DWORD fileSize = GetFileSize(hSrc, NULL);
	//буфер для чтения
	vector<byte> bufRead(fileSize);
	//сколько байт было прочитано/записано
	DWORD byteRead, byteWritten;
	//планируем чтение
	ReadFile(
		hSrc, //откуда читаем
		bufRead.data(), //куда читаем
		bufRead.size(), //сколько читаем
		&byteRead, //потом узнаем сколько байт прочитано
		NULL
	);
	//планируем запись
	WriteFile(
		hDst, //файл приемник (куда пишем)
		bufRead.data(), //что пишем
		byteRead, //пишем сколько, сколько почитали
		&byteWritten, //потом узнаем, сколько байт записали
		NULL
	);
	cout << "Rewrite " << byteWritten << " byte.\n";
	//Закрытие файлов и освобождение памяти
	CloseHandle(hSrc);
	CloseHandle(hDst);
}

int main(int argc, char* argv[])
{
	setlocale(0, "");

	const char* SrcName = "lab4_dop.cpp";
	const char* DstName = "copy.txt";

	HANDLE hSrc = CreateFileA(
		SrcName, //какой файл открываем
		GENERIC_READ, //для чтения
		FILE_SHARE_READ, //другие могут тоже читать
		NULL, //атрибуты безопасности по умолчанию
		OPEN_EXISTING, //открыть сущ. файл
		NULL, //синхронный режим
		NULL //игнорирование параметра
	);

	if (hSrc == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		cout << "cant open " << SrcName << endl;
		cout << "error code: " << err << endl;
	}
	else {
		cout << "open for read " << SrcName << endl;
		//работа с файлом
		HANDLE hDst = CreateFileA(
			DstName, //имя файла
			GENERIC_WRITE, //для записи
			0,
			NULL, //настройка безопасности файла по умолчанию
			CREATE_ALWAYS,
			NULL, //синхронный режим
			NULL //игнорирование параметра
		);
		if (hDst == INVALID_HANDLE_VALUE) {
			DWORD err = GetLastError();
			cout << "cant open " << SrcName << endl;
			cout << "error code: " << err << endl;

		}
		else {
			cout << "open for rewrite " << DstName << endl;
			CopyFile(hSrc, hDst);
			CloseHandle(hDst);
		}
		CloseHandle(hSrc);
	}

	return 0;
}