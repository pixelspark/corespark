#include "../include/tjshared.h"

namespace tj {
	namespace shared {
		template<> bool StringTo(std::wstring s, const bool& def) {
			std::wstring ln = s;
			std::transform(ln.begin(), ln.end(), ln.begin(), tolower);
			return def?(ln!=std::wstring(Language::Get(L"no"))):(ln==std::wstring(Language::Get(L"yes")));
		}

		template<> std::wstring Stringify(const bool& x) {
			std::wostringstream o;
			o << x?Language::Get(L"yes"):Language::Get(L"no");
			return o.str();
		}

		template<> std::wstring Stringify(const int& x) {
			wchar_t buffer[33];
			_itow_s(x, buffer, (size_t)16, 10);
			return std::wstring(buffer);
		}
	}
}