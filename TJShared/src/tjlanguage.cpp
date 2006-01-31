#include "../include/tjshared.h"
#include <map>
#include <fstream>

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
		return L"...";
	}

	return L"...";
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
