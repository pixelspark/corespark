#ifndef _TJTIME_H
#define _TJTIME_H

namespace tj {
	namespace shared {
		enum TimeFormat {
			TimeFormatMS=0,
			TimeFormatFriendly,
			TimeFormatSequential,
		};

		typedef unsigned char Month; /* 1..12 */
		typedef unsigned char DayOfWeek; /* 0..6 */
		typedef unsigned char DayOfMonth; /* 1..31 */
		typedef unsigned short Year;
		typedef unsigned char Seconds; /* 0..59 */
		typedef unsigned char Minutes; /* 0..59 */
		typedef unsigned char Hours; /* 0..23 */


		class EXPORTED Date {
			public:
				static String GetFriendlyMonthName(Month m);
				static String GetFriendlyDayName(DayOfWeek d);

				Date();
				~Date();
				Month GetMonth() const;
				DayOfWeek GetDayOfWeek() const;
				DayOfMonth GetDayOfMonth() const;
				Year GetYear() const;
				Seconds GetSeconds() const;
				Minutes GetMinutes() const;
				Hours GetHours() const;
				String ToFriendlyString() const;

			private:
				#ifdef TJ_OS_WIN
					SYSTEMTIME _time;
				#endif
			
				#ifdef TJ_OS_MAC
					double _time;
				#endif
		};

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
	

		/** The Timestamp class is a very precise time stamp. It uses QueryPerformanceCounter on Windows. **/
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
				long long _time;
		};
	}
}

#endif