#include "../include/tjshared.h"
using namespace tj::shared;

volatile long _gc_live = 0;
volatile long _gc_size = 0;

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
