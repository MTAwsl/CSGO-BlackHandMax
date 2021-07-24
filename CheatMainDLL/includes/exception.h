#pragma once
#include <exception>
#include <sstream>

class BHException : public std::exception {
public:
	BHException(int line, const char* file, HRESULT hr);
	BHException(int line, const char* file, const char* err);
	const char* what() const noexcept;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	std::string GetFile() const noexcept;

	void Report() const noexcept;

	HRESULT GetErrorCode() const noexcept;
	virtual std::string GetErrorString() const noexcept;
private:
	int line;
	std::string file;
	enum errorType {
		UnknownError,
		WindowsError,
		CheatError
	};
	errorType type;
	HRESULT hr;

	std::string whatString;
	std::string errorString;
private:
	std::string TranslateErrorCode(HRESULT hr) noexcept;
};