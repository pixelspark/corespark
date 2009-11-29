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

/** WebItemResource **/
WebItemResource::WebItemResource(const tj::shared::String& fn, const tj::shared::String& dn, const tj::shared::String& contentType, unsigned int length): _fn(fn), _dn(dn), _contentType(contentType), _length(length) {
	Touch();
}

WebItemResource::WebItemResource(const tj::shared::String& contentType): _fn(L""), _dn(L""), _contentType(contentType), _length(0) {
	Touch();
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

bool WebItemResource::IsEditable() const {
	return false;
}

/** WebItemCollection **/
WebItemCollection::WebItemCollection(const tj::shared::String& fn, const tj::shared::String& dn, const tj::shared::String& contentType):
	WebItemResource(fn,dn,contentType, 0) {
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

Resolution WebItemCollection::Get(ref<WebRequest> frq, String& error, char** data, unsigned int& dataLength) {
	return ResolutionEmpty;
}

ref<WebItem> WebItemCollection::Resolve(const String& file) {
	String::const_iterator beginOfFirstFile = file.begin();
	if(beginOfFirstFile!=file.end() && *beginOfFirstFile==L'/') {
		beginOfFirstFile++;
	}

	String::const_iterator endOfFirstFile = std::find(beginOfFirstFile, file.end(), L'/');
	String firstFile(beginOfFirstFile, endOfFirstFile);
	String restOfPath(endOfFirstFile, file.end());

	if(firstFile.length()==0) {
		return this;
	}

	std::deque< ref<WebItem> >::iterator it = _children.begin();
	while(it!=_children.end()) {
		ref<WebItem> wi = *it;
		if(wi && wi->GetName()==firstFile) {
			return wi->Resolve(restOfPath);
		}
		++it;
	}
	
	return null;
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

bool WebItemResolver::IsEditable() const {
	return false;
}

Resolution WebItemResolver::Get(tj::shared::ref<WebRequest> frq, tj::shared::String &error, char **data, unsigned int &dataLength) {
	return ResolutionNone;
}