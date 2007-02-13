#ifndef _TJSPLASH_H
#define _TJSPLASH_H

class EXPORTED SplashWnd: public Wnd {
	public:
		SplashWnd(std::wstring path, int w, int h);
		virtual ~SplashWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void Paint(Gdiplus::Graphics& g);
		virtual wchar_t GetPreferredHotkey();
		virtual void Layout();

	protected:
		Gdiplus::Image* _image;
		ref<ProgressWnd> _progress;
};

class EXPORTED SplashThread: public Thread {
	public:
		SplashThread(std::wstring path, int w, int h);
		virtual ~SplashThread();
		virtual void Run();
		void Hide();
	protected:
		int _w, _h;
		std::wstring _path;
		ref<SplashWnd> _wnd;
		Event _closeEvent;
};

#endif