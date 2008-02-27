#ifndef _TJIMAGEBUTTON_H
#define _TJIMAGEBUTTON_H

namespace tj {
	namespace shared {
		class EXPORTED ButtonWnd: public ChildWnd {
			public:
				ButtonWnd(const wchar_t* image, const wchar_t* text=0);
				virtual ~ButtonWnd();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual void SetListener(ref<Listener> lf);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void Fill(LayoutFlags lf, Area& rect, bool direct = true);
				virtual void SetText(const wchar_t* t);
				virtual void SetDisabled(bool d);
				virtual bool IsDisabled() const;
				
			protected:
				virtual void OnKey(Key k, wchar_t t, bool down);
				virtual void OnFocus(bool f);

				std::wstring _text;
				Gdiplus::Bitmap* _image;
				weak<Listener> _listener;
				bool _down;
				bool _disabled;
		};

		class EXPORTED StateButtonWnd: public ButtonWnd {
			public:
				enum ButtonState {On, Off, Other};
				StateButtonWnd(const wchar_t* imageOn, const wchar_t* imageOff, const wchar_t* imageOther);
				virtual ~StateButtonWnd();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				void SetOn(ButtonState o);
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				
			protected:
				Gdiplus::Bitmap* _offImage;
				Gdiplus::Bitmap* _otherImage;
				ButtonState _on;
		};
	}
}

#endif