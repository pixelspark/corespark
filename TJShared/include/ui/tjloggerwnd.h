#ifndef _TJLOGGER_H
#define _TJLOGGER_H

namespace tj {
	namespace shared {
		class EXPORTED LoggerWnd: public Wnd {
			public:
				LoggerWnd();
				virtual ~LoggerWnd();
				void Log(const std::wstring& ws);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Layout();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual std::wstring GetContents();

			protected:
				HFONT _font;
				HWND _list;
		};
	}
}

#endif