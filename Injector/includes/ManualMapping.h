#pragma once
#include <iostream>
#include <fstream>
#include <windows.h>
#define ERROR_MANUAL_MAPPING_DATA_NOT_AVALIABLE 0x80000000
#define ERROR_RELOCATION_TABLE_NOT_AVALIABLE 0x80000001
typedef HINSTANCE(WINAPI* f_LoadLibraryA)(const char* lpLibFilename);
typedef FARPROC(WINAPI* f_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);
typedef BOOL(WINAPI* f_DLL_ENTRY_POINT)(void* hDll, DWORD dwReason, void* pReserved);

namespace ManualMapping{
	struct MANUALMAPPING_DATA {
		f_LoadLibraryA pLoadLibraryA;
		f_GetProcAddress pGetProcAddress;
		uintptr_t pBase;
		HMODULE hMod;
	};
	bool Inject32(HANDLE hProc, char* path);
	void __stdcall Shellcode32(MANUALMAPPING_DATA* mmap_data);
}