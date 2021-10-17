#include "pch.h"
#include <algorithm>
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include "dll.h"

using namespace std;

void ReplaceString(DWORD processPid, const char* replacedString, const char* replacementString)
{

    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processPid);
    if (process)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        MEMORY_BASIC_INFORMATION info;
        std::vector<char> procMem;
        char* p = 0;
        while (p < si.lpMaximumApplicationAddress)
        {
            if (VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info))
            {
                if (info.State == MEM_COMMIT && info.AllocationProtect == PAGE_READWRITE)
                {
                    p = (char*)info.BaseAddress;
                    procMem.resize(info.RegionSize);
                    SIZE_T bytesRead;
                    try
                    {
                        if (ReadProcessMemory(process, p, &procMem[0], info.RegionSize, &bytesRead))
                        {
                            for (size_t i = 0; i < (bytesRead - strlen(replacedString)); ++i)
                            {
                                int len = strlen(replacedString);
                                if ((memcmp(replacedString, &procMem[i], len) == 0) && (replacedString != (char*)p + i))
                                {
                                    MessageBox(NULL, L"Replaced", L"Error", MB_OK | MB_ICONERROR);
                                    char* ref = (char*)p + i;

                                    for (int j = 0; j < strlen(replacementString); j++)
                                        ref[j] = replacementString[j];

                                    ref[strlen(replacementString)] = 0;
                                }
                            }
                        }
                    }
                    catch (std::bad_alloc& e)
                    {

                    }
                }
                p += info.RegionSize;
            }
        }
    }
}


void InjectionReplace(LPVOID test)
{
    char* src_str = (char*)test;
    char* res_str = (char*)test + strlen((char*)test) + 1;
    //system("pause");
    ReplaceString(GetCurrentProcessId(), src_str, res_str);
    return;
}

void Test()
{
    MessageBox(NULL, L"sd", L"Error", MB_OK | MB_ICONERROR);
}