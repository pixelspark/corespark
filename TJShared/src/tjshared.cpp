#include "../include/tjshared.h"
using namespace tj::shared;
using namespace tj::shared::intern;

volatile long _gc_live = 0;
volatile long _gc_size = 0;

/* GC */
void GC::IncrementLive(size_t size) {
	InterlockedIncrement(&_gc_live);
	InterlockedExchangeAdd(&_gc_size, long(size));
}

void GC::DecrementLive(size_t size) {
	InterlockedDecrement(&_gc_live);
	InterlockedExchangeAdd(&_gc_size, -long(size));
}

long GC::GetLiveCount() {
	return _gc_live;
}

long GC::GetSize() {
	return _gc_size;
}

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

/* Listener */
Listener::~Listener() {
}

/* Object */
void Object::OnCreated() {
}