#ifndef _TJPROGRESSWND_H
#define _TJPROGRESSWND_H

class EXPORTED ProgressWnd: public ChildWnd {
	public:
		ProgressWnd(HWND parent);
		virtual ~ProgressWnd();
		virtual void Paint(Gdiplus::Graphics& g);
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual wchar_t GetPreferredHotkey();
		void SetIndeterminate(bool t);
		void SetValue(float x);
		void Update();

	protected:
		bool _indeterminate;
		float _value;
};

#endif