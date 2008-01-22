#ifndef _TJUTIL_H
#define _TJUTIL_H

namespace tj {
	namespace shared {
		template<typename T> T GetNextPowerOfTwo(T k) {
			k--;
			for(int i=1; i < (sizeof(T)*8); i *= 2) {
				k = k | k >> i;
			}
			return k+1;
		}

		inline std::string Mbs(const std::wstring& ws) {
			char* buf  = new char[ws.length()+2];
			wcstombs_s(0, buf, ws.length()+1, ws.c_str(), _TRUNCATE);

			std::string w(buf);
			delete[] buf;
			return w;
		}

		inline std::wstring Wcs(const std::string& ws) {
			wchar_t* buf  = new wchar_t[ws.length()+2];
			mbstowcs_s(0, buf, ws.length()+1, ws.c_str(), _TRUNCATE);
				
			std::wstring w(buf);
			delete[] buf;
			return w;
		}

		template<typename StringType> std::vector<StringType> Explode (const StringType &inString, const StringType &separator) {
			std::vector<StringType> returnVector;
			StringType::size_type start = 0;
			StringType::size_type end = 0;

			while((end = inString.find (separator, start)) != StringType::npos) {
				returnVector.push_back(inString.substr (start, end-start));
				start = end + separator.size();
			}

			returnVector.push_back (inString.substr (start));
			return returnVector;
		}

		template<typename T> inline std::wstring Stringify(const T& x) {
			std::wostringstream os;
			os << x;
			return os.str();
		}

		template<typename T> inline std::wstring StringifyHex(const T& x) {
			std::wostringstream os;
			os << std::hex << std::uppercase << x;
			return os.str();
		}

		template<typename T> inline std::string StringifyMbs(const T& x) {
			std::ostringstream os;
			os << x;
			return os.str();
		}

		template<> std::string inline StringifyMbs(const std::wstring& x) {
			return Mbs(x);
		}

		template<typename T> inline T StringTo(std::wstring s, const T& def) {
			std::wistringstream i(s);
			T x;
			if (!(i >> x)) {
				return def;
			}

			return x;
		}


		template<typename T> inline T StringTo(std::string s, const T& def) {
			std::istringstream i(s);
			T x;
			if (!(i >> x)) {
				return def;
			}

			return x;
		}

		template<typename T> inline T StringTo(const char* s, const T& def) {
			if(s==0) return def;
			std::string inp(s);
			return StringTo<T>((std::string&)inp, def);
		}

		template<> inline std::wstring StringTo(const char* s, const std::wstring& def) {
			if(s==0) return def;
			return Wcs(std::string(s));
		}

		template<typename T> inline T StringTo(const wchar_t* s, const T& def) {
			if(s==0) return def;
			return StringTo<T>(std::wstring(s), def);
		}

		template<typename T> inline bool Near(const T& a, const T& b, const T limit) {
			return ((a<(b+limit)) && (a>(b-limit)));
		} 

		template<typename T> inline bool Between(const T&a, const T& b, const T& c) {
			return (c>a)&&(c<b);
		}

		template<typename StringType> void Trim(StringType& str) {
			StringType::size_type pos = str.find_last_not_of(' ');
			if(pos != StringType::npos) {
				str.erase(pos + 1);
				pos = str.find_first_not_of(' ');
				if(pos != StringType::npos) {
					str.erase(0, pos);
				}
			}
			else {
				str.erase(str.begin(), str.end());
			}
		}

		template<> EXPORTED bool StringTo(std::wstring s, const bool& def);
		template<> EXPORTED std::wstring Stringify(const bool& x);
		template<> EXPORTED std::wstring Stringify(const int& x);
	}
}
#endif