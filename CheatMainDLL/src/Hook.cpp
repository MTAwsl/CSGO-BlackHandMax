#include "stdafx.h"

// patch bytes function
void Hook::Patch(byte* dst, byte* src, size_t size) {
	DWORD oProc;
	if (!VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oProc)) throw BHException(__LINE__, __FILE__, GetLastError());
	memcpy(dst, src, size);
	if (!VirtualProtect(dst, size, oProc, &oProc)) throw BHException(__LINE__, __FILE__, GetLastError());
}

// hook function
bool Hook::Detour(byte* src, byte* dst, size_t len) {
	if (len < 5) return false;
	DWORD oProc;
	if (!VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &oProc)) throw BHException(__LINE__, __FILE__, GetLastError());
	memset(src, 0x90, len);
	uintptr_t relAddy = (uintptr_t)(dst - src - 5);
	*src = (char)0xE9;
	*(uintptr_t*)(src + 1) = (uintptr_t)relAddy;
	if (!VirtualProtect(src, len, oProc, &oProc)) throw BHException(__LINE__, __FILE__, GetLastError());
	return true;
}

// trampoline hook function
byte* Hook::TrampHook(byte* src, byte* dst, size_t len) {
	if (len < 5) return 0;
	byte* gateway = (byte*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (gateway == NULL)
		return nullptr;
	memcpy(gateway, src, len);
	uintptr_t jumpAddy = (uintptr_t)(src - gateway - 5);
	*(gateway + len) = (byte)0xE9;
	*(uintptr_t*)(gateway + len + 1) = jumpAddy;
	if (Hook::Detour(src, dst, len)) {
		return gateway;
	}
	else return nullptr;
}