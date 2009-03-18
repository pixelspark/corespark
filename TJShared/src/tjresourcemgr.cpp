#include "../include/tjcore.h"
#include <shlwapi.h>
#include <commctrl.h>
#include <shellapi.h>
#include <winioctl.h>
using namespace tj::shared;

ref<ResourceManager> ResourceManager::_instance;

/** ResourceProvider **/
ResourceProvider::~ResourceProvider() {
}

bool ResourceProvider::GetPathToLocalResource(const ResourceIdentifier& rid, String& path) {
	return false;
}

/** AbsoluteLocalFileResourceProvider **/
AbsoluteLocalFileResourceProvider::AbsoluteLocalFileResourceProvider() {
}

AbsoluteLocalFileResourceProvider::~AbsoluteLocalFileResourceProvider() {
}

ref<Resource> AbsoluteLocalFileResourceProvider::GetResource(const ResourceIdentifier& rid) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	return GC::Hold(new LocalFileResource(rid, rid));
}

bool AbsoluteLocalFileResourceProvider::GetPathToLocalResource(const ResourceIdentifier& rid, String& path) {	
	if(!Zones::Get(Zones::LocalFileInfoZone).CanEnter()) {
		return false;
	}

	ZoneEntry ze(Zones::LocalFileInfoZone);
	path = rid;
	return true;
}

ResourceIdentifier AbsoluteLocalFileResourceProvider::GetRelative(const String& path) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	return path;
}


/** LocalFileResourceProvider **/
LocalFileResourceProvider::LocalFileResourceProvider(const String& searchPath): _searchPath(searchPath) {
}

LocalFileResourceProvider::~LocalFileResourceProvider() {
}

ref<Resource> LocalFileResourceProvider::GetResource(const ResourceIdentifier& rid) {
	if(!Zones::Get(Zones::LocalFileInfoZone).CanEnter()) {
		return null;
	}

	ZoneEntry ze(Zones::LocalFileInfoZone);

	String path;
	if(GetPathToLocalResource(rid, path)) {
		return GC::Hold(new LocalFileResource(rid, path));
	}
	return null;
}

bool LocalFileResourceProvider::GetPathToLocalResource(const ResourceIdentifier& rid, String& path) {	
	if(!Zones::Get(Zones::LocalFileInfoZone).CanEnter()) {
		return false;
	}

	ZoneEntry ze(Zones::LocalFileInfoZone);
	String myPath = _searchPath + File::PathSeparator + rid;

	// check if that file exists
	if(GetFileAttributes(myPath.c_str())!=INVALID_FILE_ATTRIBUTES) {
		path = myPath;
		return true;
	}
	return false;
}

ResourceIdentifier LocalFileResourceProvider::GetRelative(const String& path) {
	if(!Zones::Get(Zones::LocalFileInfoZone).CanEnter()) {
		return L"";
	}

	ZoneEntry ze(Zones::LocalFileInfoZone);

	wchar_t relativePath[MAX_PATH+3];
	if(PathRelativePathTo(relativePath, _searchPath.c_str(), FILE_ATTRIBUTE_DIRECTORY, path.c_str(), FILE_ATTRIBUTE_NORMAL)==TRUE) {
		// Windows sometimes adds './' in front of relative paths, remove it
		if(relativePath[0]==L'.' && relativePath[1]==L'\\') {
			return ResourceIdentifier((const wchar_t*)&(relativePath[2]));
		} 
	}

	return L"";
}

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
}

void ResourceManager::AddProvider(strong<ResourceProvider> rp, bool upFront) {
	if(upFront) {
		_paths.push_front(rp);
	}
	else {
		_paths.push_back(rp);
	}
}

void ResourceManager::RemoveProvider(strong<ResourceProvider> rp) {
	std::remove(_paths.begin(), _paths.end(), rp);
}

