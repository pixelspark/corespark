#include "../include/tjshared.h"
#include <windows.h>

unsigned int _gc_live = 0;
size_t _gc_size = 0;

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
	_gc_live++;
	_gc_size += size;
}

void GC::DecrementLive(size_t size) {
	_gc_live--;
	_gc_size -= size;
}

int GC::GetLiveCount() {
	return _gc_live;
}

size_t GC::GetSize() {
	return _gc_size;
}