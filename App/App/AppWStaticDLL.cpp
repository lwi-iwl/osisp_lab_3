#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <comdef.h>
#include "dll.h"
#include <stdio.h>
#include <locale.h>
#include <TlHelp32.h>



LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND programName;
HWND stringName;
HWND rStringName;
HWND isInjectButton;
HWND isStaticButton;
HWND isDynamicButton;
int condition = 0;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow)
{
    const char testString[] = "qwertyuiop";
    const char testString1[] = "qwertyuiop";
    WNDCLASS windowClass = { 0 };
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = L"HELLO_WORLD";
    windowClass.hbrBackground = (HBRUSH)GetStockObject(COLOR_WINDOW + 1);
    RegisterClass(&windowClass);
    HWND hwnd = CreateWindow(
        windowClass.lpszClassName,
        L"CatXP",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        100, 50, 720, 360,
        nullptr, nullptr,
        hInstance,
        nullptr);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}

DWORD GetProcessIdByName(const char* process_name)
{
    DWORD Id = -1;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 procEntry;
    ZeroMemory(&procEntry, sizeof(PROCESSENTRY32W));
    procEntry.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32First(hSnapshot, &procEntry))
    {
        while (Process32Next(hSnapshot, &procEntry))
        {
            _bstr_t b(procEntry.szExeFile);
            const char* c = b;
            if (strcmp(c, process_name) == 0)
            {
                Id = procEntry.th32ProcessID;
                break;
            }
        }
    }
    return Id;
}

typedef void ReplaceStringStructure(DWORD, const char*, const char*);

void ReplaceStringDynamic(DWORD pid, const char* src_str, const char* res_str)
{
    HMODULE hDll = LoadLibraryA("Dll1.dll");

    if (hDll != NULL)
    {
        ReplaceStringStructure* replaceString = (ReplaceStringStructure*)GetProcAddress(hDll, "ReplaceString");

        if (replaceString != NULL)
        {
            replaceString(pid, src_str, res_str);
        }

        FreeLibrary(hDll);
    }
}

typedef int InjectStructure(HANDLE, HMODULE, ReplaceStringStructure, char[]);

