#include "../include/tjshared.h"
#include <windows.h>

unsigned int GC::_live = 1;
GC _gcInstance; // for checking our 'live' value

GC::GC() {
}

GC::~GC() {
	if(GC::_live>0) {
		MessageBox(0L, L"One or more objects were not correctly deleted.", L"GC Warning", MB_ICONWARNING|MB_OK);
	}
}