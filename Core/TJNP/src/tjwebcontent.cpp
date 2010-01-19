#include "../include/tjwebcontent.h"
#include <algorithm>

using namespace tj::np;
using namespace tj::shared;

/** WebItemWalker **/
WebItemWalker::~WebItemWalker() {
}

/** WebItem **/
WebItem::~WebItem() {
}

void WebItem::Rename(const String& newName) {
	Throw(L"Cannot rename this web-item", ExceptionTypeError);
}

bool WebItem::Create(const String& resource, ref<WebItem> wi, bool overwrite) {
	return false;
}

ref<WebItem> WebItem::CreateCollection(const String& resource) {
	return null;
}

bool WebItem::Delete(const String& resource) {
	return false;
}

bool WebItem::Put(const String& resource, ref<Data> data) {	
	return false;
}

bool WebItem::Move(const tj::shared::String& from, const tj::shared::String& to, bool copy, bool overwrite) {
	return false;
}

/** WebItemResource **/
WebItemResource::WebItemResource(const String& fn, const String& dn, const String& contentType, Bytes length): _fn(fn), _dn(dn), _contentType(contentType), _length(length) {
	Touch();
	_perms.Set(WebItem::PermissionGet, true);
}

WebItemResource::WebItemResource(const String& contentType): _fn(L""), _dn(L""), _contentType(contentType), _length(0) {
	Touch();
	_perms.Set(WebItem::PermissionGet, true);
}

WebItemResource::~WebItemResource() {
}

void WebItemResource:: Touch() {
	_etag = Util::RandomIdentifier(L'E');
}

void WebItemResource::Rename(const String& newName) {
	_fn = newName;
}

tj::shared::String WebItemResource::GetETag() const {
	return _etag;
}

tj::shared::String WebItemResource::GetDisplayName() const {
	return _dn;
}

tj::shared::String WebItemResource::GetName() const {
	return _fn;
}

tj::shared::String WebItemResource::GetContentType() const {
	return _contentType;
}

Bytes WebItemResource::GetContentLength() const {
	return _length;
}

bool WebItemResource::IsCollection() const {
	return false;
}

void WebItemResource::Walk(tj::shared::strong<WebItemWalker> wiw, const tj::shared::String& prefix, int level) {
}

ref<WebItem> WebItemResource::Resolve(const String &file) {
	if(file.length()==0) {
		return this;
	}
	return null;
}

void WebItemResource::SetPermissions(const Flags<WebItem::Permission>& perms) {
	_perms = perms;
}

Flags<WebItem::Permission> WebItemResource::GetPermissions() const {
	return _perms;
}

/** WebItemCollection **/
WebItemCollection::WebItemCollection(const tj::shared::String& fn, const tj::shared::String& dn, const tj::shared::String& contentType):
	WebItemResource(fn,dn,contentType, 0) {
		Flags<WebItem::Permission> perms;
		perms.Set(WebItem::PermissionGet, true);
		perms.Set(WebItem::PermissionPropertyRead, true);
		SetPermissions(perms);
}

WebItemCollection::~WebItemCollection() {
}

bool WebItemCollection::IsCollection() const {
	return true;
}

void WebItemCollection::Walk(strong<WebItemWalker> wiw, const String& prefix, int level) {
	if(level!=0) {
		std::deque< ref<WebItem> >::iterator it = _children.begin();
		while(it!=_children.end()) {
			ref<WebItem> wi = *it;
			if(wi) {
				wiw->Add(prefix+L"/"+wi->GetName(), wi, level - 1);
			}
			++it;
		}
	}
}

bool WebItemCollection::Delete(const String& resource) {
	// Require Delete permission to create new collections
	if(!GetPermissions().IsSet(WebItem::PermissionDelete)) {
		return false;
	}
	
	String collectionName = resource;
	if(collectionName.length()<1) {
		return false;
	}
	
	if(collectionName.at(0)==L'/') {
		collectionName = collectionName.substr(1);
		if(collectionName.length()<1) {
			return false;
		}
	}
	
	String restOfPath;
	ref<WebItem> wi = GetNextByPath(collectionName, restOfPath);
	
	if(wi) {
		if(restOfPath.length()==0 || restOfPath==L"/") {
			ThreadLock lock(&_lock);
			// This item is directly under me; delete
			std::deque< ref<WebItem> >::iterator it = _children.begin();
			while(it!=_children.end()) {
				if((*it)==wi) {
					_children.erase(it);
					return true;
				}
				else {
					++it;
				}
			}
		}
		else {
			return wi->Delete(restOfPath);
		}
	}
	
	return false;
}

