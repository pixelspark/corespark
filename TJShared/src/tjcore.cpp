#include "../include/tjcore.h"
using namespace tj::shared;

volatile long intern::Resource::_resourceCount = 0L;

/* GC */
void GC::Log(const char* tp, bool allocate) {
	if(allocate) {
		OutputDebugString(L"A ");
	}
	else {
		OutputDebugString(L"D ");
	}

	OutputDebugStringA(tp);
	OutputDebugString(L"r\n");
}

/* Endpoint */
Endpoint::EndpointType Endpoint::GetType() const {
	return EndpointTypeThreaded;
}

Endpoint::~Endpoint() {
}

String Endpoint::GetName() const {
	return TL(endpoint_unknown);
}

/* Resource */
intern::Resource::Resource(): _referenceCount(0), _weakReferenceCount(0) {
	InterlockedIncrement(&_resourceCount);
}

intern::Resource::~Resource() {
	InterlockedDecrement(&_resourceCount);
}

long intern::Resource::GetResourceCount() {
	return _resourceCount;
}

/* Object */
void Object::OnCreated() {
}

/* Serializable */
Serializable::~Serializable() {
}

OutOfMemoryException::OutOfMemoryException(): Exception(L"Out of memory!", ExceptionTypeError) {
}