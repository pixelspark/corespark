#include "../include/tjshared.h"

namespace tj {
	namespace shared {
		template<> bool StringTo(std::wstring s, bool def) {
			std::wstring ln = s;
			std::transform(ln.begin(), ln.end(), ln.begin(), tolower);
			return def?(ln!=std::wstring(Language::Get(L"no"))):(ln==std::wstring(Language::Get(L"yes")));
		}

		template<> std::wstring Stringify(const bool& x) {
			std::wostringstream o;
			o << x?Language::Get(L"yes"):Language::Get(L"no");
			return o.str();
		}
	}
}