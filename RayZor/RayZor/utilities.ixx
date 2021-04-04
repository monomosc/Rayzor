module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>
#include <sstream>
#include <comdef.h>
#include <cstdlib>
#include <wrl/client.h>

export module utilities;

class rayzor_exception : public std::exception {
public:
	rayzor_exception(HRESULT h) : hr(h) {
		std::wstringstream ss;
		_com_error cerr(hr);
		ss << L"Exception. " << cerr.ErrorMessage();
		auto s = ss.str().c_str();
		size_t size = (std::wcslen(s) + 1) * sizeof(wchar_t);
		what_buf = new char[size];
		size_t numConverted;
		wcstombs_s(&numConverted, what_buf, size, s, size);
	}
	virtual const char* what() const noexcept override {
		return what_buf;
	}
	virtual ~rayzor_exception() {
		delete[] what_buf;
	}
private:
	HRESULT hr;
	char* what_buf;
};
class string_exception : public std::exception {
public:
	string_exception(std::string s) {
		what_buf = s.c_str();
	}
	virtual const char* what() const noexcept override {
		return what_buf;
	}
	virtual ~string_exception() {
		delete[] what_buf;
	}
private:
	const char* what_buf;
};

export void CHECK_EXCEPTION(HRESULT hr) {
	if (FAILED(hr)) {
		throw rayzor_exception(hr);
	}
}
