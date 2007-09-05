#ifndef _TJTIMESTAMP_H
#define _TJTIMESTAMP_H

namespace tj {
	namespace shared {

		/** The Timestamp class is a very precise time stamp. It uses QueryPerformanceCounter on Windows. **/
		class EXPORTED Timestamp {
			public:
				Timestamp(const Timestamp& t);
				~Timestamp();
				Timestamp(bool now=false);
				void Now();
				std::wstring ToString();
				Timestamp Difference(const Timestamp& other) const;
				long long ToMicroSeconds() const;
				long double ToMilliSeconds() const;
				bool operator>(const Timestamp& o) const;
				bool operator<(const Timestamp& o) const;
				Timestamp& operator =(const Timestamp& o);

			protected:
				long long _time;
		};
	}
}

#endif