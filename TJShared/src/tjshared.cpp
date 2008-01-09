#include "../include/tjshared.h"
using namespace tj::shared;
using namespace tj::shared::intern;

volatile long Resource::_resourceCount = 0L;

/* GC */
void GC::Log(const char* tp, bool allocate) {
	if(allocate) {
		OutputDebugStringA("A ");
	}
	else {
		OutputDebugStringA("D ");
	}
	OutputDebugStringA(tp);
	OutputDebugStringA("\r\n");
}

/* Endpoint */
Endpoint::EndpointType Endpoint::GetType() const {
	return EndpointTypeThreaded;
}

Endpoint::~Endpoint() {
}

std::wstring Endpoint::GetName() const {
	return TL(endpoint_unknown);
}

/* Resource */
Resource::Resource(): _referenceCount(0), _weakReferenceCount(0) {
	InterlockedIncrement(&_resourceCount);
}

Resource::~Resource() {
	InterlockedDecrement(&_resourceCount);
}

long Resource::GetResourceCount() {
	return _resourceCount;
}

/* Listener */
Listener::~Listener() {
}

/* Object */
void Object::OnCreated() {
}

/* Serializable */
Serializable::~Serializable() {
}