#include "../include/tjcore.h"
using namespace tj::shared;

Timestamp::Timestamp(const Timestamp& t) {
	_time = t._time;
}

Timestamp::~Timestamp() {
}

Timestamp::Timestamp(bool now) {
	if(now) {
		Now();
	}
	else {
		_time = 0;
	}
}

void Timestamp::Now() {
	if(QueryPerformanceCounter((LARGE_INTEGER*)&_time)==FALSE) {
		Log::Write(L"TJShow/Timestamp", L"Performance counter does not work, reverting to tickcount");
		_time = GetTickCount();
	}
}

std::wstring Timestamp::ToString() {
	return Stringify(ToMicroSeconds());
}

Timestamp& Timestamp::operator =(const Timestamp& o) {
	_time = o._time;
	return *this;
}

Timestamp Timestamp::Difference(const Timestamp& other) const {
	Timestamp t;
	if(other._time>_time) {
		t._time = other._time - _time;
	}
	else {
		t._time = _time - other._time;
	}
	return t;
}

long long Timestamp::ToMicroSeconds() const {
	LARGE_INTEGER freq;
	if(QueryPerformanceFrequency(&freq)==FALSE) {
		// Tickcount used, so our frequency would be 1000
		freq.QuadPart = 1000;
	}

	long long ms = (1000*1000*_time) / freq.QuadPart;
	return ms;
}

long double Timestamp::ToMilliSeconds() const {
	LARGE_INTEGER freq;
	if(QueryPerformanceFrequency(&freq)==FALSE) {
		// Tickcount used, so our frequency would be 1000
		freq.QuadPart = 1000;
	}

	long long us = (1000*1000*_time) / freq.QuadPart;
	return (long double)us/(long double)1000.0;
}

bool Timestamp::operator>(const Timestamp& o) const {
	return _time > o._time;
}

bool Timestamp::operator<(const Timestamp& o) const {
	return _time < o._time;
}