#include <Windows.h>
#include <iostream>

HWINEVENTHOOK hEvent;

typedef DWORD(WINAPI* GetModuleFileNameExFunc)(HANDLE hProcess, HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
GetModuleFileNameExFunc pGetModuleFileNameEx = (GetModuleFileNameExFunc)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "K32GetModuleFileNameExW");

const WCHAR* getOnlyProcess(const WCHAR* fullProcess) {
    const size_t lastBackslashPos = wcsrchr(fullProcess, L'\\') - fullProcess;
    const WCHAR* fileNameWithExt = fullProcess + lastBackslashPos + 1;

    return fileNameWithExt;
}

VOID CALLBACK WinEventProcCallback (HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    WCHAR processName[MAX_PATH];
    HANDLE processHandle;
    DWORD processId;

    // Get the process ID and name of the foreground window
    GetWindowThreadProcessId(hwnd, &processId);
    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (processHandle == NULL) {
        std::cerr << "Failed to open process handle: " << GetLastError() << std::endl;
        return;
    }

    // Open the process handle
    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (processHandle == NULL) {
        std::cerr << "Failed to open process handle: " << GetLastError() << std::endl;
        return;
    }

    // Get the process name
    if (pGetModuleFileNameEx(processHandle, NULL, processName, MAX_PATH)) {

        //std::wcout << "Foreground window process: " << getOnlyProcess(processName) << std::endl << std::flush;
        //
        //TCHAR windowTitle[MAX_PATH];
        //GetWindowText(hwnd, windowTitle, MAX_PATH);
        //std::wcout << "Foreground window title: " << "|" << windowTitle << "|" << std::endl << std::flush;
        //
        //WCHAR windowClassName[MAX_PATH];
        //GetClassNameW(hwnd, windowClassName, MAX_PATH);
        //std::wcout << "Foreground window class name: " << "|" << windowClassName << "|" << std::endl << std::flush;

        WCHAR windowClassName[MAX_PATH];
        GetClassNameW(hwnd, windowClassName, MAX_PATH);

        if (_wcsicmp(getOnlyProcess(processName), L"alacritty.exe") == 0) {
            ShellExecute(NULL, "open", "hideTask.exe", NULL, NULL, SW_SHOWDEFAULT);

            EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                WCHAR windowProcessName[MAX_PATH];
                DWORD windowProcessId;
                HANDLE windowProcessHandle;

                // Get the process ID and name of the window
                GetWindowThreadProcessId(hwnd, &windowProcessId);
                if (IsWindowVisible(hwnd)) {
                    windowProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, windowProcessId);
                    if (windowProcessHandle != NULL) {
                        pGetModuleFileNameEx(windowProcessHandle, NULL, windowProcessName, MAX_PATH);
                        CloseHandle(windowProcessHandle);
                    }

                    WCHAR windowClassName2[MAX_PATH];
                    GetClassNameW(hwnd, windowClassName2, MAX_PATH);

                    // If window is not alacritty AND SearchApp
                    if (_wcsicmp(getOnlyProcess(windowProcessName), L"alacritty.exe") &&
                        _wcsicmp(getOnlyProcess(windowProcessName), L"SearchApp.exe")) {

                        if (!_wcsicmp(getOnlyProcess(windowProcessName), L"explorer.exe")) {
                            if (!_wcsicmp(windowClassName2, L"CabinetWClass")) {
                                ShowWindow(hwnd, SW_MINIMIZE);
                            }
                        } else {
                            ShowWindow(hwnd, SW_MINIMIZE);
                        }
                    }
                }

                return TRUE;
            }, 0);
        } else if ((_wcsicmp(getOnlyProcess(processName), L"explorer.exe") == 0 && _wcsicmp(windowClassName, L"CabinetWClass") == 0 || _wcsicmp(getOnlyProcess(processName), L"explorer.exe"))) {
            HWND taskbar = FindWindow("Shell_TrayWnd", NULL);
            ShowWindow(taskbar, SW_SHOW);
        }
    }

    // Close the process handle
    CloseHandle(processHandle);
}

int main() {
    // Set the WinEventHook
    hEvent = SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND, NULL, 
        WinEventProcCallback, 0, 0, 
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    
    if (hEvent == NULL) {
        std::cerr << "Failed to set WinEventHook: " << GetLastError() << std::endl;
        return 1;
    }

    // Run the message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook the WinEventHook
    UnhookWinEvent(hEvent);

    return 0;
}
