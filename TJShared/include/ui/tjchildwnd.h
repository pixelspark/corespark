#ifndef _TJCHILDWND_H
#define _TJCHILDWND_H

namespace tj {
	namespace shared {
		class EXPORTED ChildWnd: public Wnd {
			public:
				ChildWnd(const wchar_t* title, bool wantDoubleClick=true, bool useDoubleBuffering=true);
		};

		/* for testing the splitter thing */
		class EXPORTED ColorWnd: public ChildWnd {
		public:
			ColorWnd(unsigned char r, unsigned char g, unsigned char b);
			virtual ~ColorWnd();
			virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
			virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);

			unsigned char _r,_g,_b;
		};
	}
}

#endif