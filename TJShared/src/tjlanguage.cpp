#include "../include/tjshared.h"
using namespace tj::shared;

Language _instance;

Language::Language() {
}

Language::~Language() {
	std::map<std::wstring, wchar_t*>::iterator it = _strings.begin();

	while(it!=_strings.end()) {
		delete it->second;
		it++;
	}
}

void Language::Translate(HWND wnd) {
	HMENU menu = GetMenu(wnd);
	if(menu==INVALID_HANDLE_VALUE) {
		return;
	}

	Translate(menu);
}

void Language::LoadDirectory(std::wstring dir) {
	WIN32_FIND_DATAW d;
	ZeroMemory(&d,sizeof(d));

	std::wstring pathfilter = dir + L"\\*.tjs";

	HANDLE hsr = FindFirstFile(pathfilter.c_str(), &d);
	wchar_t buf[MAX_PATH+1];

	do {
		if(hsr==INVALID_HANDLE_VALUE) {
			continue;
		}

		std::wstring naam = dir + L"\\";
		naam += d.cFileName;

		if(GetFullPathName(naam.c_str(), MAX_PATH,buf,0)==0) {
			continue;
		}

		Log::Write(L"TJShared/Language/DirLoad" , std::wstring(L"Loading language file ")+naam);
		Load(naam);
	} 
	while(FindNextFile(hsr, &d));
	FindClose(hsr);
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
				std::wstring text = buffer;
				delete[] buffer;

				if(text.length()>0 && text.at(0) == L'$') {
					std::wstring replacement = Get(text.substr(1));
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

template<typename StringType> std::pair<StringType,StringType> Split (const StringType &inString, const StringType &separator) {
	std::vector<StringType> returnVector;
	StringType::size_type end = inString.find(separator, 0);
	return std::pair<StringType,StringType>(inString.substr(0, end), inString.substr(end+1));
}

const wchar_t* Language::Get(std::wstring key) {
	try {
		std::map<std::wstring, wchar_t*>::iterator it = _instance._strings.find(key);
		if(it!=_instance._strings.end()) {
			return it->second;
		}
	}
	catch(...) {
		//Log::Write(L"TJShared/Language", std::wstring(L"String not found: ")+key);
		return L"...";
	}

	//Log::Write(L"TJShared/Language", std::wstring(L"String not found: ")+key);
	return L"...";
}

void Language::Clear() {
	_instance._strings.clear();
}

void Language::Load(std::wstring file) {
	std::wifstream fs(file.c_str());
	wchar_t line[1024];

	while(!fs.eof() && fs.good()) {	
		fs.getline(line,1023);
		
		std::pair<std::wstring, std::wstring> items = Split<std::wstring>(line, L":");
		_instance._strings[items.first] = _wcsdup(items.second.c_str());
	}
}
