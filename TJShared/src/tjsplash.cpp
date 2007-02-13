#include "../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

SplashWnd::SplashWnd(std::wstring path, int w, int h): Wnd(L"TJShow", 0L) {
	SetStyleEx(WS_EX_TOPMOST);
	SetStyle(WS_POPUP);
	UnsetStyle(WS_CAPTION);
	_image = Bitmap::FromFile(path.c_str(), TRUE);
	_progress = GC::Hold(new ProgressWnd(_wnd));
	_progress->SetIndeterminate(true);
	//_progress->SetValue(0.618);
	_progress->Show(true);
	
	RECT rc;
	GetWindowRect(_wnd, &rc);
    HMONITOR mon = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
    GetMonitorInfo(mon, &mi);

	RECT nrc;
	nrc.left   = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - w) / 2;
    nrc.top   = mi.rcWork.top  + (mi.rcWork.bottom - mi.rcWork.top  - h) / 2;
	MoveWindow(_wnd, nrc.left, nrc.top, w, h, FALSE);
	Layout();
}

SplashWnd::~SplashWnd() {
	delete _image;
}

void SplashWnd::Paint(Gdiplus::Graphics& g) {
	RECT rc;
	GetClientRect(_wnd, &rc);
	g.DrawImage(_image, RectF(float(rc.left), float(rc.top), float(rc.right-rc.left), float(rc.bottom-rc.top)));
}

void SplashWnd::Layout() {
	tj::shared::Rectangle rc = GetClientRectangle();
	if(_progress) {
		_progress->Move(7, rc.GetHeight()-34, rc.GetWidth()-14, 18);
	}
}

LRESULT SplashWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_LBUTTONDOWN) {
		CloseWindow(_wnd);
	}
	else if(msg==WM_SIZE) {
		Layout();
	}

	return Wnd::Message(msg, wp, lp);
}

wchar_t SplashWnd::GetPreferredHotkey() {
	return L'S';
}

/* Splash thread */
SplashThread::SplashThread(std::wstring path, int w, int h) {
	_path = path;
	_w = w;
	_h = h;
}

SplashThread::~SplashThread() {
	Hide();
}

void SplashThread::Hide() {
	if(_wnd) {
		_wnd->Message(WM_CLOSE, 0, 0);
	}
	_closeEvent.Signal();
}

void SplashThread::Run() {
	SetName("SplashThread");
	MSG msg;

	// init GDI+
	ULONG_PTR ptr;
	GdiplusStartupInput input;
	GdiplusStartup(&ptr, &input, 0);

	_wnd = GC::Hold(new SplashWnd(_path, _w, _h));
	_wnd->Show(true);

	HANDLE handles[1] = { _closeEvent.GetHandle() };

	while(true) {
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		int result = MsgWaitForMultipleObjects(1, handles, FALSE, 1000, QS_ALLINPUT|QS_ALLPOSTMESSAGE); 

		// The result tells us the type of event we have.
		if(result==(WAIT_OBJECT_0 + 1)) {
			// New messages have arrived
			continue;
		} 
		else if(result==WAIT_ABANDONED) {
			continue;
		}
		else if(result==WAIT_OBJECT_0) {
			break; // show is over
		}
	}

	Log::Write(L"TJShared/SplashThread", L"Splash thread ends");

	GdiplusShutdown(ptr);
}