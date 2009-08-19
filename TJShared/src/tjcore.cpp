#include "../include/tjshared.h"

#ifdef TJ_OS_MAC
	#include <CoreFoundation/CoreFoundation.h>
#endif

using namespace tj::shared;

volatile ReferenceCount intern::Resource::_resourceCount = 0L;

/* GC */
void GC::Log(const char* tp, bool allocate) {
	#ifdef TJ_OS_WIN
		if(allocate) {
				OutputDebugString(L"A ");
		}
		else {
			OutputDebugString(L"D ");
		}

		OutputDebugStringA(tp);
		OutputDebugString(L"\r\n");
	#endif
	
	#ifdef TJ_OS_MAC
		std::cout << (allocate?"A":"D") << " " << tp << std::endl;
	#endif
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
	#ifdef TJ_OS_WIN
		InterlockedIncrement(&_resourceCount);
	#endif
	
	#ifdef TJ_OS_MAC
		OSAtomicAdd32(1, &_resourceCount);
	#endif
}

intern::Resource::~Resource() {
	#ifdef TJ_OS_WIN
		InterlockedDecrement(&_resourceCount);
	#endif
	
	#ifdef TJ_OS_MAC
		OSAtomicAdd32(-1, &_resourceCount);
	#endif
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