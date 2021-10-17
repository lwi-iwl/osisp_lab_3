#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string>
#include <cstring>
#include <iostream>
#include "dll.h"
#include <stdio.h>
#include <vector>
#include <locale.h>  
#include <TlHelp32.h>


void printMemoryStrings()
{
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	if (process)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		MEMORY_BASIC_INFORMATION info;
		std::vector<char> chunk;
		char* p = 0;
		while (p < si.lpMaximumApplicationAddress)
		{
			if (VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info))
			{
				if (info.State == MEM_COMMIT && info.AllocationProtect == PAGE_READWRITE)
				{
					p = (char*)info.BaseAddress;
					chunk.resize(info.RegionSize);
					SIZE_T bytesRead;
					try
					{
						if (ReadProcessMemory(process, p, &chunk[0], info.RegionSize, &bytesRead))
						{
							for (char i : chunk)
								std::cout << i;
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


int main()
{
	const char testString[] = "Working fine!";
	const char testString1[] = "Working fine!";
    std::string replaced;
    std::cin >> replaced;
    std::string replacement;
    std::cin >> replacement;
	//const char* replacedS = "fine";
	//const char* replacementS = "dine";
    const char *replacedS = replaced.c_str();
    const char* replacementS = replacement.c_str();
    ReplaceString(GetCurrentProcessId(), (char*)replacedS, (char*)replacementS);
	printMemoryStrings();
}
