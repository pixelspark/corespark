#ifndef _TJIMAGEBUTTON_H
#define _TJIMAGEBUTTON_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED ButtonWnd: public ChildWnd {
	public:
		ButtonWnd(HWND parent, wchar_t hotkey, ref<Listener> listener, const wchar_t* image, const wchar_t* text=0);
		virtual ~ButtonWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual wchar_t GetPreferredHotkey();
		virtual void EnterHotkeyMode();
		virtual void Paint(Gdiplus::Graphics& g);
		virtual void SetListener(ref<Listener> lf);
		
	protected:
		wchar_t _hotkey;
		std::wstring _text;
		Gdiplus::Bitmap* _image;
		ref<Listener> _listener;
		bool _down;
};

class EXPORTED StateButtonWnd: public ButtonWnd {
	public:
		enum ButtonState {On, Off, Other};
		StateButtonWnd(HWND parent, wchar_t hotkey, ref<Listener> listener, const wchar_t* imageOn, const wchar_t* imageOff, const wchar_t* imageOther);
		virtual ~StateButtonWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		void SetOn(ButtonState o);
		virtual void Paint(Gdiplus::Graphics& g);
		
	protected:
		Gdiplus::Bitmap* _offImage;
		Gdiplus::Bitmap* _otherImage;
		ButtonState _on;
};

#pragma warning(pop)
#endif