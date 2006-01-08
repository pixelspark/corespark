#include "../include/tjshared.h"
#include <windows.h>

CriticalSection::CriticalSection() {
	InitializeCriticalSection(&_cs);
}

CriticalSection::~CriticalSection() {
	DeleteCriticalSection(&_cs);
}

void CriticalSection::Enter() {
	EnterCriticalSection(&_cs);
}

void CriticalSection::Leave() {
	LeaveCriticalSection(&_cs);
}

ThreadLock::ThreadLock(CriticalSection *cs): _cs(cs) {
	_cs->Enter();
}

ThreadLock::~ThreadLock() {
	_cs->Leave();
}