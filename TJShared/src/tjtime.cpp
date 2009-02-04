#include "../include/tjcore.h"
#include <time.h>
using namespace tj::shared;

/* Date */
std::wstring Date::GetFriendlyMonthName(Month m) {
	static const wchar_t* localeKeys[12] = {L"month_january", L"month_february", L"month_march", 
											L"month_april", L"month_may", L"month_june", L"month_july",
											L"month_august", L"month_september", L"month_october",
											L"month_november", L"month_december" };

	if(m >= 1 && m <= 12) {
		return Language::Get(localeKeys[m-1]);
	}
	return L"";
}

std::wstring Date::GetFriendlyDayName(DayOfWeek m) {
	static const wchar_t* localeKeys[7] = {L"day_monday", L"day_tuesday", L"day_wednesday", L"day_thursday", 
											L"day_friday", L"day_saturday", L"day_sunday"};

	if(m >= 0 && m <= 6) {
		return Language::Get(localeKeys[m]);
	}
	return L"";
}

Date::Date() {
	GetLocalTime(&_time);
}

Date::~Date() {
}

Month Date::GetMonth() const {
	return (Month)_time.wMonth;
}

DayOfWeek Date::GetDayOfWeek() const {
	return (DayOfWeek)_time.wDayOfWeek;
}

DayOfMonth Date::GetDayOfMonth() const {
	return (DayOfMonth)_time.wDay;
}
Year Date::GetYear() const {
	return (Year)_time.wYear;
}

Seconds Date::GetSeconds() const {
	return (Seconds)_time.wSecond;
}

Minutes Date::GetMinutes() const {
	return (Minutes)_time.wMinute;
}

Hours Date::GetHours() const {
	return (Hours)_time.wHour;
}

std::wstring Date::ToFriendlyString() const {
	wchar_t time[255];
	memset(time, 0, sizeof(wchar_t)*255);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &_time, NULL, time, 253);
	return std::wstring(time);
}

/* Timestamp */
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