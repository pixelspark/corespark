#include "../include/tjtime.h"
#include "../include/tjutil.h"
#include "../include/tjlanguage.h"
#include <time.h>

#ifdef TJ_OS_MAC
	#include <CoreFoundation/CFDate.h>
	#include <CoreFoundation/CFTimeZone.h>
	#include <CoreFoundation/CFDateFormatter.h>
#endif

#ifdef TJ_OS_POSIX
	#include <sys/time.h>
	#include <stdint.h>
#endif

#ifdef TJ_OS_WIN
	#include <intsafe.h>
#endif

/* This code contains modified versions of parts of the OpenCFLite sources,
 * copyright (c) 2008-2009 Brent Fulgham <bfulgham@gmail.org>.  All rights reserved.
 * The OpenCFLite source code is a modified version of the CoreFoundation sources released by Apple Inc. under
 * the terms of the APSL version 2.0 (see below).
 * For information about changes from the original Apple source release can be found by reviewing the
 * source control system for the project at https://sourceforge.net/svn/?group_id=246198.
 * The original license information is as follows:
 * 
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

using namespace tj::shared;

/* Date */
const double Date::KIntervalSince1970 = 978307200.0L;
const double Date::KIntervalSince1904 = 3061152000.0L;
const DayOfMonth Date::KDaysInMonth[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const int Date::KDaysBeforeMonth[14] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
const int Date::KDaysAfterMonth[14] = {365, 334, 306, 275, 245, 214, 184, 153, 122, 92, 61, 31, 0, 0};

#ifdef TJ_OS_WIN
	AbsoluteDate AbsoluteDateFromFileTime(FILETIME* ft) {
		AbsoluteDate ret = (AbsoluteDate)ft->dwHighDateTime * 429.49672960;
		ret += (AbsoluteDateInterval)ft->dwLowDateTime / 10000000.0;
		ret -= (11644473600.0 + Date::KIntervalSince1970);
		/* seconds between 1601 and 1970, 1970 and 2001 */
		return ret;
	}
#endif

bool Date::IsLeapYear(Year year) {
	year -= 2001; /* year now is absolute year; Gregorian 2001 == year 0; 2001/1/1 = absolute date 0 */
    Year y = (year + 1) % 400;	/* correct to nearest multiple-of-400 year, then find the remainder */
    if (y < 0) y = -y;
    return (0 == (y & 3) && 100 != y && 200 != y && 300 != y);
}

DayOfWeek Date::GetDayOfWeek(AbsoluteDate at) {
    int64 absolute;
    absolute = (int64)floor(at / 86400.0);
    return ((absolute < 0) ? ((absolute + 1) % 7 + 7) : (absolute % 7 + 1))-1; /* Monday = 0, Tuesday=1 onwards */
}

DayOfMonth Date::GetDaysInMonth(Month month, bool leap) {
    return KDaysInMonth[month] + (2 == month && leap);
}

int Date::GetDaysBeforeMonth(Month month, bool leap) {
    return KDaysBeforeMonth[month] + (2 < month && leap);
}

int Date::GetDaysAfterMonth(Month month, bool leap) {
    return KDaysAfterMonth[month] + (month < 2 && leap);
}

int DoubleModToInt(double d, int modulus) {
    int result = (int)(float)floor(d - floor(d / modulus) * modulus);
    if (result < 0) {
		result += modulus;
	}
    return result;
}

double DoubleMod(double d, int modulus) {
    double result = d - floor(d / modulus) * modulus;
    if (result < 0.0) {
		result += (double)modulus;
	}
    return result;
}

AbsoluteDate Date::FromGMT(Year year, Month month, DayOfMonth day) {
    AbsoluteDate absolute = 0.0;
	year -= 2001;
    int64 b = year / 400; // take care of as many multiples of 400 years as possible
    absolute += b * 146097.0;
    year -= b * 400;
	
    if (year < 0) {
		for (int64 idx = year; idx < 0; idx++) {
			absolute -= GetDaysAfterMonth(0, IsLeapYear(2001+idx));
		}
    }
	else {
		for(int64 idx = 0; idx < year; idx++) {
			absolute += GetDaysAfterMonth(0, IsLeapYear(2001+idx));
		}
    }
    /* Now add the days into the original year */
    absolute += GetDaysBeforeMonth(month, IsLeapYear(2001+year)) + day - 1;
    return absolute;
}

void Date::FromGMT(Year y, Month m, DayOfMonth d, Hours h, Minutes min, Seconds s) {
    _date = 86400.0 * FromGMT(y, m, d);
    _date += 3600.0 * h + 60.0 * min + s;
	_year = y;
	_month = m;
	_day = d;
	_hour = h;
	_minute = min;
	_second = s;
}

/* year arg is absolute year; Gregorian 2001 == year 0; 2001/1/1 = absolute date 0 */
void Date::YMDFromAbsolute(int64 absolute, int64* year, int* month, int* day) {
    int64 b = absolute / 146097; // take care of as many multiples of 400 years as possible
    int64 y = b * 400;
    int ydays;
	
    absolute -= b * 146097;
    while(absolute < 0) {
		y -= 1;
		absolute += GetDaysAfterMonth(0, IsLeapYear(y+2001));
    }
	
    /* Now absolute is non-negative days to add to year */
    ydays = GetDaysAfterMonth(0, IsLeapYear(y+2001));
    while (ydays <= absolute) {
		y += 1;
		absolute -= ydays;
		ydays = GetDaysAfterMonth(0, IsLeapYear(y+2001));
    }
	
    /* Now we have year and days-into-year */
    if (year) {
		*year = y;
	}
	
    if(month || day) {
		int64 m = absolute / 33 + 1; /* search from the approximation */
		bool leap = IsLeapYear(y+2001);
		while(GetDaysBeforeMonth(Month(m + 1), leap) <= absolute) {
			m++;
		}
		
		if(month) {
			*month = int(m);
		}
		
		if(day) {
			*day = int(absolute - GetDaysBeforeMonth(Month(m), leap) + 1);
		}
    }
}

void Date::FromAbsoluteDate(AbsoluteDate at) {
    int64 year;
    int month, day;
    
    int64 absolute = (int64)floor(at / 86400.0);
    YMDFromAbsolute(absolute, &year, &month, &day);
    /**if (INT32_MAX - 2001 < year) {
		year = INT32_MAX - 2001;
	}**/
	
    _year = year + 2001;
    _month = month;
    _day = day;
    _hour = DoubleModToInt(floor(at / 3600.0), 24);
    _minute = DoubleModToInt(floor(at / 60.0), 60);
    _second = DoubleMod(at, 60);
    if (0.0 == _second) _second = 0.0;	// stomp out possible -0.0
}

AbsoluteDate Date::GetAbsoluteDate() {
	#ifdef TJ_OS_POSIX
		struct timeval tv;
		gettimeofday(&tv, NULL);
		AbsoluteDate ret = (AbsoluteDateInterval)tv.tv_sec - KIntervalSince1970;
		ret += (1.0E-6 * (AbsoluteDateInterval)tv.tv_usec);
		return ret;
	#endif
	
	#ifdef TJ_OS_WIN
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		return AbsoluteDateFromFileTime(&ft);
	#endif
}

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
	_date = GetAbsoluteDate();
	FromAbsoluteDate(_date);
}

