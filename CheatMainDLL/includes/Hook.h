#pragma once

namespace Hook {
	void Patch(byte* dst, byte* src, size_t size);
	bool Detour(byte* src, byte* dst, size_t len);
	byte* TrampHook(byte* src, byte* dst, size_t len);
}