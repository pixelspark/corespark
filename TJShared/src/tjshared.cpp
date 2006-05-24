#include "../include/tjshared.h"
using namespace tj::shared;

volatile long _gc_live = 0;
volatile long _gc_size = 0;

void _gc_check() {
#ifdef _DEBUG
	if(_gc_live>0) {
		MessageBox(0L, L"There are still objects left in memory!", L"TJShared GC", MB_OK|MB_ICONWARNING);
	}
#endif
}

class GCChecker {
	public:
		GCChecker() {
			atexit(_gc_check);
		}
};

GCChecker _gc_checker;

std::map< void* , std::wstring> GC::_objects;

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

void GC::AddLog(void* id, std::wstring info) {
#ifdef _DEBUG
	_objects[(void*)id] = info;
#endif
}

void GC::RemoveLog(void* id) {
#ifdef _DEBUG	
	std::map<void*, std::wstring>::iterator it = _objects.find(id);
	if(it!=_objects.end()) {
		_objects.erase(it);
		return;
	}
#endif
}