#ifndef _TJCHILDWND_H
#define _TJCHILDWND_H

namespace tj {
	namespace shared {
		class EXPORTED ChildWnd: public Wnd {
			public:
				ChildWnd(const wchar_t* title, bool wantDoubleClick=true, bool useDoubleBuffering=true);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
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

		class EXPORTED CheckboxWnd: public ChildWnd {
			public:
				CheckboxWnd();
				virtual ~CheckboxWnd();
				virtual bool IsChecked() const;
				virtual void SetChecked(bool t);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual void OnSize(const Area& ns);
				virtual void OnFocus(bool f);
				virtual void OnKey(Key k, wchar_t ch, bool down);

			protected:
				bool _checked;
				Icon _checkedIcon;
		};
	}
}

#endif