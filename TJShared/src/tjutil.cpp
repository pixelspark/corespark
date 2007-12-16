#include "../include/tjshared.h"

namespace tj {
	namespace shared {
		template<> bool StringTo(std::wstring s, const bool& def) {
			std::wstring ln = s;
			std::transform(ln.begin(), ln.end(), ln.begin(), tolower);
			return def?(ln!=std::wstring(Language::Get(L"no"))):(ln==std::wstring(Language::Get(L"yes")));
		}

		template<> std::wstring Stringify(const bool& x) {
			std::wostringstream o;
			o << x?Language::Get(L"yes"):Language::Get(L"no");
			return o.str();
		}

		template<> std::wstring Stringify(const int& x) {
			wchar_t buffer[33];
			_itow_s(x, buffer, (size_t)16, 10);
			return std::wstring(buffer);
		}

		Time::Time(const std::wstring& txt) {
			std::wistringstream is(txt);
			Time time;
			is >> time;
			_time = time._time;
		}

		std::wstring Time::Format() const {
			// Value 0 seems to cause some trouble (displays weird values) so we handle that separately then...
			if(_time==0) {
				return L"00:00:00/000";
			}

			int time = abs(_time);
			float ft = float(time);
			float seconds = ft/1000.0f;
			unsigned int ms = (time%1000);

			float mseconds = floor(fmod(seconds,60.0f));
			float minutes = floor(fmod(seconds/60.0f, 60.0f));
			float hours = floor(seconds/3600.0f);

			// TJShow's format is hour:minute:second/ms
			std::wostringstream os;
			os.fill('0');
			if(_time<0) {
				os << L'-';
			}
			os  << hours << L':' << std::setw(2) << minutes << L':' << std::setw(2) << mseconds  << L'/' << std::setw(3)  << float(ms);

			return os.str();
		} 

		std::wistream& operator>>(std::wistream& strm, Time& time) {
			// {hour,minute,seconds,ms}
			int data[4] = {0,0,0,0};

			// The idea is that we put every new value we find in data[3], while shifting the existing value
			// in data[3] to the left.
			for(int a=0;a<5;a++) {
				if(strm.eof()) break;
				int val = 0;
				strm >> val;

				for(int b=0;b<4;b++) {
					data[b] = data[b+1];
				}
				data[3] = val;

				wchar_t delim;
				if(strm.eof()) break;
				strm >> delim;
			}

			time = Time(data[0]*3600*1000 + data[1]*60*1000 + data[2]*1000 + data[3]);
			return strm;
		}

		std::istream& operator>>(std::istream& strm, Time& time) {
			int x;
			strm >> x;
			time = Time(x);
			return strm;
		}

		std::wostream& operator<<( std::wostream& strm, const Time& time ) {
			strm << time.ToInt();
			return strm;
		}

		std::ostream& operator<<( std::ostream& strm, const Time& time ) {
			strm << time.ToInt();
			return strm;
		}
	}
}