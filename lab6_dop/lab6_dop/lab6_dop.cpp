
#include <iostream>
#include <vector>
#include <Windows.h>
#include <string>

using namespace std;

const size_t max_queue_size = 3; //макс размер очереди
vector<string> queue; //очередь заданий
HANDLE queue_mutex; //mutex на очередь
HANDLE task_added;//новое задание
HANDLE task_taken; //задание изъято
HANDLE stop; //останов 
HANDLE cout_mutex; //mutex на консоль

int count_lines(const char* filename)
{
    // считает число строк в файле
    // ничего не выводит в cout
    // возвращает -1 при ошибке

    // буфер для чтения/записи
    vector<byte> buf(1024 * 1024 * 4);
    // сколько байт было прочитано или записано
    DWORD bytesRead;

    HANDLE hSrc = CreateFileA(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hSrc != INVALID_HANDLE_VALUE)
    {
        int lines = 1;

        ReadFile(
            hSrc,
            buf.data(),
            buf.size(),
            &bytesRead,
            NULL
        );
        for (size_t i = 0; i < bytesRead; i++)
            if (buf[i] == '\n')
            {
                lines++;
                Sleep(100);
            }
        return lines;
    }
    return -1;
}

DWORD WINAPI WorkerThread(LPVOID param) {
    //массив ожидаемых событий
    //более ранние имеют приоритет
    HANDLE events[2] = { task_added, stop };
    WaitForSingleObject(cout_mutex, INFINITE);
    cout << "рабочий поток запущен\n";
    ReleaseMutex(cout_mutex);
    while (true) {
        WaitForSingleObject(queue_mutex, INFINITE);
        if (queue.empty()) {
            //очередь пуста
            ResetEvent(task_added);
            ReleaseMutex(queue_mutex);
            //ждем либо новое задание либо сигнал об остановке
            DWORD result = WaitForMultipleObjects(
                2, events,
                FALSE, //ждем любое из событий
                INFINITE
            );
            if (result != WAIT_OBJECT_0 + 0) {
                //или сигнал стоп или ошибка
                break;
            }
        }
        else {
            //не пуста (есть задание)
            //создаем локальную копию задания
            string filename = queue[0];
            //удаляем взятое задание из очереди
            queue.erase(queue.begin());
            ReleaseMutex(queue_mutex);
            //сигнал задание взято
            SetEvent(task_taken);
            WaitForSingleObject(cout_mutex, INFINITE);
            cout << ":: обрабатываемый файл " << filename.c_str() << endl;
            ReleaseMutex(cout_mutex);
            //обрабатываем задание 
            int lines = count_lines(filename.c_str());
            //выводим результат
            WaitForSingleObject(cout_mutex, INFINITE);
            if (lines == -1) {
                cout << ":: " << filename.c_str() << ": ошибка\n";

            }
            else {
                cout << ":: " << filename.c_str() << ": " << lines << " строк\n";

            }
            ReleaseMutex(cout_mutex);
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    setlocale(0, "RU-ru");
    int thread_count;
    if (argc > 1 && isdigit(*argv[1])) {
        thread_count = stoi(argv[1]);
    }
    else {
        thread_count = 4; //число рабочих потоков
    }
    cout << "~~~~ " << "Количество потоков - " << thread_count << " ~~~~" << endl;

    vector<HANDLE> workers; //коллекция рабочих потоков
    queue_mutex = CreateMutexA(NULL, FALSE, NULL);
    cout_mutex = CreateMutexA(NULL, FALSE, NULL);
    //событие новое задание автоматическое
    task_added = CreateEventA(NULL, FALSE, FALSE, NULL);
    //событие задание извлечено автоматическое
    task_taken = CreateEventA(NULL, FALSE, FALSE, NULL);
    //событие стоп ручное
    stop = CreateEventA(NULL, TRUE, FALSE, NULL);
    //рапуск рабочих потоков
    for (int i = 0; i < thread_count; i++) {
        HANDLE hThread = CreateThread(
            NULL,
            4096,
            WorkerThread,
            NULL,
            0,
            NULL
        );
        workers.push_back(hThread);
    }
    //основная работа

    //рабочий цикл
    string line;
    //читаем строки до конца ввода
    while (getline(cin, line)) {
        //игнорирование пустых строк
        if (line.empty()) continue;
        WaitForSingleObject(cout_mutex, INFINITE);
        cout << "получен файл: " << line.c_str() << endl;
        ReleaseMutex(cout_mutex);

        //пытаемся поместить задание в очередь
        while (true) {
            WaitForSingleObject(queue_mutex, INFINITE);
            if (queue.size() < max_queue_size) {
                //в очереди есть место для задания
                queue.push_back(line);
                ReleaseMutex(queue_mutex);
                SetEvent(task_added);
                break;
            }
            else {
                //очередь задания полна
                ResetEvent(task_taken);
                ReleaseMutex(queue_mutex);
                WaitForSingleObject(cout_mutex, INFINITE);
                cout << "очередь переполнена\n";
                ReleaseMutex(cout_mutex);
                //ждем пока в очереди не появится место
                WaitForSingleObject(task_taken, INFINITE);
            }
        }
    }

    //-------------
    SetEvent(stop); //сигнал потокам завершиться
    WaitForMultipleObjects(
        workers.size(), //сколько объектов ждем
        workers.data(), //какие объекты ждем
        TRUE,
        INFINITE
    );
    cout << "рабочие потоки завершились\n";
    for (int i = 0; i < workers.size(); i++) {
        CloseHandle(workers[i]);
    }
    CloseHandle(queue_mutex);
    CloseHandle(cout_mutex);
    CloseHandle(task_added);
    CloseHandle(task_taken);
    CloseHandle(stop);

    return 0;
}