#ifndef _TJTIME_H
#define _TJTIME_H

namespace tj {
	namespace shared {
		enum TimeFormat {
			TimeFormatMS=0,
			TimeFormatFriendly,
			TimeFormatSequential,
		};

		struct EXPORTED Time {
			public:
				inline Time(int time=0) {
					_time = time;
				}

				Time(const std::wstring& t);

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

				std::wstring Format() const;

			protected:
				int _time;
		};


		EXPORTED std::wostream& operator<<(std::wostream& strm, const Time& time);
		EXPORTED std::ostream& operator<<(std::ostream& strm, const Time& time);
		EXPORTED std::wistream& operator>>(std::wistream& strm, Time& time);
		EXPORTED std::istream& operator>>(std::istream& strm, Time& time);
	}
}

#endif