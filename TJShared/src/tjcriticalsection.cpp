#include "../include/tjshared.h"
using namespace tj::shared;

CriticalSection::CriticalSection() {
	InitializeCriticalSectionAndSpinCount(&_cs, 2);
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