#ifndef _TJCHILDWND_H
#define _TJCHILDWND_H

namespace tj {
	namespace shared {
		class EXPORTED ChildWnd: public Wnd {
			public:
				ChildWnd(const wchar_t* title, bool useDoubleBuffering=true);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		};

		/* for testing the splitter thing */
		class EXPORTED ColorWnd: public ChildWnd {
		public:
				ColorWnd(unsigned char r, unsigned char g, unsigned char b);
				virtual ~ColorWnd();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);

				unsigned char _r,_g,_b;
		};

		class EXPORTED CheckboxWnd: public ChildWnd {
			public:
				CheckboxWnd();
				virtual ~CheckboxWnd();
				virtual bool IsChecked() const;
				virtual void SetChecked(bool t);
				virtual void SetReadOnly(bool r);
				virtual bool IsReadOnly() const;
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);

			protected:
				virtual void OnTimer(unsigned int id);
				virtual void OnSize(const Area& ns);
				virtual void OnFocus(bool f);
				virtual void OnKey(Key k, wchar_t ch, bool down, bool isAccelerator);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);

				Icon _checkedIcon;
				Animation _checkAnimation;

			private:
				bool _checked;
				bool _readOnly;
		};
	}
}

#endif