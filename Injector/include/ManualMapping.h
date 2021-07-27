#pragma once
#include <stdint.h>
#include <windef.h>
namespace ManualMapping{
	struct MANUALMAPPING_DATA {
		uintptr_t pLoadLibraryA;
		uintptr_t pGetProcAddress;
		uintptr_t pBase;
	};
	bool Inject(HANDLE hProc, char* path);
	void __stdcall Shellcode(MANUALMAPPING_DATA* mmap_data);
}