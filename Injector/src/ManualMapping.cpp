#include "ManualMapping.h"

void __stdcall ManualMapping::Shellcode32(MANUALMAPPING_DATA* mmap_data) {

	if (!mmap_data) {
		mmap_data->hMod = (HMODULE)ERROR_MANUAL_MAPPING_DATA_NOT_AVALIABLE;
		return;
	}

	// MZ-PE Headers
	IMAGE_DOS_HEADER* pHead = reinterpret_cast<IMAGE_DOS_HEADER*>(mmap_data->pBase);
	IMAGE_NT_HEADERS* pNtHead = reinterpret_cast<IMAGE_NT_HEADERS*>(pHead->e_lfanew);
	IMAGE_OPTIONAL_HEADER* pOptionalHead = reinterpret_cast<IMAGE_OPTIONAL_HEADER*>(&pNtHead->OptionalHeader);

	// Relocation
	uintptr_t delta = (mmap_data->pBase - pOptionalHead->ImageBase);
	if (delta) {
		if (!pOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
			mmap_data->hMod = (HMODULE)ERROR_RELOCATION_TABLE_NOT_AVALIABLE;
			return;
		}
		IMAGE_BASE_RELOCATION* pRelocTable = reinterpret_cast<IMAGE_BASE_RELOCATION*>(pOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		while (pRelocTable->VirtualAddress) {
			size_t AmountOfEntries = (pRelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			WORD* pRelocData = reinterpret_cast<WORD*>(pRelocTable + 1);
			for (size_t i = 0; i < AmountOfEntries; i++, pRelocData++) {
				if (((*pRelocData) >> 0x0C) == IMAGE_REL_BASED_HIGHLOW) {
					DWORD* pPatch = reinterpret_cast<DWORD*>(mmap_data->pBase + pRelocTable->VirtualAddress + ((*pRelocData) & 0xFFF));
					*pPatch += delta;
				}
			}
			pRelocTable = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<uintptr_t>(pRelocTable) + pRelocTable->SizeOfBlock);
		}
	}

	// Building IAT
	if (pOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		IMAGE_IMPORT_DESCRIPTOR* pImportDesc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(mmap_data->pBase + pOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDesc->Name) {
			char* szName = reinterpret_cast<char*>(mmap_data->pBase + pImportDesc->Name);
			HINSTANCE hMod = mmap_data->pLoadLibraryA(szName);
			uintptr_t* pThunkRef = reinterpret_cast<uintptr_t*>(mmap_data->pBase + pImportDesc->OriginalFirstThunk);
			uintptr_t* pFuncRef = reinterpret_cast<uintptr_t*>(mmap_data->pBase + pImportDesc->FirstThunk);

			if (!pThunkRef)
				pThunkRef = pFuncRef;

			for (; *pThunkRef; ++pThunkRef, ++pFuncRef)
			{
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef))
				{
					*pFuncRef = (uintptr_t)mmap_data->pGetProcAddress(hMod, reinterpret_cast<char*>(*pThunkRef & 0xFFFF));
				}
				else
				{
					IMAGE_IMPORT_BY_NAME* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(mmap_data->pBase + (*pThunkRef));
					*pFuncRef = (uintptr_t)mmap_data->pGetProcAddress(hMod, pImport->Name);
				}
			}
			++pImportDesc;
		}
	}

	// Calling TLS
	if (pOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
	{
		auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(mmap_data->pBase + pOptionalHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		auto* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);

		for (; pCallback && *pCallback; ++pCallback)
		{
			(*pCallback)(reinterpret_cast<void*>(mmap_data->pBase), DLL_PROCESS_ATTACH, nullptr);
		}
	}

	// Call DllMain
	reinterpret_cast<f_DLL_ENTRY_POINT>(mmap_data->pBase + pOptionalHead->AddressOfEntryPoint)(reinterpret_cast<void*>(mmap_data->pBase), DLL_PROCESS_ATTACH, nullptr);

	// Return Value
	mmap_data->hMod = (HMODULE)(mmap_data->pBase);
}

