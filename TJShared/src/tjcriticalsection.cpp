#include "../include/tjcore.h"
using namespace tj::shared;

CriticalSection::CriticalSection() {
	#ifdef _WIN32
		InitializeCriticalSectionAndSpinCount(&_cs, 1024);
	#endif
}

CriticalSection::~CriticalSection() {
	#ifdef _WIN32
		DeleteCriticalSection(&_cs);
	#endif
}

void CriticalSection::Enter() {
	#ifdef _WIN32
		EnterCriticalSection(&_cs);
	#else
		#error CriticalSection::Enter not implemented on this platform
	#endif
}

void CriticalSection::Leave() {
	#ifdef _WIN32
		LeaveCriticalSection(&_cs);
	#else
		#error CriticalSection::Enter not implemented on this platform
	#endif
}

ThreadLock::ThreadLock(CriticalSection *cs): _cs(cs) {
	_cs->Enter();
}

ThreadLock::~ThreadLock() {
	_cs->Leave();
}