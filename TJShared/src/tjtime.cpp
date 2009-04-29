#include "../include/tjcore.h"
#include <time.h>

#ifdef TJ_OS_MAC
	#include <CoreFoundation/CFDate.h>
	#include <CoreFoundation/CFTimeZone.h>
	#include <CoreFoundation/CFDateFormatter.h>
#endif

using namespace tj::shared;

/* Date */
String Date::GetFriendlyMonthName(Month m) {
	static const wchar_t* localeKeys[12] = {L"month_january", L"month_february", L"month_march", 
											L"month_april", L"month_may", L"month_june", L"month_july",
											L"month_august", L"month_september", L"month_october",
											L"month_november", L"month_december" };

	if(m >= 1 && m <= 12) {
		return Language::Get(localeKeys[m-1]);
	}
	return L"";
}

String Date::GetFriendlyDayName(DayOfWeek m) {
	static const wchar_t* localeKeys[7] = {L"day_monday", L"day_tuesday", L"day_wednesday", L"day_thursday", 
											L"day_friday", L"day_saturday", L"day_sunday"};
	return Language::Get(localeKeys[m]);
}

Date::Date() {
	#ifdef TJ_OS_WIN
		GetLocalTime(&_time);
	#endif
	
	#ifdef TJ_OS_MAC
		// _time is relative to januari 1, 2001 00:00 GMT
		_time = CFAbsoluteTimeGetCurrent();
	#endif
}

Date::~Date() {
}

Month Date::GetMonth() const {
	// Caution, Month is 1-based (e.g. 1...12, januari is 1)
	#ifdef TJ_OS_MAC
		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		CFGregorianDate date = CFAbsoluteTimeGetGregorianDate(_time, tz);
		CFRelease(tz);
		return (Month)date.month;
	#endif
	
	#ifdef TJ_OS_WIN
		return (Month)_time.wMonth;
	#endif
}

DayOfWeek Date::GetDayOfWeek() const {
	// Caution, DayOfWeek is 0..7, where 0 is monday
	#ifdef TJ_OS_MAC
		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		DayOfWeek dow = CFAbsoluteTimeGetDayOfWeek(_time, tz)-1;
		CFRelease(tz);
		return dow;
	#endif

	#ifdef TJ_OS_WIN
		return (DayOfWeek)_time.wDayOfWeek;
	#endif
}

DayOfMonth Date::GetDayOfMonth() const {
	// DayOfMonth is 1-based
	#ifdef TJ_OS_MAC
		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		CFGregorianDate date = CFAbsoluteTimeGetGregorianDate(_time, tz);
		CFRelease(tz);
		return (DayOfMonth)date.day;
	#endif
	
	#ifdef TJ_OS_WIN
		return (DayOfMonth)_time.wDay;
	#endif
}
Year Date::GetYear() const {
	#ifdef TJ_OS_MAC
		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		CFGregorianDate date = CFAbsoluteTimeGetGregorianDate(_time, tz);
		CFRelease(tz);
		return (Year)date.year;
	#endif
	
	#ifdef TJ_OS_WIN
		return (Year)_time.wYear;
	#endif
}

Seconds Date::GetSeconds() const {
	#ifdef TJ_OS_MAC
		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		CFGregorianDate date = CFAbsoluteTimeGetGregorianDate(_time, tz);
		CFRelease(tz);
		return (Seconds)date.second;
	#endif
	
	#ifdef TJ_OS_WIN
		return (Seconds)_time.wSecond;
	#endif
}

Minutes Date::GetMinutes() const {
	#ifdef TJ_OS_MAC
		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		CFGregorianDate date = CFAbsoluteTimeGetGregorianDate(_time, tz);
		CFRelease(tz);
		return (Minutes)date.minute;
	#endif
	
	#ifdef TJ_OS_WIN
		return (Minutes)_time.wMinute;
	#endif
}

Hours Date::GetHours() const {
	#ifdef TJ_OS_MAC
		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		CFGregorianDate date = CFAbsoluteTimeGetGregorianDate(_time, tz);
		CFRelease(tz);
		return (Seconds)date.hour;
	#endif
		
	#ifdef TJ_OS_WIN	
		return (Hours)_time.wHour;
	#endif
}

String Date::ToFriendlyString() const {
	#ifdef TJ_OS_WIN
		wchar_t time[255];
		memset(time, 0, sizeof(wchar_t)*255);
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &_time, NULL, time, 253);
		return String(time);
	#endif
	
	#ifdef TJ_OS_MAC
		CFLocaleRef locale = CFLocaleCopyCurrent();
		CFDateFormatterRef frm = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterShortStyle);
		CFStringRef str = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, frm, _time);
		String formatted = Util::MacStringToString(str);
		CFRelease(str);
		CFRelease(locale);
		CFRelease(frm);
		return formatted;
	#endif
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
	#ifdef TJ_OS_MAC
		_time = (long long)CFAbsoluteTimeGetCurrent();
	#endif
	
	#ifdef TJ_OS_WIN
		if(QueryPerformanceCounter((LARGE_INTEGER*)&_time)==FALSE) {
			Log::Write(L"TJShow/Timestamp", L"Performance counter does not work, reverting to tickcount");
			_time = GetTickCount();
		}
	#endif
}

String Timestamp::ToString() const {
	return Stringify(ToMicroSeconds());
}

String Timestamp::ToHexString() const {
	return StringifyHex(ToMicroSeconds());
}

Timestamp& Timestamp::operator =(const Timestamp& o) {
	_time = o._time;
	return *this;
}

Timestamp Timestamp::Increment(const Time& t) const {
	Timestamp ret(false);

	#ifdef TJ_OS_WIN
		LARGE_INTEGER freq;
		if(QueryPerformanceFrequency(&freq)==FALSE) {
			// Tickcount used, so our frequency would be 1000
			freq.QuadPart = 1000;
		}
		ret._time = _time + (t.ToInt() * freq.QuadPart) / 1000;
	#endif

	#ifdef TJ_OS_MAC
		ret._time = _time + (double(t._time) / 1000.0);
	#endif

	return ret;
}

bool Timestamp::IsEarlierThan(const Timestamp& o) const {
	return _time < o._time;
}

bool Timestamp::IsLaterThan(const Timestamp& o) const {
	return _time > o._time;
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
	#ifdef TJ_OS_MAC
		return ToMilliSeconds() * 1000.0;
	#endif
	
	#ifdef TJ_OS_WIN
		LARGE_INTEGER freq;
		if(QueryPerformanceFrequency(&freq)==FALSE) {
			// Tickcount used, so our frequency would be 1000
			freq.QuadPart = 1000;
		}

		long long ms = (1000*1000*_time) / freq.QuadPart;
		return ms;
	#endif
}

long double Timestamp::ToMilliSeconds() const {
	#ifdef TJ_OS_MAC
		return _time * 1000.0;
	#endif

	#ifdef TJ_OS_WIN
		LARGE_INTEGER freq;
		if(QueryPerformanceFrequency(&freq)==FALSE) {
			// Tickcount used, so our frequency would be 1000
			freq.QuadPart = 1000;
		}

		long long us = (1000*1000*_time) / freq.QuadPart;
		return (long double)us/(long double)1000.0;
	#endif
}

bool Timestamp::operator>(const Timestamp& o) const {
	return _time > o._time;
}

bool Timestamp::operator<(const Timestamp& o) const {
	return _time < o._time;
}