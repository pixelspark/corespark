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

ref<WebItem> WebItem::CreateCollection(const String& resource) {
	return null;
}

bool WebItem::Delete(const String& resource) {
	return false;
}

/** WebItemResource **/
WebItemResource::WebItemResource(const String& fn, const String& dn, const String& contentType, unsigned int length): _fn(fn), _dn(dn), _contentType(contentType), _length(length) {
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

unsigned int WebItemResource::GetContentLength() const {
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

Resolution WebItemCollection::Get(ref<WebRequest> frq, String& error, char** data, unsigned int& dataLength) {
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

unsigned int WebItemResolver::GetContentLength() const {
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

Resolution WebItemResolver::Get(tj::shared::ref<WebRequest> frq, tj::shared::String &error, char **data, unsigned int &dataLength) {
	return ResolutionNone;
}