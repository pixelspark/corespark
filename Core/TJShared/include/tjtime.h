/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJTIME_H
#define _TJTIME_H

#include "tjsharedinternal.h"

namespace tj {
	namespace shared {
		enum TimeFormat {
			TimeFormatMS=0,
			TimeFormatFriendly,
			TimeFormatSequential,
		};

		typedef char Month; /* 1..12 */
		typedef char DayOfWeek; /* 0..6 */
		typedef char DayOfMonth; /* 1..31 */
		typedef int64 Year;
		typedef double Seconds; /* [0.0,60.0> */
		typedef char Minutes; /* 0..59 */
		typedef char Hours; /* 0..23 */

		typedef long double AbsoluteDate;
		typedef long double AbsoluteDateInterval;

		class EXPORTED Date {
			public:
				static String GetFriendlyMonthName(Month m);
				static String GetFriendlyDayName(DayOfWeek d);

				Date();
				Date(Year y, Month m, DayOfMonth d, Hours h, Minutes min, Seconds s);
				Date(AbsoluteDate ad);
				~Date();
				Month GetMonth() const;
				DayOfWeek GetDayOfWeek() const;
				DayOfMonth GetDayOfMonth() const;
				Year GetYear() const;
				Seconds GetSeconds() const;
				Minutes GetMinutes() const;
				Hours GetHours() const;
				String ToFriendlyString() const;
				AbsoluteDate ToAbsoluteDate() const;
			
				static AbsoluteDate GetAbsoluteDate();
				static AbsoluteDate FromGMT(Year year, Month month, DayOfMonth day);
				static DayOfMonth GetDaysInMonth(Month m, bool leap);
				static int GetDaysBeforeMonth(Month m, bool leap);
				static int GetDaysAfterMonth(Month m, bool leap);
				static bool IsLeapYear(Year y);
			
				bool operator < (const Date& o) const;
				bool operator > (const Date& o) const;
				bool operator== (const Date& o) const;
				bool operator!= (const Date& o) const;

				const static AbsoluteDateInterval KIntervalSince1970;
				const static AbsoluteDateInterval KIntervalSince1904;

			private:
				const static DayOfMonth KDaysInMonth[13];
				const static int KDaysBeforeMonth[14];
				const static int KDaysAfterMonth[14];
			
				void FromAbsoluteDate(AbsoluteDate ad);
				void FromGMT(Year y, Month m, DayOfMonth d, Hours h, Minutes min, Seconds s);
				static DayOfWeek GetDayOfWeek(AbsoluteDate d);
				static void YMDFromAbsolute(int64 absolute, int64* year, int* month, int* day);
			
				AbsoluteDate _date;
				Year _year;
				Month _month;
				DayOfMonth _day;
				Hours _hour;
				Minutes _minute;
				Seconds _second;
		};

		/** Legacy class that records a time interval in milliseconds (used by TJShow) **/
		struct EXPORTED Time {
			friend class Timestamp;
			
			public:
				inline Time(int time=0) {
					_time = time;
				}

				Time(const String& t);

				inline Time& operator=(const Time& that) {
					_time = that._time;
					return *this;
				}

				inline const int ToInt() const {
					return _time;
				}

				inline operator const int() const {
					return _time;
				}

				inline operator const float() const {
					return float(_time);
				}

				inline operator const unsigned int() const {
					return (unsigned int)_time;
				}

				// Increment/decrement operators
				inline Time& operator++() {
					_time++;
					return *this;
				}

				inline Time operator++(int) {
					Time temp = *this;
				   ++*this;
				   return temp;
				}
			 
				inline Time& operator--() {
					_time--;
					return *this;
				}

				inline Time operator--(int) {
					Time temp = *this;
				   --*this;
				   return temp;
				}

				inline Time operator+(const Time& t) const {
					return Time(_time+t._time);
				}

				inline Time operator-(const Time& t) const {
					return Time(_time-t._time);
				}

				inline Time operator*(const Time& t) const {
					return Time(_time*t._time);
				}

				inline Time operator*(float s) const {
					return Time(int(float(_time)*s));
				}

				inline Time operator/(const Time& t) const {
					return Time(_time/t._time);
				}

				inline Time& operator+=(int t) {
					_time += t;
					return *this;
				}

				inline Time& operator-=(int t) {
					_time -= t;
					return *this;
				}

				inline Time& operator+=(const Time& t) {
					_time += t._time;
					return *this;
				}

				inline Time& operator-=(const Time& t) {
					_time -= t._time;
					return *this;
				}

				inline Time& operator *=(int t) {
					_time *= t;
					return *this;
				}

				inline Time& operator *=(float s) {
					_time = int(float(_time) * s);
					return *this;
				}

				inline Time& operator /=(int t) {
					_time /= t;
					return *this;
				}

				// Time comparison
				inline bool operator<(const Time& t) const {
					return _time<t._time;
				}

				inline bool operator>(const Time& t) const {
					return _time>t._time;
				}

				inline bool operator<=(const Time& t) const {
					return _time<=t._time;
				}

				inline bool operator>=(const Time& t) const {
					return _time>=t._time;
				}

				inline bool operator==(const Time& t) const {
					return _time==t._time;
				}

				inline bool operator !=(const Time& t) const {
					return _time!=t._time;
				}

				String Format() const;

				inline static const Time& Earliest(const Time& a, const Time& b) {
					if(a==Time(-1)) {
						return b;
					}
					else if(b==Time(-1)) {
						return a;
					}
					else if(a<b) {
						return a;
					}
					else {
						return b;
					}
				}

			protected:
				int _time;
		};

		EXPORTED std::wostream& operator<<(std::wostream& strm, const Time& time);
		EXPORTED std::ostream& operator<<(std::ostream& strm, const Time& time);
		EXPORTED std::wistream& operator>>(std::wistream& strm, Time& time);
		EXPORTED std::istream& operator>>(std::istream& strm, Time& time);
	

		/** The Timestamp class is a very precise time stamp **/
		class EXPORTED Timestamp {
			public:
				Timestamp(const Timestamp& t);
				~Timestamp();
				Timestamp(bool now = false);
				void Now();
				String ToString() const;
				String ToHexString() const;
				Timestamp Difference(const Timestamp& other) const;
				Timestamp Increment(const Time& t) const;
				long long ToMicroSeconds() const;
				long double ToMilliSeconds() const;
				bool operator>(const Timestamp& o) const;
				bool operator<(const Timestamp& o) const;
				Timestamp& operator =(const Timestamp& o);
				bool IsEarlierThan(const Timestamp& o) const;
				bool IsLaterThan(const Timestamp& o) const;

			protected:
				AbsoluteDateInterval _time;
		};
	}
}

#endif