ResourceIdentifier ResourceManager::GetRelative(const String& path) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	
	std::deque< strong<ResourceProvider> >::iterator it = _paths.begin();
	while(it!=_paths.end()) {
		strong<ResourceProvider> rp = *it;
		ResourceIdentifier rel = rp->GetRelative(path);
		if(rel.length()>0) {
			return rel;
		}
		++it;
	}

	return L"";
}

ref<Resource> ResourceManager::GetResource(const ResourceIdentifier& ident) {
	std::deque< strong<ResourceProvider> >::iterator it = _paths.begin();
	while(it!=_paths.end()) {
		strong<ResourceProvider> rp = *it;
		ref<Resource> rs = rp->GetResource(ident);
		if(rs) {
			return rs;
		}
		++it;
	}

	Log::Write(L"TJShared/ResourceManager", L"Resource not found: rid="+ident);
	return null;
}

bool ResourceManager::GetPathToLocalResource(const ResourceIdentifier& rid, String& path) {
	std::deque< strong<ResourceProvider> >::iterator it = _paths.begin();

	while(it!=_paths.end()) {
		strong<ResourceProvider> rp = *it;
		
		if(rp->GetPathToLocalResource(rid, path)) {
			return true;
		}
		++it;
	}

	Log::Write(L"TJShared/ResourceManager", L"Resource not found locally: rid="+rid);
	return false;
}

strong<ResourceManager> ResourceManager::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ResourceManager());

		/** Add location of tjshared.dll as provider **/
		wchar_t* buf = new wchar_t[MAX_PATH];
		memset(buf,0,sizeof(wchar_t)*MAX_PATH);
		GetModuleFileName(GetModuleHandle(NULL), buf, MAX_PATH);
		PathRemoveFileSpec(buf);
		_instance->AddProvider(strong<ResourceProvider>(GC::Hold(new LocalFileResourceProvider(buf))));
	}

	return _instance;
}

/* ResourceBundle */
ResourceBundle::ResourceBundle(strong<ResourceManager> mgr, strong<ResourceProvider> rp, bool upFront): _mgr(mgr), _rp(rp) {
	_mgr->AddProvider(rp, upFront);
}

ResourceBundle::~ResourceBundle() {
	_mgr->RemoveProvider(_rp);
}

/* Resource */
Resource::~Resource() {
}

void Resource::Save(TiXmlElement* el) {
	SaveAttributeSmall(el, "rid", GetIdentifier());
}

/** LocalFileResource **/
LocalFileResource::LocalFileResource(const ResourceIdentifier& rid, const String& path): _rid(rid), _path(path), _cachedSize(0) {
}

LocalFileResource::~LocalFileResource() {
}

void LocalFileResource::Save(TiXmlElement* el) {
	Resource::Save(el);
}

bool LocalFileResource::Exists() const {
	if(GetFileAttributes(_path.c_str())!=INVALID_FILE_ATTRIBUTES) {
		return true;
	}

	return false;
}

String LocalFileResource::GetExtension() const {
	return String(PathFindExtension(_path.c_str()));
}

ResourceIdentifier LocalFileResource::GetIdentifier() const {
	return _rid;
}

String LocalFileResource::GetPath() const {
	return _path;
}

void LocalFileResource::Open() const {
	ShellExecute(NULL, L"open", _path.c_str(), L"", L"", SW_SHOW);
}

Bytes LocalFileResource::GetSize() {
	if(_cachedSize!=0) {
		return _cachedSize;
	}

	HANDLE hdl = CreateFile(_path.c_str(), FILE_READ_ACCESS, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0L, 0L);
	LARGE_INTEGER size;
	GetFileSizeEx(hdl, &size);
	_cachedSize = size.QuadPart;
	CloseHandle(hdl);
	return _cachedSize;
}

void LocalFileResource::OpenFolder() const {
	wchar_t* path = _wcsdup(_path.c_str());
	PathRemoveFileSpec(path);
	ShellExecute(NULL, L"explore", path, L"", L"", SW_SHOW);
	delete[] path;
}

bool LocalFileResource::IsScript() const {
	return Util::StringToLower(GetExtension()) == L".tss";
}