#include "../include/tjshared.h"
#include <windows.h>

volatile long _gc_live = 0;
volatile long _gc_size = 0;

class GCChecker {
	public:
		GCChecker() {
		}

		virtual ~GCChecker() {
			if(_gc_live>0) {
				MessageBox(0L, L"One or more objects were not correctly deleted. This is a problem in the program and does not affect your data. Probably it's just poor coding done by some programmer :-) You can safely ignore this message.", L"Error", MB_OK|MB_ICONWARNING);
			}
		}
};

GCChecker _gc_checker;

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