bool WebItemCollection::Move(const String& from, const String& to, bool copy, bool overwrite) {
	if(!GetPermissions().IsSet(WebItem::PermissionPropertyWrite)) {
		return false;
	}

	ThreadLock lock(&_lock);
	ref<WebItem> wi = Resolve(from);
	if(wi) {
		if(copy || Delete(from)) {
			return Create(to, wi, overwrite);
		}
		// Could not delete origin
	}
	// Could not find origin
	return false;
}

bool WebItemCollection::Create(const String& resource, ref<WebItem> wi, bool overwrite) {
	// Require put permission to create new collections
	if(!GetPermissions().IsSet(WebItem::PermissionPut)) {
		return false;
	}

	if(!GetPermissions().IsSet(WebItem::PermissionPropertyWrite)) {
		return false;
	}
	
	String collectionName = resource;
	if(collectionName.length()<1) {
		return false;
	}
	
	if(collectionName.at(0)==L'/') {
		collectionName = collectionName.substr(1);
		if(collectionName.length()<1) {
			return false;
		}
	}
	
	String::const_iterator firstSlash = std::find(collectionName.begin(), collectionName.end(), L'/');
	if(firstSlash!=collectionName.end()) {
		// This is not the final resource name (yet); recurse
		String restOfPath;
		ref<WebItem> wi = GetNextByPath(collectionName, restOfPath);
		if(wi && wi!=ref<WebItem>(this)) {
			return wi->Create(restOfPath, wi, overwrite);
		}
	}
	else {
		// The resource to be put onto is right under me; see if it exists already
		ThreadLock lock(&_lock);
		std::deque< ref<WebItem> >::iterator it = _children.begin();
		while(it!=_children.end()) {
			ref<WebItem> wi = *it;
			if(wi) {
				if(wi->GetName()==collectionName) {
					if(overwrite) {
						_children.erase(it);
						_children.push_back(wi);
						return true;
					}
					else {
						return false;
					}
				}
			}
			++it;
		}

		wi->Rename(collectionName);
		_children.push_back(wi);
		return true;
	}
	
	return false;
}

bool WebItemCollection::Put(const String& resource, ref<Data> data) {
	// Require put permission to create new collections
	if(!GetPermissions().IsSet(WebItem::PermissionPut)) {
		return false;
	}
	
	String collectionName = resource;
	if(collectionName.length()<1) {
		return null;
	}
	
	if(collectionName.at(0)==L'/') {
		collectionName = collectionName.substr(1);
		if(collectionName.length()<1) {
			return false;
		}
	}
	
	String::const_iterator firstSlash = std::find(collectionName.begin(), collectionName.end(), L'/');
	if(firstSlash!=collectionName.end()) {
		// This is not the final resource name (yet); recurse
		String restOfPath;
		ref<WebItem> wi = GetNextByPath(collectionName, restOfPath);
		if(wi && wi!=ref<WebItem>(this)) {
			if(wi->Put(restOfPath, data)) {
				Touch();
				return true;
			}
			return false;
		}
	}
	else {
		// The resource to be put onto is right under me; see if it exists already
		ThreadLock lock(&_lock);
		std::deque< ref<WebItem> >::iterator it = _children.begin();

		while(it!=_children.end()) {
			ref<WebItem> wi = *it;
			if(wi) {
				if(wi->GetName()==collectionName) {
					if(wi->Put(collectionName, data)) {
						Touch();
						return true;
					}
					return false;
				}
			}
			++it;
		}

		// File not found; create the item as resource in this collection if we have PropertyWrite permissions
		if(!GetPermissions().IsSet(WebItem::PermissionPropertyWrite)) {
			return false;
		}

		// TODO: use the Content-Type from the request, if any (add to ::Put function as parameter)
		ref<WebItemDataResource> wd = GC::Hold(new WebItemDataResource(collectionName, collectionName, L"application/octet-stream", data));
		wd->SetPermissions(GetPermissions()); // Newly created resource inherits permissions
		_children.push_back(wd);
		Touch();
		return true;
	}
	
	return false;
}

