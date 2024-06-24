#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ID_BTN_NOTEPAD 1
#define ID_BTN_SHOWTIME 2
#define ID_BTN_ADD 3
#define ID_TIMER 4
#define ID_TEXTBOX1 5
#define ID_TEXTBOX2 6
//gcc -o agenda_app agenda_app.c -mwindows
typedef struct {
    SYSTEMTIME time;
    char message[256];
} Event;

Event events[100];
int event_count = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void LoadEvents() {
    FILE *file = fopen("clock.ini", "r");
    if (!file) {
        file = fopen("clock.ini", "w");
        if (!file) {
            MessageBox(NULL, "Could not create clock.ini file", "Error", MB_OK | MB_ICONERROR);
            return;
        }
        fclose(file);
        return;
    }

    event_count = 0;
    char line[512];
    while (fgets(line, sizeof(line), file) && event_count < 100) {
        int year, month, day, hour, minute;
        char message[256];
        if (sscanf(line, "%d\\%d\\%d %d:%d=%255[^\n]", &year, &month, &day, &hour, &minute, message) == 6) {
            events[event_count].time.wYear = year;
            events[event_count].time.wMonth = month;
            events[event_count].time.wDay = day;
            events[event_count].time.wHour = hour;
            events[event_count].time.wMinute = minute;
            events[event_count].time.wSecond = 0;
            events[event_count].time.wMilliseconds = 0;
            strncpy(events[event_count].message, message, sizeof(events[event_count].message));
            event_count++;
        }
    }

    fclose(file);
}

void SaveEvent(const char *datetime, const char *message) {
    FILE *file = fopen("clock.ini", "a");
    if (file) {
        fprintf(file, "%s=%s\r\n", datetime, message);
        fclose(file);
    }
}

void CheckEvents() {
    SYSTEMTIME now;
    GetLocalTime(&now);

    for (int i = 0; i < event_count; i++) {
        if (CompareFileTime(&now, &events[i].time) >= 0) {
            MessageBox(NULL, events[i].message, "Event Notification", MB_OK | MB_ICONINFORMATION);

            // Remove the event from the list
            for (int j = i; j < event_count - 1; j++) {
                events[j] = events[j + 1];
            }
            event_count--;
            i--; // Check the new event at this position
        }
    }
}

void ShowCurrentTime() {
    SYSTEMTIME now;
    GetLocalTime(&now);

    char buffer[256];
    sprintf(buffer, "Current Time: %04d\\%02d\\%02d %02d:%02d:%02d",
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond);

    MessageBox(NULL, buffer, "Current Time", MB_OK | MB_ICONINFORMATION);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "AgendaClass";
    
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 0)); // Amarelo

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Agenda",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndTextBox1, hwndTextBox2;
    switch (uMsg) {
    case WM_CREATE:
        hwndTextBox1 = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 10, 200, 20, hwnd, (HMENU)ID_TEXTBOX1, NULL, NULL);
        hwndTextBox2 = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 220, 10, 200, 20, hwnd, (HMENU)ID_TEXTBOX2, NULL, NULL);

        CreateWindowEx(0, "BUTTON", "Open Notepad", WS_CHILD | WS_VISIBLE, 10, 40, 150, 30, hwnd, (HMENU)ID_BTN_NOTEPAD, NULL, NULL);
        CreateWindowEx(0, "BUTTON", "Show Current Time", WS_CHILD | WS_VISIBLE, 170, 40, 150, 30, hwnd, (HMENU)ID_BTN_SHOWTIME, NULL, NULL);
        CreateWindowEx(0, "BUTTON", "Add Event", WS_CHILD | WS_VISIBLE, 330, 40, 150, 30, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);

        LoadEvents();
        SetTimer(hwnd, ID_TIMER, 60000, NULL); // Verifica os eventos a cada minuto
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BTN_NOTEPAD) {
            system("notepad clock.ini");
        } else if (LOWORD(wParam) == ID_BTN_SHOWTIME) {
            ShowCurrentTime();
        } else if (LOWORD(wParam) == ID_BTN_ADD) {
            char datetime[256], message[256];
            GetWindowText(hwndTextBox1, datetime, sizeof(datetime));
            GetWindowText(hwndTextBox2, message, sizeof(message));
            SaveEvent(datetime, message);
            LoadEvents(); // Recarrega os eventos
        }
        break;

    case WM_TIMER:
        if (wParam == ID_TIMER) {
            CheckEvents();
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

