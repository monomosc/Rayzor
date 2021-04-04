module;

#include <d3d12.h>
#include <sstream>
#include <exception>
#include <comdef.h>

export module core_utilities;

namespace core {
	export constexpr D3D12_GPU_VIRTUAL_ADDRESS gpu_address_null() { return (D3D12_GPU_VIRTUAL_ADDRESS)0; }

	export struct non_copyable {
		non_copyable(const non_copyable& other) = delete;
		non_copyable& operator=(const non_copyable&) = delete;
		non_copyable() = default;
	};
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
	export void CHECK_EXCEPTION(HRESULT hr) {
		if (FAILED(hr)) {
			throw rayzor_exception(hr);
		}
	}
}