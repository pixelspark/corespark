#include "../../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

SplashWnd::SplashWnd(std::wstring path, int w, int h): Wnd(L"TJShow", 0L, TJ_DROPSHADOW_CLASS_NAME) {
	SetStyleEx(WS_EX_TOPMOST);
	SetStyle(WS_POPUP);
	UnsetStyle(WS_CAPTION);

	_image = Bitmap::FromFile(path.c_str(), TRUE);
	_progress = GC::Hold(new ProgressWnd());
	_progress->SetIndeterminate(true);
	Add(_progress);
	
	RECT rc;
	GetWindowRect(GetWindow(), &rc);
    HMONITOR mon = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
    GetMonitorInfo(mon, &mi);

	RECT nrc;
	nrc.left   = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - w) / 2;
    nrc.top   = mi.rcWork.top  + (mi.rcWork.bottom - mi.rcWork.top  - h) / 2;
	MoveWindow(GetWindow(), nrc.left, nrc.top, w, h, FALSE);
	SetWindowPos(GetWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	Layout();

	// Make window transparent
	SetWindowLong(GetWindow(), GWL_EXSTYLE, GetWindowLong(GetWindow(), GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(GetWindow(), 0, (255 * 90) / 100, LWA_ALPHA);
}

SplashWnd::~SplashWnd() {
	delete _image;
}

void SplashWnd::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();
	g.DrawImage(_image, (RectF)rc);
}

void SplashWnd::Layout() {
	Area rc = GetClientArea();
	if(_progress) {
		_progress->Move(7, rc.GetHeight()-34, rc.GetWidth()-14, 18);
	}
}

LRESULT SplashWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_LBUTTONDOWN) {
		CloseWindow(GetWindow());
	}
	else if(msg==WM_SIZE) {
		Layout();
	}

	return Wnd::Message(msg, wp, lp);
}

/* Splash thread */
SplashThread::SplashThread(std::wstring path, int w, int h) {
	_path = path;
	_w = w;
	_h = h;
}

SplashThread::~SplashThread() {
	Hide();
	WaitForCompletion();
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

	GdiplusShutdown(ptr);
}