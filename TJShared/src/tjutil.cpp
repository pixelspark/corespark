#include "../include/tjcore.h"
#include <math.h>
#include <iomanip>
#include <dshow.h>
using namespace tj::shared;

std::set<Copyright*> Copyright::_copyrights;

Copyright::~Copyright() {
	std::set<Copyright*>::iterator it = _copyrights.find(this);
	if(it!=_copyrights.end()) {
		_copyrights.erase(it);
	}
}

String Copyright::Dump() {
	std::wostringstream wos;
	std::set<Copyright*>::const_iterator it = _copyrights.begin();
	while(it!=_copyrights.end()) {
		Copyright* cr = *it;
		wos << File::GetFileName(cr->_module) << L" uses " << cr->_component << L' ' << cr->_description << std::endl;
		++it;
	}

	return wos.str();
}

void Copyright::AddCopyright(Copyright* cs) {
	_copyrights.insert(cs);
}

Time MediaUtil::GetDuration(const String& file) {
	ZoneEntry ze(Zones::LocalFileInfoZone);

	#ifdef WIN32
		if(GetFileAttributes(file.c_str())==INVALID_FILE_ATTRIBUTES) {
			Throw(L"File does not exist, cannot get length!", ExceptionTypeError);
		}

		// Graph files have infinite length
		if(file.length()>4 && file.substr(file.length()-4, 4)==L".grf") {
			return INT_MAX;
		}

		IGraphBuilder* graph = 0;
		if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&graph))) {
			Throw(L"Failed to create filter graph, cannot get length!", ExceptionTypeError);
		}

		if(FAILED(graph->RenderFile(file.c_str(), NULL))) {
			graph->Release();
			Throw(L"Failed to render file, cannot get length!", ExceptionTypeError);
		}

		IMediaSeeking* ms = 0;
		if(FAILED(graph->QueryInterface(IID_IMediaSeeking, (void**)&ms))) {
			graph->Release();
			Throw(L"Could not get media seeking interface, cannot get length!", ExceptionTypeError);
		}

		LONGLONG dur = 0;
		ms->GetDuration(&dur);
		graph->Release();
		ms->Release(); // needed?
		return Time(int(dur/10000));

	#else
		#error Not implemented
	#endif
}

const wchar_t* Bool::KTrue = L"yes";
const wchar_t* Bool::KFalse = L"no";

float Util::RandomFloat() { 
	#ifdef _CRT_RAND_S
		unsigned int pr = 0;
		if(rand_s(&pr)!=0) {
			Throw(L"Rand_s failed", ExceptionTypeError);
		}
		return float(pr)/float(UINT_MAX);
	#else
		return rand()/(float)RAND_MAX;
	#endif
}

/* rand() returns a number between 0 and RAND_MAX, which is guaranteed to be at least
32767, which is 7FFF (so we have 15 random bits). So just call this function twice and we
get a random number between 0 and 0x3FFFFFF (30 bits). This means that the int this function
returns is always positive, since the left-most bit is always 0.

The rand()&0x7FFF make sure that this also works correctly on systems with a RAND_MAX higher than
0x7FFF (we just cut the remaining bits off)
*/
int Util::RandomInt() {
	#ifdef _CRT_RAND_S
		unsigned int pr = 0;
		if(rand_s(&pr)!=0) {
			Throw(L"Rand_s failed", ExceptionTypeError);
		}
		return (int)(pr >> 1);
	#else
		return (rand() & 0x7FFF) | ((rand() & 0x7FFF) << 15);
	#endif
}

String Util::RandomIdentifier(wchar_t prefix) {
	std::wostringstream wos;
	wos << prefix << std::hex << std::uppercase << RandomInt() << std::hex << std::uppercase << Timestamp().ToMilliSeconds();
	return wos.str();
}

char* Util::CopyString(const char* str) {
	char* buf = new char[strlen(str)+1];
	strcpy_s(buf,strlen(str)+1,str);
	return buf;
}

wchar_t* Util::IntToWide(int x) {
	wchar_t* str = new wchar_t[33];
	_itow_s(x,str,33,10);
	return str;
}

String Util::GetModuleName() {
	#ifdef _WIN32
		HMODULE mod = (HMODULE)GetModuleHandle(NULL);
		wchar_t mfn[MAX_PATH+1];
		GetModuleFileName(mod, mfn, MAX_PATH);
		return String(mfn);
	#else
		#error Not implemented
	#endif
}

