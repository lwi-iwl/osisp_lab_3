#pragma once



#define DLL_API __declspec(dllexport)



extern "C" DLL_API void ReplaceString(DWORD processPid, const char* replacedString, const char* replacementString);

extern "C" DLL_API void InjectionReplace(LPVOID test);

extern "C" DLL_API void Test();