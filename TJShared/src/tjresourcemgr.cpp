#include "../include/tjshared.h"
using namespace tj::shared;

ref<ResourceManager> ResourceManager::_instance;

ResourceManager::ResourceManager() {
	/** Add location of tjshared.dll to the search list **/
	wchar_t* buf = new wchar_t[MAX_PATH];
	memset(buf,0,sizeof(wchar_t)*MAX_PATH);
	GetModuleFileName(GetModuleHandle(NULL), buf, MAX_PATH);
	PathRemoveFileSpec(buf);
	_paths.push_back(std::wstring(buf));
}

ResourceManager::~ResourceManager() {
}

void ResourceManager::AddSearchPath(std::wstring path) {
	Log::Write(L"TJShared/ResourceManager", std::wstring(L"Added path to search path list: ")+path);
	_paths.push_back(path);
}

std::wstring ResourceManager::Get(std::wstring ident) {
	Log::Write(L"TJShared/ResourceManager", ident);
	std::vector<std::wstring>::iterator it = _paths.begin();
	while(it!=_paths.end()) {
		std::wstring path = *it + L"\\"+ident;

		// file exists
		if(GetFileAttributes(path.c_str())!=INVALID_FILE_ATTRIBUTES) {
			Log::Write(L"TJShared/ResourceManager", std::wstring(L"Resource ")+ident+L" => "+path);
			return path;
		}
		else {
			Log::Write(L"TJShared/ResourceManager", std::wstring(L"\tTry ")+path);
			
		}
		it++;
	}

	if(GetFileAttributes(ident.c_str())!=INVALID_FILE_ATTRIBUTES) {
		Log::Write(L"TJShared/ResourceManager", std::wstring(L"Resource ")+ident+L" => absolute path already ");
		return ident;
	}
	else {
		Log::Write(L"TJShared/ResourceManager", std::wstring(L"Resource not found: ")+ident);
	}

	return L"";
}

ref<ResourceManager> ResourceManager::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ResourceManager());
	}

	return _instance;
}