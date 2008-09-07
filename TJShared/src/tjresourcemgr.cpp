#include "../include/tjcore.h"
#include <shlwapi.h>
#include <commctrl.h>
#include <shellapi.h>
#include <winioctl.h>
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

std::wstring ResourceManager::GetSearchPath() const {
	std::wostringstream os;
	std::vector< std::wstring >::const_iterator it = _paths.begin();
	while(it!=_paths.end()) {
		const std::wstring& root = *it;
		os << root << L";";
		++it;
	}
	return os.str();
}

void ResourceManager::AddSearchPath(const std::wstring& path) {
	_paths.push_back(path);
}

void ResourceManager::RemoveSearchPath(const std::wstring& path) {
	std::remove(_paths.begin(), _paths.end(), path);
}

void ResourceManager::SetListener(ref<ResourceListener> l) {
	_listener = l;
}

/* This method created a relative resource path ('rid') from a full, absolute path
using the search paths */
ResourceIdentifier ResourceManager::GetRelative(const std::wstring& path) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	std::vector< std::wstring >::const_reverse_iterator it = _paths.rbegin();
	wchar_t relativePath[MAX_PATH+3];
	while(it!=_paths.rend()) {
		const std::wstring& root = *it;
		
		if(PathRelativePathTo(relativePath, root.c_str(), FILE_ATTRIBUTE_DIRECTORY, path.c_str(), FILE_ATTRIBUTE_NORMAL)==TRUE) {
			// Windows sometimes adds './' in front of relative paths, remove it
			if(relativePath[0]==L'.' && relativePath[1]==L'\\') {
				return std::wstring((const wchar_t*)&(relativePath[2]));
			} 

			return relativePath;
		}
		++it;
	}
	return path;
}

std::wstring ResourceManager::Get(const ResourceIdentifier& ident, bool silent) {
	ZoneEntry ze(Zones::LocalFileInfoZone);

	// If the path starts with a protocol identifier such as http://, just return the URL
	if(ident.substr(0,7)==L"http://") {
		return ident;
	}

	std::vector<std::wstring>::iterator it = _paths.begin();
	while(it!=_paths.end()) {
		std::wstring path = *it + L"\\"+ident;

		// file exists
		if(GetFileAttributes(path.c_str())!=INVALID_FILE_ATTRIBUTES) {
			return path;
		}
		++it;
	}

	if(GetFileAttributes(ident.c_str())!=INVALID_FILE_ATTRIBUTES) {
		return ident;
	}

	if(!silent) {
		Log::Write(L"TJShared/ResourceManager", std::wstring(L"Resource not found: ")+ident);
		if(_listener) {
			return _listener->OnResourceNotFound(ident);
		}
	}

	return L"";
}

strong<ResourceManager> ResourceManager::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ResourceManager());
	}

	return _instance;
}

ResourceListener::~ResourceListener() {
}

/* ResourceBundle */
ResourceBundle::ResourceBundle(ref<ResourceManager> mgr, const std::wstring& path): _path(path), _mgr(mgr) {
	assert(_mgr);
	_mgr->AddSearchPath(_path);
}

ResourceBundle::~ResourceBundle() {
	_mgr->RemoveSearchPath(_path);
}

/* Resource */
Resource::~Resource() {
}

ref<Resource> Resource::LoadResource(TiXmlElement* serialized) {
	std::wstring type = LoadAttributeSmall<std::wstring>(serialized, "type", L"local");
	if(type==L"local") {
		return GC::Hold(new LocalFileResource(serialized));
	}
}

/** LocalFileResource **/
LocalFileResource::LocalFileResource(const ResourceIdentifier& rid): _rid(rid), _cachedSize(0) {
}

LocalFileResource::LocalFileResource(TiXmlElement* e): _cachedSize(0) {
	assert(e!=0);
	_rid = LoadAttributeSmall<ResourceIdentifier>(e, "rid",L"");
}

LocalFileResource::~LocalFileResource() {
}

void LocalFileResource::Save(TiXmlElement* el) {
	SaveAttributeSmall(el, "rid", _rid);
	SaveAttributeSmall<std::wstring>(el, "type", L"local");
}

bool LocalFileResource::Exists() const {
	std::wstring rpath = ResourceManager::Instance()->Get(_rid,true);
	if(GetFileAttributes(rpath.c_str())!=INVALID_FILE_ATTRIBUTES) {
		return true;
	}

	return false;
}

std::wstring LocalFileResource::GetExtension() const {
	return std::wstring(PathFindExtension(_rid.c_str()));
}

ResourceIdentifier LocalFileResource::GetIdentifier() const {
	return _rid;
}

std::wstring LocalFileResource::GetPath() const {
	return ResourceManager::Instance()->Get(_rid, true);
}

void LocalFileResource::Open() const {
	std::wstring rpath = ResourceManager::Instance()->Get(_rid,true);
	ShellExecute(NULL, L"open", rpath.c_str(), L"", L"", SW_SHOW);
}

Bytes LocalFileResource::GetSize() {
	if(_cachedSize!=0) {
		return _cachedSize;
	}

	std::wstring rpath = ResourceManager::Instance()->Get(_rid,true);
	HANDLE hdl = CreateFile(rpath.c_str(), FILE_READ_ACCESS, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0L, 0L);
	LARGE_INTEGER size;
	GetFileSizeEx(hdl, &size);
	_cachedSize = size.QuadPart;
	CloseHandle(hdl);
	return _cachedSize;
}

void LocalFileResource::OpenFolder() const {
	std::wstring rpath = ResourceManager::Instance()->Get(_rid,true);
	wchar_t* path = _wcsdup(rpath.c_str());
	PathRemoveFileSpec(path);
	ShellExecute(NULL, L"explore", path, L"", L"", SW_SHOW);
	delete[] path;
}

bool LocalFileResource::IsScript() const {
	return Util::StringToLower(GetExtension()) == L".tss";
}