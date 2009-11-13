#ifndef _TJUTIL_H
#define _TJUTIL_H

#include "tjsharedinternal.h"
#include "tjtime.h"
#include "tjcrypto.h"

struct in_addr;
#include <set>
#include <vector>
#include <list>
#include <map>
#include <deque>

namespace tj {
	namespace shared {
		class Serializable;
		typedef int64 Bytes; 

		class EXPORTED Bool {
			public:
				static const wchar_t* ToString(bool t);
				static bool FromString(const wchar_t* str);

				static const wchar_t* KTrue;
				static const wchar_t* KFalse;
		};
		
		template<typename T> class Range {
			public:
				Range(T start, T end) {
					_start = start;
					_end = end;
				}
				
				inline T& Start() {
					return _start;
				}
				
				inline T& End() {
					return _end;
				}
				
				inline void SetStart(T start) {
					_start = start;
				}
				
				inline void SetEnd(T end) {
					_end = end;
				}
				
				inline bool IsValid() {
					return _start<=_end;
				}
				
				inline T Length() {
					return _end - _start;
				}
				
				template<typename Q> static Range<Q>& Widest(Range<Q> a, Range<Q> b) {
					return a.Length()>b.Length()?a:b;
				}
				
				template<typename Q> static Range<T>& Narrowest(Range<Q> a, Range<Q> b) {
					return a.Length()<b.Length()?a:b;
				}
				
			protected:
				T _start;
				T _end;
		};
		
		template<typename T, typename IntType=int> class Flags {
			public:
				Flags(T flags) {
					_data = flags;
				}

				Flags() {
					_data = (T)0;
				}

				bool IsSet(T flag) const {
					return (IntType(_data) & IntType(flag)) != 0;
				}

				void Set(T flag, bool active) {
					if(active) {
						_data = (T)(IntType(flag)|IntType(_data));
					}
					else {
						_data = (T)(IntType(_data) & (~IntType(flag)));
					}
				}

				const T GetValue() const {
					return _data;
				}

				inline void operator += (T flag) {
					Set(flag, true);
				}

				inline void operator -= (T flag) {
					Set(flag, false);
				}

				inline bool operator[](T flag) {
					return IsSet(flag);
				}

			protected:
				T _data;
		};

		class EXPORTED Power {
			public:
				enum Status {
					PowerUnknown = 0,
					PowerHasAC = 1,
					PowerIsOnACBackup = 2,
					PowerHasBattery = 4,
				};

				static Flags<Status> GetStatus();
		};

		class EXPORTED Clipboard {
			public:
				static void SetClipboardText(const String& text);
				static bool GetClipboardText(String& text);
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
				static String RandomIdentifier(wchar_t prefix);
				static char* CopyString(const char* str);
				static wchar_t* CopyString(const wchar_t* str);
				static wchar_t* IntToWide(int x);
				static String& StringToLower(String& r);
				static String GetSizeString(Bytes bytes);
				static String IPToString(const in_addr& ip);
				static String GetApplicationDirectory();
				static String GetDescriptionOfSystemError(int errorNumber);
			
				#ifdef TJ_OS_MAC
					static CFStringRef StringToMacString(const std::wstring& s);
					static std::wstring MacStringToString(CFStringRef cr);
				#endif

				template<typename T> static inline T Min(T a, T b) {
					if(a<b) {
						return a;
					}
					return b;
				}

				template<typename T> static inline T Max(T a, T b) {
					if(a>b) {
						return a;
					}
					return b;
				}
		};

		/* This class takes care of disabling the screensaver */
		class EXPORTED ScreensaverOff {
			public:
				ScreensaverOff();
				~ScreensaverOff();

			protected:
				int* _values;
		};

		class EXPORTED Copyright {
			public:
				inline Copyright(const String& module, const String& component, const String& description): _module(module), _component(component), _description(description) {
					AddCopyright(this);
				}

				~Copyright();
				static String Dump();
				static void AddCopyright(Copyright* cs);

