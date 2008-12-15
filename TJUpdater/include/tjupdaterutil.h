#ifndef _TJUPDATERUTIL_H
#define _TJUPDATERUTIL_H

namespace tj {
	namespace updater {
		template<typename T> inline T GetAttribute(const TiXmlElement* el, const char* name, const T& defaultValue) {
			if(el==0) return defaultValue;
			const char* value = el->Attribute(name);
			if(value==0) return defaultValue;

			std::istringstream is(std::string(value));
			T dataValue = defaultValue;
			is >> dataValue;
			return dataValue;
		}

		template<> inline std::wstring GetAttribute(const TiXmlElement* el, const char* name, const std::wstring& defaultValue) {
			if(el==0) return defaultValue;
			const char* value = el->Attribute(name);
			if(value==0) return defaultValue;

			std::wostringstream wos;
			wos << value;
			return wos.str();
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

		class UpdaterLog {
			public:
				static void Write(const std::wstring& msg);
		};

		class UpdaterSettings {
			public:
				static std::wstring GetSetting(const std::wstring& key, const std::wstring& defaultValue = L"");
				static void SetSetting(const std::wstring& key, const std::wstring& value);
				
			private:
				static std::map<std::wstring, std::wstring> _settings;	
		};
	}
}

#endif