String Util::IPToString(const in_addr& ip) {
	return Wcs(std::string(inet_ntoa(ip)));
}

String Util::GetSizeString(Bytes bytes) {
	// A negative size means something is wrong or the size is unknown
	if(bytes<0) {
		return L"";
	}

	String x;
	const static Bytes BytesInATeraByte = Bytes(1024*1024*1024)*1024;
	const static Bytes BytesInAGigaByte = 1024*1024*1024;
	const static Bytes BytesInAMegaByte = 1024*1024;
	const static Bytes BytesInAKiloByte = 1024;

	if(bytes>BytesInATeraByte) {
		x = Stringify(int(bytes/BytesInATeraByte)) + String(L" TB");
	}
	else if(bytes>BytesInAGigaByte) {
		x = Stringify(int(bytes/BytesInAGigaByte)) + String(L" GB");
	}

	else if(bytes>BytesInAMegaByte) { 
		x = Stringify(int(bytes/BytesInAMegaByte)) + String(L" MB");
	}
	else if(bytes>BytesInAKiloByte) {
		x = Stringify(int(bytes/BytesInAKiloByte)) + String(L" kB");
	}
	else {
		x = Stringify(bytes)+ String(L" B");
	}

	return x;
}

String& Util::StringToLower(String& r) {
	transform(r.begin(), r.end(), r.begin(), tolower);
	return r;
}

namespace tj {
	namespace shared {
		template<> bool StringTo(const String& s, const bool& def) {
			String ln = s;
			std::transform(ln.begin(), ln.end(), ln.begin(), tolower);
			return def ? (ln!=Bool::KFalse) : (ln==Bool::KTrue);
		}

		template<> int StringTo(const String& s, const int& def) {
			return _wtoi(s.c_str());
		}

		template<> String StringTo(const String& s, const String& def) {
			return s;
		}

		template<> String Stringify(const bool& x) {
			std::wostringstream o;
			o << x ? Bool::KTrue : Bool::KFalse;
			return o.str();
		}

		template<> String Stringify(const int& x) {
			wchar_t buffer[33];
			_itow_s(x, buffer, (size_t)16, 10);
			return String(buffer);
		}

		Time::Time(const String& txt) {
			std::wistringstream is(txt);
			Time time;
			is >> time;
			_time = time._time;
		}

		String Time::Format() const {
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

// Clipboard (Windows implementation)
#ifdef _WIN32
	void Clipboard::SetClipboardText(const String& text) {
		ZoneEntry ze(Zones::ClipboardZone);
		if(OpenClipboard(NULL)) {
			EmptyClipboard();

			HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*(text.length()+1));
			if(mem) {
				wchar_t* textBuf = (wchar_t*)GlobalLock(mem);
				if(textBuf!=0) {
					memcpy(textBuf, text.c_str(), sizeof(wchar_t)*(text.length()+1));
					GlobalUnlock(mem);
				}

				SetClipboardData(CF_UNICODETEXT, mem);
				CloseClipboard();

				// We must not free the object until CloseClipboard is called.
				if(mem!=0) {
					GlobalFree(mem);
				}
			}
		}
	}

	bool Clipboard::GetClipboardText(String& text) {
		ZoneEntry ze(Zones::ClipboardZone);

		if(!IsTextAvailable()) {
			return false;
		}

		if(OpenClipboard(NULL)) {
			HANDLE handle = GetClipboardData(CF_UNICODETEXT);
			if(handle!=0) {
				wchar_t* textPointer = (wchar_t*)GlobalLock(handle);
				if(textPointer!=0) {
					text = String(textPointer);
					GlobalUnlock(handle);
				}
				else {
					CloseClipboard();
					return false;
				}
			}
			CloseClipboard();
			return true;
		}
		return false;
	}

	unsigned int Clipboard::_formatID = 0;
	bool Clipboard::_formatInitialized = false;
	const wchar_t* Clipboard::_formatName = L"application/x-tjshared-object";

	void Clipboard::Initialize() {
		if(!_formatInitialized) {
			_formatID = RegisterClipboardFormat(_formatName);
			_formatInitialized = true;
		}
	}

