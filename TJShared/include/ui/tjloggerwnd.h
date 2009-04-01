#ifndef _TJLOGGER_H
#define _TJLOGGER_H

namespace tj {
	namespace shared {
		class EXPORTED LoggerWnd: public Wnd {
			public:
				LoggerWnd();
				virtual ~LoggerWnd();
				void Log(const String& ws);
				virtual void Layout();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual String GetContents();

			protected:
				#ifdef TJ_OS_WIN
					virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
			
					HFONT _font;
					HWND _list;
				#endif
		};
	}
}

#endif