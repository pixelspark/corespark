#ifndef _TJTIME_H
#define _TJTIME_H

namespace tj {
	namespace shared {
		enum TimeFormat {
			TimeFormatMS=0,
			TimeFormatFriendly,
			TimeFormatSequential,
		};

		struct Time {
			public:
				inline Time(int time=0) {
					_time = time;
				}

				inline Time(std::wstring t) {
					std::wistringstream is(t);
					is >> _time;
				}

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

				std::wstring Format() const {
					float ft = float(_time);
					float seconds = ft/1000.0f;
					unsigned int ms = (_time%1000) / 10;

					float mseconds = floor(fmod(seconds,60.0f));
					float minutes = floor(seconds/60.0f);
					float hours = floor(seconds/3600.0f);

					std::wostringstream os;
					os.fill('0');
					os  << hours << L':' << std::setw(2) << minutes << L':' << std::setw(2) << mseconds  << L'.' << std::setw(2)  << float(ms);

					return os.str();
				}

			protected:
				int _time;
		};


		std::wostream& operator<<(std::wostream& strm, const Time& time);
		std::wistream& operator>>(std::wistream& strm, Time& time);
		std::ostream& operator<<(std::ostream& strm, const Time& time);
		std::istream& operator>>(std::istream& strm, Time& time);

		inline std::wostream& operator<<( std::wostream& strm, const Time& time ) {
			///if(ThemeManager::IsFriendlyTime()) {
			///	strm << time.Format();
			///}
			///else {
			strm << time.ToInt();
			///}
			return strm;
		}

		inline std::ostream& operator<<( std::ostream& strm, const Time& time ) {
			strm << time.ToInt();
			return strm;
		}

		inline std::wistream& operator>>(std::wistream& strm, Time& time) {
			int x;
			strm >> x;

			wchar_t y = L'\0';
			strm >> y;

			if(y==L':') {
				int z = 0;
				strm >> z;

				y = L'\0';
				strm >> y;
				if(y==L':') {
					int a = 0; // ms
					strm >> a;
					time = Time((x*60+z)*1000 + a);
				}
				else {
					// x = minutes,
					// z = seconds
					time = Time((x*60+z)*1000);
				}
			}
			else {
				time = Time(x);
			}
			return strm;
		}

		inline std::istream& operator>>(std::istream& strm, Time& time) {
			int x;
			strm >> x;
			time = Time(x);
			return strm;
		}
	}
}

#endif