#include "../include/tjshared.h"
#include <shlwapi.h>
#include <commctrl.h>
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
	_paths.push_back(path);
}

void ResourceManager::RemoveSearchPath(std::wstring path) {
	std::remove(_paths.begin(), _paths.end(), path);
}

void ResourceManager::SetListener(ref<ResourceListener> l) {
	_listener = l;
}

std::wstring ResourceManager::Get(std::wstring ident, bool silent) {
	std::vector<std::wstring>::iterator it = _paths.begin();
	while(it!=_paths.end()) {
		std::wstring path = *it + L"\\"+ident;

		// file exists
		if(GetFileAttributes(path.c_str())!=INVALID_FILE_ATTRIBUTES) {
			return path;
		}
		it++;
	}

	if(GetFileAttributes(ident.c_str())!=INVALID_FILE_ATTRIBUTES) {
		return ident;
	}

	Log::Write(L"TJShared/ResourceManager", std::wstring(L"Resource not found: ")+ident);
	if(!silent && _listener) {
		return _listener->OnResourceNotFound(ident);
	}

	return L"";
}

ref<ResourceManager> ResourceManager::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ResourceManager());
	}

	return _instance;
}

ResourceListener::~ResourceListener() {
}

/* ResourceBundle */
ResourceBundle::ResourceBundle(ref<ResourceManager> mgr, std::wstring path): _path(path), _mgr(mgr) {
	assert(_mgr);

	_mgr->AddSearchPath(_path);
}

ResourceBundle::~ResourceBundle() {
	_mgr->RemoveSearchPath(_path);
}