			private:
				String _module;
				String _component;
				String _description;
				static ref< std::set<Copyright*> > _copyrights;
		};

		class EXPORTED MediaUtil {
			public:
				static Time GetDuration(const String& file);
		};

		/** Command-line argument parser. **/
		class EXPORTED Arguments: public virtual Object {
			public:
				Arguments(const wchar_t* commandLine); 
				virtual ~Arguments();
				std::vector<wchar_t*>* GetOptions();
				bool IsSet(const String& option);

			protected:
				void Parse();
				wchar_t* m_cmdline;

				/* Gives C4251 in VC++: _options needs to have dll-interface to be used by clients. Since
				_options is protected, it should and normally cannot be used by  'clients' */
				std::vector<wchar_t*> _options;
		};

		template<typename T> T GetNextPowerOfTwo(T k) {
			k--;
			for(int i=1; i < (sizeof(T)*8); i *= 2) {
				k = k | k >> i;
			}
			return k+1;
		}

		template<typename T> void CleanWeakReferencesList(std::vector< weak<T> >& list) {
			typename std::vector< weak<T> >::iterator it = list.begin();
			while(it!=list.end()) {
				if(!it->IsValid()) {
					it = list.erase(it);
				}
				else {
					++it;
				}
			}
		}

		inline std::string Mbs(const String& ws) {
			unsigned int n = (unsigned int)ws.length();
			char* buf  = new char[n+1];
			
			#ifdef TJ_OS_POSIX
				wcstombs(buf, ws.c_str(), n+1);
			#endif
			
			#ifdef TJ_OS_WIN
				wcstombs_s(0, buf, n+1, ws.c_str(), _TRUNCATE);
			#endif

			buf[n] = '\0';
			std::string w(buf);
			delete[] buf;
			return w;
		}

		inline String Wcs(const std::string& ws) {
			unsigned int n = (unsigned int)ws.length();
			
			#ifdef TJ_OS_WIN
				wchar_t* buf  = new wchar_t[n+1];
				mbstowcs_s(0, buf, n+1, ws.c_str(), _TRUNCATE);
			#endif
			
			#ifdef TJ_OS_POSIX
				wchar_t* buf = reinterpret_cast<wchar_t*>(alloca((n+1)*sizeof(wchar_t)));
				mbstowcs(buf, ws.c_str(), n+1);
			#endif
			
			buf[n] = 0;
			String w(buf);
			
			#ifdef TJ_OS_WIN
				delete[] buf;
			#endif
			
			return w;
		}

		template<typename StringType> std::vector<StringType> Explode(const StringType &inString, const StringType &separator) {
			std::vector<StringType> returnVector;
			typename StringType::size_type start = 0;
			typename StringType::size_type end = 0;

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

		template<typename T> inline String Stringify(const T& x) {
			std::wostringstream os;
			os << x;
			return os.str();
		}

		template<typename T> inline String StringifyHex(const T& x) {
			std::wostringstream os;
			os << std::hex << std::uppercase << x;
			return os.str();
		}

		template<typename T> inline std::string StringifyMbs(const T& x) {
			std::ostringstream os;
			os << x;
			return os.str();
		}

		template<> std::string inline StringifyMbs(const String& x) {
			return Mbs(x);
		}

		template<typename T> inline T StringTo(const String& s, const T& def) {
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

		template<> inline String StringTo(const char* s, const String& def) {
			if(s==0) return def;
			return Wcs(std::string(s));
		}

		template<typename T> inline T StringTo(const wchar_t* s, const T& def) {
			if(s==0) return def;
			return StringTo<T>(String(s), def);
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
			typename StringType::size_type pos = str.find_last_not_of(' ');
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

		template<> EXPORTED bool StringTo(const String& s, const bool& def);
		template<> EXPORTED int StringTo(const String& s, const int& def);
		template<> EXPORTED String StringTo(const String& s, const String& def);
		template<> EXPORTED String Stringify(const bool& x);
		template<> EXPORTED String Stringify(const int& x);
	}
}
#endif