ref<WebItem> WebItemCollection::CreateCollection(const String& resource) {
	// Require PropertyWrite permission to create new collections
	if(!GetPermissions().IsSet(WebItem::PermissionPropertyWrite)) {
		return null;
	}
	
	String collectionName = resource;
	if(collectionName.length()<1) {
		return null;
	}
	
	if(collectionName.at(0)==L'/') {
		collectionName = collectionName.substr(1);
		if(collectionName.length()<1) {
			return null;
		}
	}
	
	String::const_iterator firstSlash = std::find(collectionName.begin(), collectionName.end(), L'/');
	if(firstSlash!=collectionName.end()) {
		// This is not the final resource name (yet); recurse
		String restOfPath;
		ref<WebItem> wi = GetNextByPath(collectionName, restOfPath);
		if(wi && wi!=ref<WebItem>(this)) {
			return wi->CreateCollection(restOfPath);
		}
	}
	else {
		// The collection has to be created directly under me
		ThreadLock lock(&_lock);
		ref<WebItemCollection> wcol = GC::Hold(new WebItemCollection(collectionName, collectionName, L""));
		wcol->SetPermissions(GetPermissions()); // Newly created collection inherits permissions
		_children.push_back(wcol);
		return wcol;
	}
	
	return null;
}

Resolution WebItemCollection::Get(ref<WebRequest> frq, String& error, char** data, Bytes& dataLength) {
	return ResolutionEmpty;
}

ref<WebItem> WebItemCollection::GetNextByPath(const String& file, String& restOfPath) {
	String::const_iterator beginOfFirstFile = file.begin();
	if(beginOfFirstFile!=file.end() && *beginOfFirstFile==L'/') {
		beginOfFirstFile++;
	}
	
	String::const_iterator endOfFirstFile = std::find(beginOfFirstFile, file.end(), L'/');
	String firstFile(beginOfFirstFile, endOfFirstFile);
	restOfPath = String(endOfFirstFile, file.end());
	
	if(firstFile.length()==0) {
		return this;
	}
	
	std::deque< ref<WebItem> >::iterator it = _children.begin();
	while(it!=_children.end()) {
		ref<WebItem> wi = *it;
		if(wi) {
			if(wi->GetName()==firstFile) {
				return wi;
			}
		}
		++it;
	}
	
	return null;
}

ref<WebItem> WebItemCollection::Resolve(const String& file) {
	String restOfPath;
	ref<WebItem> wi = GetNextByPath(file,restOfPath);
	if(wi && wi!=ref<WebItem>(this)) {
		wi = wi->Resolve(restOfPath);
	}
	return wi;
}

void WebItemCollection::Add(ref<WebItem> item) {
	_children.push_back(item);
}

/** WebItemResolver **/
WebItemResolver::~WebItemResolver() {
}

void WebItemResolver::Touch() {
}

String WebItemResolver::GetETag() const {
	return L"";
}

String WebItemResolver::GetDisplayName() const {
	return L"";
}

String WebItemResolver::GetName() const {
	return L"";
}

String WebItemResolver::GetContentType() const {
	return L"";
}

Bytes WebItemResolver::GetContentLength() const {
	return 0;
}

bool WebItemResolver::IsCollection() const {
	return true;
}

void WebItemResolver::Walk(tj::shared::strong<WebItemWalker> wiw, const tj::shared::String& prefix, int level) {
}

Flags<WebItem::Permission> WebItemResolver::GetPermissions() const {
	return Flags<WebItem::Permission>(WebItem::PermissionGet);
}

Resolution WebItemResolver::Get(tj::shared::ref<WebRequest> frq, tj::shared::String &error, char **data, Bytes& dataLength) {
	return ResolutionNone;
}

/** WebItemDataResource **/
WebItemDataResource::WebItemDataResource(const String& fn, const String& dn, const String& contentType, strong<Data> data):
	WebItemResource(fn,dn,contentType, 0), _dataLength(0), _data(0) {
		SetData(data);
}

WebItemDataResource::~WebItemDataResource() {
	delete[] _data;
}

Resolution WebItemDataResource::Get(tj::shared::ref<WebRequest> frq, tj::shared::String& error, char** data, Bytes& dataLength) {
	if(!GetPermissions().IsSet(WebItem::PermissionGet)) {
		return ResolutionPermissionDenied;
	}

	if(_data==0) {
		return ResolutionNone;
	}
	*data = new char[(unsigned int)_dataLength];
	memcpy(*data, _data, (size_t)_dataLength);
	dataLength = _dataLength;
	return ResolutionData;
}

bool WebItemDataResource::Put(const tj::shared::String& resource, ref<Data> data) {
	if(!GetPermissions().IsSet(WebItem::PermissionPut)) {
		return false;
	}

	Touch();
	if(data) {
		SetData(data);
	}
	else {
		delete[] _data;
		_data = 0;
		_dataLength = 0;
	}
	return true;
}

Bytes WebItemDataResource::GetContentLength() const {
	if(_data==0) {
		return 0;
	}
	return _dataLength;
}

void WebItemDataResource::SetData(strong<Data> cw) {
	if(_data!=0) {
		delete[] _data;
	}
	_dataLength = cw->GetSize();
	_data = cw->TakeOverBuffer(false);
}