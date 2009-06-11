#include "../include/tjcore.h"
#include "../include/properties/tjproperties.h"
using namespace tj::shared;

Language _instance;
std::vector<String> Language::_availableLocales;

Language::Language() {
}

Language::~Language() {
	std::map<std::string, wchar_t*>::iterator it = _strings.begin();

	while(it!=_strings.end()) {
		delete it->second;
		++it;
	}
}

#ifdef TJ_OS_WIN
	void Language::Translate(HWND wnd) {
		HMENU menu = GetMenu(wnd);
		if(menu==INVALID_HANDLE_VALUE) {
			return;
		}

		Translate(menu);
	}

	void Language::Translate(HMENU menu) {
		int count = GetMenuItemCount(menu);
		if(count>0) {
			MENUITEMINFO inf;
			memset(&inf, 0, sizeof(MENUITEMINFO));
			for(int a=0;a<count;a++) {
				inf.fMask = MIIM_STRING;
				inf.cbSize = sizeof(MENUITEMINFO);
				inf.dwTypeData = NULL;
				GetMenuItemInfo(menu, a, TRUE, &inf);

				// allocate string buffer
				if(inf.fMask & MIIM_STRING) {
					
					wchar_t* buffer = new wchar_t[inf.cch+2];
					inf.cch++;
					inf.dwTypeData = buffer;
					GetMenuItemInfo(menu, a, TRUE, &inf);
					String text = buffer;
					delete[] buffer;

					if(text.length()>0 && text.at(0) == L'$') {
						String replacement = Get(text.substr(1));
						inf.dwTypeData = (wchar_t*)replacement.c_str();
						inf.cch = (UINT)replacement.length();
						SetMenuItemInfo(menu, a, TRUE, &inf);
					}

					HMENU sub = GetSubMenu(menu, a);
					if(sub!=NULL) {
						Translate(sub);
					}
				}

			}
		}
	}
#endif

ref<Property> Language::CreateLanguageProperty(const String& title, ref<Inspectable> holder, LocaleIdentifier* lang) {
	assert(lang!=0);
	ref< GenericListProperty<LocaleIdentifier> > pp = GC::Hold(new GenericListProperty<LocaleIdentifier>(title, holder, lang, *lang));

	std::vector<LocaleIdentifier>::const_iterator it = _availableLocales.begin();
	while(it!=_availableLocales.end()) {
		std::wostringstream name;
		name << *it;
		std::string key = "locale_"+Mbs(*it);
		if(_instance._strings.find(key)!=_instance._strings.end()) {
			name << L": " << Language::GetLiteral(key);
		}
		pp->AddOption(name.str(), *it);
		++it;
	}

	return pp;
}

void Language::FindLocales(const String& dir) {
	ZoneEntry zea(Zones::LocalFileInfoZone);
	ZoneEntry zeb(Zones::LocalFileReadZone);
	ZoneEntry zec(Zones::ModifyLocaleZone);

	#ifdef TJ_OS_WIN
		WIN32_FIND_DATAW d;
		ZeroMemory(&d,sizeof(d));
		String pathfilter = dir + L"\\*.*";
		HANDLE hsr = FindFirstFile(pathfilter.c_str(), &d);

		do {
			if(hsr==INVALID_HANDLE_VALUE) {
				continue;
			}

			if(d.cFileName[0]==L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||(d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0) {
				// skip, file is not a directory or it is hidden, or the first character is a '.'
				// (which indicates the special '.' or '..' directories under Windows and Unix and (under Unix)
				// hidden files).
			}
			else {
				_availableLocales.push_back(d.cFileName);
				Log::Write(L"TJShared/Language", L"Locale found: "+String(d.cFileName));
			}
		} 
		while(FindNextFile(hsr, &d));
		FindClose(hsr);
	#endif
	
	#ifdef TJ_OS_MAC
		#warning Not implemented
	#endif
}

void Language::LoadDirectory(const String& locdir, const LocaleIdentifier& lang) {
	ZoneEntry zea(Zones::LocalFileInfoZone);
	ZoneEntry zeb(Zones::LocalFileReadZone);
	ZoneEntry zec(Zones::ModifyLocaleZone);

	// Find languages
	Language::FindLocales(locdir);

	#ifdef TJ_OS_WIN
		// Find .tjs files in the locale dir
		String dir = locdir + L'\\' + lang;

		WIN32_FIND_DATAW d;
		ZeroMemory(&d,sizeof(d));

		String pathfilter = dir + L"\\*.tjs";

		HANDLE hsr = FindFirstFile(pathfilter.c_str(), &d);
		wchar_t buf[MAX_PATH+1];

		do {
			if(hsr==INVALID_HANDLE_VALUE) {
				continue;
			}

			String naam = dir + L"\\";
			naam += d.cFileName;

			if(GetFullPathName(naam.c_str(), MAX_PATH,buf,0)==0) {
				continue;
			}

			#ifdef _DEBUG
				// Only be verbose if we are a debug build
				Log::Write(L"TJShared/Language/DirLoad" , String(L"Loading language file ")+naam);
			#endif
			
			Load(naam);
		} 
		while(FindNextFile(hsr, &d));
		FindClose(hsr);
	#endif
	
	#ifdef TJ_OS_MAC
		#warning Not implemented
	#endif
}

template<typename StringType> std::pair<StringType,StringType> Split (const StringType &inString, const StringType &separator) {
	typename std::vector<StringType> returnVector;
	typename StringType::size_type end = inString.find(separator, 0);
	return std::pair<StringType,StringType>(inString.substr(0, end), inString.substr(end+1));
}

const wchar_t* Language::Get(const String& key) {
	return GetLiteral(Mbs(key));
}

const wchar_t* Language::GetLiteral(const std::string& lit) {
	std::map<std::string, wchar_t*>::const_iterator it = _instance._strings.find(lit);
	if(it!=_instance._strings.end()) {
		return it->second;
	}
	return L"...";
	
}

void Language::Clear() {
	ZoneEntry zec(Zones::ModifyLocaleZone);
	_instance._strings.clear();
}

void Language::Load(const String& file) {
	ZoneEntry zeb(Zones::LocalFileReadZone);
	ZoneEntry zec(Zones::ModifyLocaleZone);

	std::wifstream fs(Mbs(file).c_str());
	wchar_t line[1024];

	while(!fs.eof() && fs.good()) {	
		fs.getline(line,1023);
		
		std::pair<String, String> items = Split<String>(line, L":");
		_instance._strings[Mbs(items.first)] = Util::CopyString(items.second.c_str());
	}
}