	void Clipboard::SetClipboardObject(ref<Serializable> sr) {
		ZoneEntry ze(Zones::ClipboardZone);
		Initialize();

		// Write the object to XML
		TiXmlDocument doc;
		TiXmlComment comment;
		comment.SetValue("TJ XML clipboard object");
		doc.InsertEndChild(comment);
		TiXmlElement root("object");
		sr->Save(&root);
		doc.InsertEndChild(root);

		// Get the XML source code
		std::ostringstream xos;
		xos << doc;
		std::string source = xos.str();

		// Push the source code to the clipboard!
		if(OpenClipboard(NULL)) {
			if(!EmptyClipboard()) {
				Log::Write(L"TJShared/Clipboard", L"Could not empty clipboard!");
			}

			HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, sizeof(char)*(source.length()+1));
			if(mem) {
				wchar_t* textBuf = (wchar_t*)GlobalLock(mem);
				if(textBuf!=0) {
					memcpy(textBuf, source.c_str(), sizeof(char)*(source.length()+1));
					GlobalUnlock(mem);
				}

				SetClipboardData(_formatID, mem);
				SetClipboardData(CF_TEXT, mem);

				CloseClipboard();

				// We must not free the object until CloseClipboard is called.
				if(mem!=0) {
					GlobalFree(mem);
				}
			}
			else {
				Log::Write(L"TJShared/Clipboard", L"GlobalAlloc failed!");
			}
		}
		else {
			Log::Write(L"TJShared/Clipboard", L"Could not open clipboard!");
		}
	}

	std::ostream& operator<< (std::ostream& out, const TiXmlNode& doc) {
		doc.StreamOut(&out);
		return out;
	}

	bool Clipboard::GetClipboardObject(TiXmlDocument& doc) {
		ZoneEntry ze(Zones::ClipboardZone);
		Initialize();

		if(IsObjectAvailable() && OpenClipboard(NULL)) {
			HGLOBAL handle = GetClipboardData(_formatID);
			if(handle!=0) {
				char* textPointer = (char*)GlobalLock(handle);
				if(textPointer!=0) {
					std::string text(textPointer);
					GlobalUnlock(handle);

					// Load text as XML
					doc.Parse(text.c_str());
					if(!doc.Error()) {
						CloseClipboard();
						return true;
					}
				}
			}
			
			CloseClipboard();
		}

		return false;
	}

	bool Clipboard::GetClipboardObject(ref<Serializable> s) {
		TiXmlDocument doc;
		if(GetClipboardObject(doc)) {
			TiXmlElement* object = doc.FirstChildElement("object");
			if(object!=0) {
				s->Load(object);
				CloseClipboard();
				return true;
			}
		}
		return false;
	}

	bool Clipboard::IsTextAvailable() {
		ZoneEntry ze(Zones::ClipboardZone);
		return IsClipboardFormatAvailable(CF_UNICODETEXT)==TRUE;
	}

	bool Clipboard::IsObjectAvailable() {
		ZoneEntry ze(Zones::ClipboardZone);
		Initialize();
		return IsClipboardFormatAvailable(_formatID)==TRUE;
	}
#endif

// lists of settings to change
#ifdef _WIN32
	static UINT dss_getlist[] = {SPI_GETLOWPOWERTIMEOUT, SPI_GETPOWEROFFTIMEOUT, SPI_GETSCREENSAVETIMEOUT};
	static UINT dss_setlist[] = {SPI_SETLOWPOWERTIMEOUT, SPI_SETPOWEROFFTIMEOUT, SPI_SETSCREENSAVETIMEOUT};
	static const int dss_listcount = 3;

	ScreensaverOff::ScreensaverOff() {
		_values = new int[dss_listcount];

		for (int x=0;x<dss_listcount;x++) {
		   SystemParametersInfo (dss_getlist[x], 0, &_values[x], 0);

			// Turn off the parameter
		   SystemParametersInfo (dss_setlist[x], 0, NULL, 0);
		}
	}

	ScreensaverOff::~ScreensaverOff() {
		for (int x=0;x<dss_listcount;x++) {
			// Set the old value
			SystemParametersInfo (dss_setlist[x],  _values[x], NULL, 0);
		}

		delete[] _values;
	}
#else
	#error ScreensaverOff not implemented yet
#endif