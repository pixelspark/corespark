#include "../include/tjcore.h"

#ifdef TJ_OS_WIN
	#include <shlwapi.h>
	#include <commctrl.h>
	#include <shellapi.h>
	#include <winioctl.h>
#endif

#ifdef TJ_OS_MAC
	#include <CoreFoundation/CFBundle.h>
	#include <unistd.h>
	#include <limits.h>
	#include <libgen.h>
	#include <sys/stat.h>
#endif

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
	
	if(File::Exists(myPath)) {
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

	#ifdef TJ_OS_WIN
		wchar_t relativePath[MAX_PATH+3];
		if(PathRelativePathTo(relativePath, _searchPath.c_str(), FILE_ATTRIBUTE_DIRECTORY, path.c_str(), FILE_ATTRIBUTE_NORMAL)==TRUE) {
			// Windows sometimes adds './' in front of relative paths, remove it
			if(relativePath[0]==L'.' && relativePath[1]==L'\\') {
				return ResourceIdentifier((const wchar_t*)&(relativePath[2]));
			} 
		}
	#endif
	
	#ifdef TJ_OS_MAC
		#warning Not implemented on Mac
	#endif
	
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

		#ifdef TJ_OS_WIN
			/** Add location of tjshared.dll as provider **/
			wchar_t* buf = new wchar_t[MAX_PATH];
			memset(buf,0,sizeof(wchar_t)*MAX_PATH);
			GetModuleFileName(GetModuleHandle(NULL), buf, MAX_PATH);
			PathRemoveFileSpec(buf);
			_instance->AddProvider(strong<ResourceProvider>(GC::Hold(new LocalFileResourceProvider(buf))));
		#endif
		
		#ifdef TJ_OS_MAC
			char resourcePath[PATH_MAX+1];
			CFBundleRef mainBundle = CFBundleGetMainBundle();
			CFURLRef resourcesDirectoryURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
			CFURLGetFileSystemRepresentation(resourcesDirectoryURL, true, (UInt8 *) resourcePath, PATH_MAX);
			CFRelease(resourcesDirectoryURL);
			_instance->AddProvider(strong<ResourceProvider>(GC::Hold(new LocalFileResourceProvider(Wcs(std::string(resourcePath))))));
		#endif
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
	return File::Exists(_path);
}

String LocalFileResource::GetExtension() const {
	#ifdef TJ_OS_WIN
		return String(PathFindExtension(_path.c_str()));
	#endif
	
	#ifdef TJ_OS_MAC
		std::string mbsPath = Mbs(_path);
		std::string baseName(basename(const_cast<char*>(mbsPath.c_str())));
		return Wcs(std::string(baseName,baseName.find_last_of(L'.')+1));
	#endif
}

ResourceIdentifier LocalFileResource::GetIdentifier() const {
	return _rid;
}

String LocalFileResource::GetPath() const {
	return _path;
}

void LocalFileResource::Open() const {
	#ifdef TJ_OS_WIN
		ShellExecute(NULL, L"open", _path.c_str(), L"", L"", SW_SHOW);
	#endif
	
	#ifdef TJ_OS_MAC
		#warning Not implemented on Mac
	#endif
}

Bytes LocalFileResource::GetSize() {
	if(_cachedSize!=0) {
		return _cachedSize;
	}

	#ifdef TJ_OS_WIN
		HANDLE hdl = CreateFile(_path.c_str(), FILE_READ_ACCESS, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0L, 0L);
		LARGE_INTEGER size;
		GetFileSizeEx(hdl, &size);
		_cachedSize = size.QuadPart;
		CloseHandle(hdl);
		return _cachedSize;
	#endif
	
	#ifdef TJ_OS_MAC
		struct stat64 sts;
		std::string mbsPath = Mbs(_path);
		stat64(mbsPath.c_str(), &sts);
		return sts.st_size;
		
	#endif
}

void LocalFileResource::OpenFolder() const {
	#ifdef TJ_OS_WIN
		wchar_t* path = _wcsdup(_path.c_str());
		PathRemoveFileSpec(path);
		ShellExecute(NULL, L"explore", path, L"", L"", SW_SHOW);
		delete[] path;
	#endif
	
	#ifdef TJ_OS_MAC
		#warning Not implemented on Mac
	#endif
}

bool LocalFileResource::IsScript() const {
	// TODO: doesn't GetExtension return just tss?
	String ext = GetExtension();
	return Util::StringToLower(ext) == L".tss";
}