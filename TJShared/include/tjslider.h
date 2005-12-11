#ifndef _TJSLIDER_H
#define _TJSLIDER_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED SliderWnd: public ChildWnd {
	public:
		SliderWnd(HWND parent, const wchar_t* title);
		virtual ~SliderWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual wchar_t GetPreferredHotkey();
		void SetHotkey(wchar_t hotkey);
		void SetListener(Listener* listener);
		float GetValue() const;
		void SetValue(float f, bool notify=true);
		virtual void Update();

	protected:
		wchar_t _hotkey;
		Listener* _listener;
		float _value;
		Gdiplus::Bitmap* _bitmap;
		bool _hasFocus;
};

#pragma warning(pop)

#endif