void InjectDLL(const char* process_name, const char* replacedStr, const char* replacementStr)
{
    DWORD procID = GetProcessIdByName(process_name);
    if (procID != -1)
    {
        DWORD remoteFunctionAddr = NULL;
        DWORD result = 0;
        HANDLE hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, NULL, procID);

        if (hProc != 0)
        {
            char path[] = "C:\\Users\\nikst\\source\\repos\\App\\Debug\\Dll1.dll";
            LPVOID baseAddress = (LPVOID)VirtualAllocEx(hProc, NULL, strlen(path) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (baseAddress != 0) {
                WriteProcessMemory(hProc, (LPVOID)baseAddress, path, strlen(path), NULL);
                HMODULE hMod = GetModuleHandle(L"kernel32.dll");
                if (hMod)
                {
                    LPVOID lpvLoadLib = (LPVOID)GetProcAddress(hMod, "LoadLibraryA");
                    if (lpvLoadLib)
                    {
                        HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)lpvLoadLib, (LPVOID)baseAddress, NULL, NULL);
                        if (hThread!=0) {
                            WaitForSingleObject(hThread, INFINITE);
                            GetExitCodeThread(hThread, &result);
                            CloseHandle(hThread);
                        }
                    }
                }
            }

            if (result != 0)
            {

                HMODULE hModule = LoadLibraryA("Dll1.dll");

                if (hModule != NULL)
                {
                    intptr_t functionAddr = (intptr_t)GetProcAddress(hModule, "InjectionReplace");
                    if (functionAddr != NULL)
                    {
                        DWORD functionOffs = (DWORD)functionAddr - (DWORD)hModule;
                        remoteFunctionAddr = functionOffs + result;

                        int argumentsLength = strlen(replacedStr) + strlen(replacementStr) + 2;

                        LPVOID argumentsAddr = (LPVOID)VirtualAllocEx(hProc, NULL, argumentsLength, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                        
                        if (argumentsAddr != 0) 
                        {
                            char* data = (char*)malloc(argumentsLength);

                            if (data) {
                                memset(data, 0, argumentsLength);

                                memcpy(data, replacedStr, strlen(replacedStr));
                                memcpy(data + strlen(replacedStr) + 1, replacementStr, strlen(replacementStr));
                                if (argumentsAddr) {
                                    WriteProcessMemory(hProc, argumentsAddr, data, argumentsLength, NULL);
                                }
                            }
                            HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)remoteFunctionAddr, (LPVOID)argumentsAddr, NULL, NULL);
                            WaitForSingleObject(hThread, INFINITE);
                            free(data);
                            VirtualFreeEx(hProc, (LPVOID)argumentsAddr, 0, MEM_RELEASE);
                        }
                    }

                    FreeLibrary(hModule);
                }
            }
            CloseHandle(hProc);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_CREATE:
    {
        stringName = CreateWindowEx(NULL, L"Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 20, 400, 30, hWnd, (HMENU)2, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        rStringName = CreateWindowEx(NULL, L"Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 90, 400, 30, hWnd, (HMENU)3, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        programName = CreateWindowEx(NULL, L"Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 160, 400, 30, hWnd, (HMENU)1, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        isStaticButton = CreateWindow(L"button", L"Static DLL", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            500, 20, 160, 30, hWnd, (HMENU)5, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        isDynamicButton = CreateWindow(L"button", L"Dynamic DLL", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            500, 90, 160, 30, hWnd, (HMENU)6, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        isInjectButton = CreateWindow(L"button", L"Injection DLL", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            500, 160, 160, 30, hWnd, (HMENU)7, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);

        HWND Button = CreateWindow(L"BUTTON", L"Replace", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 230, 100, 30, hWnd, (HMENU)4,(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        ShowWindow(programName, SW_HIDE);
        SendMessage(isStaticButton, BM_SETCHECK, BST_CHECKED, 0);
    }
    break;
    
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC winDC = BeginPaint(hWnd, &ps);
        TextOut(winDC, 10, 3, L"Replaced string", 15);
        TextOut(winDC, 10, 73, L"Replacement string", 18);
        TextOut(winDC, 10, 143, L"Program name", 12);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
            case BN_CLICKED:
            {
                if (LOWORD(wParam) == 4)
                {
                    char replacedS[MAX_CLASS_NAME];
                    GetWindowTextA((HWND)stringName, &replacedS[0], 199);
                    const char* replaced = replacedS;
                    char replacementS[MAX_CLASS_NAME];
                    GetWindowTextA((HWND)rStringName, &replacementS[0], 199);
                    const char* replacement = replacementS;
                    SetWindowText(stringName, L"");
                    SetWindowText(rStringName, L"");
                    if (condition == 0)
                    {
                        ReplaceString(GetCurrentProcessId(), replaced, replacement);
                    }
                    else if (condition == 1)
                    {
                        ReplaceStringDynamic(GetCurrentProcessId(), replaced, replacement);
                    }
                    else if (condition == 2)
                    {
                        char IDname[MAX_CLASS_NAME];
                        GetWindowTextA((HWND)programName, &IDname[0], 199);
                        const char* programN = IDname;
                        InjectDLL(programN, replaced, replacement);
                    }
                    SetWindowText(programName, L"");
                }
                else if (LOWORD(wParam) == 5)
                {
                    SendMessage(isInjectButton, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendMessage(isDynamicButton, BM_SETCHECK, BST_UNCHECKED, 0);
                    condition = 0;
                    ShowWindow(programName, SW_HIDE);
                    LRESULT res = SendMessage(isInjectButton, BM_GETCHECK, 0, 0);
                    if (res == BST_UNCHECKED)
                        SendMessage(isStaticButton, BM_SETCHECK, BST_CHECKED, 0);
                }
                else if (LOWORD(wParam) == 6)
                {
                    SendMessage(isInjectButton, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendMessage(isStaticButton, BM_SETCHECK, BST_UNCHECKED, 0);
                    condition = 1;
                    ShowWindow(programName, SW_HIDE);
                    LRESULT res = SendMessage(isInjectButton, BM_GETCHECK, 0, 0);
                    if (res == BST_UNCHECKED)
                        SendMessage(isDynamicButton, BM_SETCHECK, BST_CHECKED, 0);
                }
                else if (LOWORD(wParam) == 7)
                {
                    SendMessage(isStaticButton, BM_SETCHECK, BST_UNCHECKED, 0);
                    SendMessage(isDynamicButton, BM_SETCHECK, BST_UNCHECKED, 0);
                    LRESULT res = SendMessage(isInjectButton, BM_GETCHECK, 0, 0);
                    if (res == BST_CHECKED)
                        ShowWindow(programName, SW_SHOW);
                    else
                        SendMessage(isInjectButton, BM_SETCHECK, BST_CHECKED, 0);
                    condition = 2;
                }
            }
        }
        break;
    }
    break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    default: {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    }
    return 0;
}
