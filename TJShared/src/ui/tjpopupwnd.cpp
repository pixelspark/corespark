#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

PopupWnd::PopupWnd(HWND parent, bool isDialog): Wnd(L"", parent, TJ_DROPSHADOW_CLASS_NAME, true, WS_EX_TOPMOST|WS_EX_TOOLWINDOW) {
	UnsetStyle(WS_CAPTION);

	if(isDialog) {
		SetStyle(WS_POPUP);
	}
}

PopupWnd::~PopupWnd() {
}

void PopupWnd::SetSize(Pixels w, Pixels h) {
	_w = w;
	_h = h;
	Wnd::SetSize(w,h);
}

void PopupWnd::OnActivate(bool activate) {
	if(activate) {
		Wnd::SetSize(_w,_h);
	}
	else {
		// Context menus disable the owner window before opening a popup, so if a context menu popup
		// is created from a PopupWnd, this will prevent this popup from disappearing
		DWORD style = GetWindowLong(GetWindow(), GWL_STYLE) & WS_DISABLED;
		if(style==0) {
			Show(false);
		}
	}
}

void PopupWnd::SetOpacity(float f) {
	int value = int(255.0f * f);
	SetWindowLong(GetWindow(), GWL_EXSTYLE, GetWindowLong(GetWindow(), GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(GetWindow(), 0, value, LWA_ALPHA);
}

void PopupWnd::PopupAt(Pixels x, Pixels y, ref<Wnd> window) {
	if(!window) {
		Throw(L"No window specified, cannot pop menu up", ExceptionTypeError);
	}

	ref<Theme> theme = ThemeManager::GetTheme();
	POINT pt;
	pt.x = long(x*theme->GetDPIScaleFactor());
	pt.y = long(y*theme->GetDPIScaleFactor());
	ClientToScreen(window->GetWindow(), &pt);
	FitToMonitor(pt);

	SetWindowPos(GetWindow(), 0, pt.x, pt.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	Show(true);
	if(SetFocus(GetWindow())==NULL) {
		DWORD error = GetLastError();
		Log::Write(L"TJShared/PopupWnd", L"SetFocus failed: "+Stringify(error)+L" hex: "+StringifyHex(error));
	}
}

void PopupWnd::PopupAtMouse() {
	POINT pt;
	GetCursorPos(&pt);
	FitToMonitor(pt);
	SetWindowPos(GetWindow(), 0, pt.x, pt.y, 0, 0, SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
}

LRESULT PopupWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	return Wnd::Message(msg,wp,lp);
}

void PopupWnd::FitToMonitor(POINT& pt) {
	MONITORINFO info;
	memset(&info,0,sizeof(info));
	info.cbSize = sizeof(MONITORINFO);
	HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
	if(GetMonitorInfo(mon, &info)==TRUE) {
		RECT wr;
		GetWindowRect(GetWindow(), &wr);

		// Adjust point so it falls inside the monitor completely
		if((pt.x+(wr.right-wr.left))>(info.rcWork.right)) {
			pt.x = (info.rcWork.right - (wr.right-wr.left));
		}
		else if(pt.x < info.rcWork.left) {
			pt.x = info.rcWork.left;
		}
		
		if(pt.y < info.rcWork.top) {
			pt.y = info.rcWork.top;
		}
		else if((pt.y+(wr.bottom-wr.top)) > (info.rcWork.bottom)) {
			pt.y = info.rcWork.bottom - (wr.bottom-wr.top);
		}
	}
}