bool ManualMapping::Inject32(HANDLE hProc, char* path)
{
	/* Check dll file attributes */
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
	{
		std::cout << "[-] The DLL file " << path << "could not be found." << std::endl;
		return false;
	}

	/* Open file */
	std::ifstream binary_file(path, std::ios::binary | std::ios::ate);
	if (binary_file.fail())
	{
		std::cout << "[-] " << "Could not load the specific DLL file" << std::endl;
		binary_file.close();
		return false;
	}

	/* Get file size */
	std::streampos file_size = binary_file.tellg();
	if (file_size < 0x1000)
	{
		std::cout << "[-] " << "DLL Size Error." << std::endl;
		binary_file.close();
		return false;
	}

	std::cout << "[+] " << "File size 0x" << std::hex << file_size << std::endl;

	/* Allocate buffer */
	byte* buffer = reinterpret_cast<PBYTE>(malloc(file_size));
	if (!buffer)
	{
		std::cout << "[-] " << "malloc() Failed. Please try again." << std::endl;
		binary_file.close();
		return false;
	}

	std::cout << "[+] " << "Allocated buffer at 0x" << std::hex << (uintptr_t)buffer << std::endl;

	// ReadFile
	binary_file.seekg(0, std::ios::beg);
	binary_file.read(reinterpret_cast<char*>(buffer), file_size);
	binary_file.close();

	IMAGE_DOS_HEADER* pDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(buffer);
	IMAGE_NT_HEADERS* pNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pDosHeader->e_lfanew);
	IMAGE_FILE_HEADER* pFileHeader = reinterpret_cast<IMAGE_FILE_HEADER*>(&pNtHeader->FileHeader);
	IMAGE_OPTIONAL_HEADER* pOptionalHeader = reinterpret_cast<IMAGE_OPTIONAL_HEADER*>(&pNtHeader->OptionalHeader);

	if (pFileHeader->Machine != IMAGE_FILE_MACHINE_I386) {
		std::cout << "[-] " << "The DLL File is not designed to work with I386 system.";
		return false;
	}

	// Alloc Buffer in target Process
	byte* pTargetBase = reinterpret_cast<byte*>(VirtualAllocEx(hProc, nullptr, pOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	if (!pTargetBase)
	{
		std::cout << "[-] " << "VirtualAllocEx() Failed with error code " << GetLastError() << std::endl;
		free(buffer);
		return false;
	}

	std::cout << "[+] " << "Allocated 0x" << std::hex << pOptionalHeader->SizeOfImage << " bytes in target process at 0x" << std::hex << (uintptr_t)pTargetBase << std::endl;

	MANUALMAPPING_DATA mmap_data = {0};
	mmap_data.pLoadLibraryA = LoadLibraryA;
	mmap_data.pGetProcAddress = GetProcAddress;
	mmap_data.pBase = reinterpret_cast<DWORD>(pTargetBase);

	// Write Header
	if (!WriteProcessMemory(hProc, pTargetBase, buffer, 0x1000, nullptr)) {
		std::cout << "[-] " << "WriteProcessMemory() Failed with error code " << GetLastError() << std::endl;
		free(buffer);
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		return false;
	}

	// Write mmap_data
	uintptr_t mmap_data_ptr = reinterpret_cast<uintptr_t>(VirtualAllocEx(hProc, nullptr, sizeof(MANUALMAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (!mmap_data_ptr)
	{
		std::cout << "[-] " << "VirtualAllocEx() Failed with error code " << GetLastError() << std::endl;
		free(buffer);
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		return false;
	}

	std::cout << "[+] " << "Allocated 0x" << std::hex << sizeof(MANUALMAPPING_DATA) << " bytes in target process at 0x" << std::hex << (uintptr_t)mmap_data_ptr << std::endl;

	if (!WriteProcessMemory(hProc, reinterpret_cast<void*>(mmap_data_ptr), &mmap_data, sizeof(MANUALMAPPING_DATA), nullptr))
	{
		std::cout << "[-] " << "WriteProcessMemory() Failed with error code " << GetLastError() << std::endl;
		free(buffer);
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		return false;
	}
	std::cout << "[+] Write mmap_data into target process." << std::endl;

	// Mapping Sections
	IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);
	for (size_t i = 0; i < pFileHeader->NumberOfSections; i++, pSectionHeader++) {
		if (!pSectionHeader->SizeOfRawData)
			continue;
		if (WriteProcessMemory(hProc, pTargetBase + pSectionHeader->VirtualAddress, buffer + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr))
		{
			std::cout << "[+] " << "Mapped [" << pSectionHeader->Name << "] Section at " << std::hex << pTargetBase + pSectionHeader->VirtualAddress << std::endl;
			continue;
		}

		std::cout << "[-] " << "WriteProcessMemory() Failed with error code: " << GetLastError() << std::endl;

		free(buffer);
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, reinterpret_cast<void*>(mmap_data_ptr), 0, MEM_RELEASE);
		return false;
	}

	/* Allocate space for our shellcode */
	size_t shellcodeSize = reinterpret_cast<uintptr_t>(&ManualMapping::Inject32) - reinterpret_cast<uintptr_t>(&ManualMapping::Shellcode32);
	void* pShellcode = VirtualAllocEx(hProc, nullptr, shellcodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pShellcode)
	{
		std::cout << "[-] " << "VirtualAllocEx() Failed with error code " << GetLastError() << std::endl;
		free(buffer);
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, reinterpret_cast<void*>(mmap_data_ptr), 0, MEM_RELEASE);
		return false;
	}

	std::cout << "[+]" << "Allocated shellcode ( " << std::hex << shellcodeSize << "bytes at 0x" << std::hex << (uintptr_t)pShellcode << ")" << std::endl;
	
	/* Write our shellcode */
	if (!WriteProcessMemory(hProc, pShellcode, ManualMapping::Shellcode32, shellcodeSize, nullptr))
	{
		std::cout << "[-] " << "WriteProcessMemory() Failed with error code: " << GetLastError() << std::endl;
		free(buffer);
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, reinterpret_cast<void*>(mmap_data_ptr), 0, MEM_RELEASE);
		VirtualFreeEx(hProc, reinterpret_cast<void*>(pShellcode), 0, MEM_RELEASE);
		return false;
	}

	std::cout << "[+] " << "Mapped shellcode" << std::endl;

	// CreateThread
	HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode), reinterpret_cast<LPVOID>(mmap_data_ptr), 0, nullptr);
	if (!hThread)
	{
		std::cout << "[-] " << "CreateRemoteThread() Failed with error code: " << GetLastError() << std::endl;
		free(buffer);
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, reinterpret_cast<void*>(mmap_data_ptr), 0, MEM_RELEASE);
		VirtualFreeEx(hProc, reinterpret_cast<void*>(pShellcode), 0, MEM_RELEASE);
		return false;
	}

	std::cout << "[+] " << "Created thread at 0x" << std::hex << (uintptr_t)pShellcode << " (handle: 0x" << std::hex << hThread << ")" << std::endl;
	CloseHandle(hThread);

	HINSTANCE hCheck = NULL;
	while (!hCheck)
	{
		MANUALMAPPING_DATA data_checked{ 0 };
		ReadProcessMemory(hProc, reinterpret_cast<void*>(mmap_data_ptr), &data_checked, sizeof(data_checked), nullptr);
		hCheck = data_checked.hMod;

		if (hCheck == (HINSTANCE)ERROR_MANUAL_MAPPING_DATA_NOT_AVALIABLE)
		{
			std::cout << "[-] " << "Manual Mapping Data not avaliable." << std::endl;
			free(buffer);
			VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
			VirtualFreeEx(hProc, reinterpret_cast<void*>(mmap_data_ptr), 0, MEM_RELEASE);
			VirtualFreeEx(hProc, reinterpret_cast<void*>(pShellcode), 0, MEM_RELEASE);
			return false;
		}
		else if (hCheck == (HINSTANCE)ERROR_RELOCATION_TABLE_NOT_AVALIABLE)
		{
			std::cout << "[-] " << "Relocation Table not avaliable." << std::endl;
			free(buffer);
			VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
			VirtualFreeEx(hProc, reinterpret_cast<void*>(mmap_data_ptr), 0, MEM_RELEASE);
			VirtualFreeEx(hProc, reinterpret_cast<void*>(pShellcode), 0, MEM_RELEASE);
			return false;
		}
		Sleep(10);
	}

	std::cout << "[+] " << "Entry point returned!" << std::endl;

	/* Allocate new empty buffer */
	PBYTE emptyBuffer = reinterpret_cast<PBYTE>(malloc(1024 * 1024));

	if (!emptyBuffer)
	{
		std::cout << "[!] " << "DLL Header overwrite failed!" << std::endl;
		std::cout << "[!] " << "DLL Sections overwrite failed!" << std::endl;
		free(buffer);
		return true;
	}

	/* Zero buffer */
	memset(emptyBuffer, 0, 1024 * 1024);

	/* Write empty buffer */
	if (!WriteProcessMemory(hProc, pTargetBase, emptyBuffer, 0x1000, nullptr))
	{
		std::cout << "[!] " << "DLL Header overwrite failed!" << std::endl;
	}

	/* Zero sections */
	pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);
	for (size_t i = 0; i != pFileHeader->NumberOfSections; ++i, ++pSectionHeader)
	{
		if (!pSectionHeader->SizeOfRawData)
			continue;

		if (strcmp((char*)pSectionHeader->Name, ".pdata") == 0 || strcmp((char*)pSectionHeader->Name, ".rsrc") == 0 || strcmp((char*)pSectionHeader->Name, ".reloc") == 0)
		{
			if (!WriteProcessMemory(hProc, pTargetBase + pSectionHeader->VirtualAddress, emptyBuffer, pSectionHeader->SizeOfRawData, nullptr))
			{
				std::cout << "[!] DLL Section " << pSectionHeader->Name << "overwrite failed!" << std::endl;
			}
			std::cout << "[+] DLL Section " << pSectionHeader->Name << "overwrited." << std::endl;
		}
	}

	/* Free shit */
	if (buffer)
	{
		free(buffer);
	}

	if (!VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE))
	{
		std::cout << "[-] " << "Failed to free shellcode" << std::endl;
	}

	if (!VirtualFreeEx(hProc, (void*)mmap_data_ptr, 0, MEM_RELEASE))
	{
		std::cout << "[-] " << "Failed to free mmap data" << std::endl;
	}

	return true;
}
