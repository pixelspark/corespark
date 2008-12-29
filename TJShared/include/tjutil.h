#ifndef _TJUTIL_H
#define _TJUTIL_H

namespace tj {
	namespace shared {
		namespace intern {
			struct CaseInsensitiveStringTraits : public std::char_traits<wchar_t> {
				static bool eq(wchar_t c1, wchar_t c2) { return toupper(c1) == toupper(c2); }
				static bool ne(wchar_t c1, wchar_t c2) { return toupper(c1) != toupper(c2); }
				static bool lt(wchar_t c1, wchar_t c2) { return toupper(c1) <  toupper(c2); }
				static int compare(const wchar_t* s1, const wchar_t* s2, size_t n) { return _memicmp( s1, s2, n ); }
				static const wchar_t* find(const wchar_t* s, int n, char a) {
					while( n-- > 0 && toupper(*s) != toupper(a) ) {
						++s;
					}
					return s;
				}
			};	
		}

		typedef std::basic_string<wchar_t, intern::CaseInsensitiveStringTraits> CaseInsensitiveString;	// Case-insensitive string
		typedef std::wstring String;
		class Serializable;
		typedef long long Bytes; // This is equivalent to __int64 on MSVC++

		class EXPORTED Bool {
			public:
				static const wchar_t* KTrue;
				static const wchar_t* KFalse;
		};

		class EXPORTED Clipboard {
			public:
				static void SetClipboardText(const std::wstring& text);
				static bool GetClipboardText(std::wstring& text);
				static void SetClipboardObject(ref<Serializable> sr);
				static bool GetClipboardObject(ref<Serializable> sr);
				static bool GetClipboardObject(TiXmlDocument& doc);
				static bool IsTextAvailable();
				static bool IsObjectAvailable();

			private:
				static void Initialize();

				static bool _formatInitialized;
				static unsigned int _formatID;
				static const wchar_t* _formatName;
		};

		class EXPORTED Util {
			public:
				static float RandomFloat();
				static int RandomInt();
				static char* CopyString(const char* str);
				static wchar_t* IntToWide(int x);
				static std::wstring& StringToLower(std::wstring& r);
				static std::wstring GetSizeString(Bytes bytes);
				static std::wstring IPToString(in_addr ip);
		};

		template<typename T> T GetNextPowerOfTwo(T k) {
			k--;
			for(int i=1; i < (sizeof(T)*8); i *= 2) {
				k = k | k >> i;
			}
			return k+1;
		}

		template<typename T> void CleanWeakReferencesList(std::vector< weak<T> >& list) {
			std::vector< weak<T> >::iterator it = list.begin();
			while(it!=list.end()) {
				if(!it->IsValid()) {
					it = list.erase(it);
				}
				else {
					++it;
				}
			}
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

		template<typename StringType> std::vector<StringType> Explode(const StringType &inString, const StringType &separator) {
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

		template<typename StringType> StringType& ReplaceAll(StringType& result, const StringType& replaceWhat, const StringType& replaceWithWhat) {
			while(true) {
				size_t pos = result.find(replaceWhat);
				if(pos==-1) {
					break;
				}
				result.replace(pos, replaceWhat.size(), replaceWithWhat);
			}

			return result;
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

		template<typename T> inline T StringTo(const std::wstring& s, const T& def) {
			std::wistringstream i(s);
			T x;
			if (!(i >> x)) {
				return def;
			}

			return x;
		}


		template<typename T> inline T StringTo(const std::string& s, const T& def) {
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

		/** This function 'clamps' an input value between a lower and upper bound
		This means that when (val >= left && val <= right), val is returned
		Otherwise, when val < left, it returns left
		Otherwise, when val > right, it returns right **/
		template<typename T> inline T Clamp(const T& val, const T& left, const T& right) {
			if(val<left) {
				return left;
			}
			else if(val>right) {
				return right;
			}
			else {
				return val;
			}
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

		template<> EXPORTED bool StringTo(const std::wstring& s, const bool& def);
		template<> EXPORTED std::wstring StringTo(const std::wstring& s, const std::wstring& def);
		template<> EXPORTED std::wstring Stringify(const bool& x);
		template<> EXPORTED std::wstring Stringify(const int& x);
	}
}
#endif