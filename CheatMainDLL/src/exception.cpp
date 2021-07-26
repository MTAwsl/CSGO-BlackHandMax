#include "stdafx.h"
#include "Exception.h"

BHException::BHException(int line, const char* file, HRESULT hr) : line(line), file(file), hr(hr), type(WindowsError) {
	std::stringstream result;
	errorString = TranslateErrorCode(hr);
	result << GetType() << std::endl << std::endl
		<< "[Line] " << GetLine() << std::endl
		<< "[File] " << GetFile() << std::endl
		<< "[Description] " << GetErrorString();
	whatString = result.str();
}

BHException::BHException(int line, const char* file, const char* err) : line(line), file(file), errorString(err), type(CheatError) {
	std::stringstream result;
	result << GetType() << std::endl << std::endl
		<< "[Line] " << GetLine() << std::endl
		<< "[File] " << GetFile() << std::endl
		<< "[Description]" << GetErrorString() << std::endl;
	whatString = result.str();
}

std::string BHException::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	const DWORD nMsgLen = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
	);
	// 0 string length returned indicates a failure
	if (nMsgLen == 0)
	{
		type = UnknownError;
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree(pMsgBuf);
	return errorString;
}

int BHException::GetLine() const noexcept { return line; }
std::string BHException::GetFile() const noexcept { return file; }
std::string BHException::GetErrorString() const noexcept { return errorString; }
HRESULT BHException::GetErrorCode() const noexcept { return hr; }
const char* BHException::GetType() const noexcept { 
	if (type == WindowsError) {
		return "Windows Error";
	}
	else if (type == CheatError) {
		return "Internal Cheat Error";
	}
	else {
		return "Unidentified Error";
	}
}
const char* BHException::what() const noexcept { return whatString.c_str(); }

void BHException::Report() const noexcept { MessageBoxA(NULL, what(), "Unexpected error occured!", MB_OK | MB_ICONERROR); };