Date::Date(AbsoluteDate ad) {
	_date = ad;
	FromAbsoluteDate(_date);
}

bool Date::operator < (const Date& o) const {
	return _date < o._date;
}

bool Date::operator > (const Date& o) const {
	return _date > o._date;
}

bool Date::operator== (const Date& o) const {
	return _date == o._date;
}

bool Date::operator!= (const Date& o) const {
	return _date != o._date;
}

AbsoluteDate Date::ToAbsoluteDate() const {
	return _date;
}

Date::Date(Year y, Month m, DayOfMonth d, Hours h, Minutes min, Seconds s) {
	FromGMT(y,m,d,h,min,s);
}

Date::~Date() {
}

Month Date::GetMonth() const {
	return _month;
}

DayOfWeek Date::GetDayOfWeek() const {
	return GetDayOfWeek(_date);
}

DayOfMonth Date::GetDayOfMonth() const {
	return _day;
}

Year Date::GetYear() const {
	return _year;
}

Seconds Date::GetSeconds() const {
	return _second;
}

Minutes Date::GetMinutes() const {
	return _minute;
}

Hours Date::GetHours() const {
	return _hour;
}

String Date::ToFriendlyString() const {
	std::wostringstream wos;
	wos << int(_year) << L'-' << int(_month) << L'-' << int(_day) << L' ' << int(_hour) << L':' << int(_minute) << L':' << _second << L" GMT";
	return wos.str();
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
		_time = 0.0;
	}
}

void Timestamp::Now() {
	_time = Date::GetAbsoluteDate();
}

String Timestamp::ToString() const {
	return Stringify((int)ToMicroSeconds());
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
	ret._time = _time + (double(t._time) / 1000.0);
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
	return (long long)(_time * 1000.0 * 1000.0);
}

long double Timestamp::ToMilliSeconds() const {
	return _time * 1000.0;
}

bool Timestamp::operator>(const Timestamp& o) const {
	return _time > o._time;
}

bool Timestamp::operator<(const Timestamp& o) const {
	return _time < o._time;
}
