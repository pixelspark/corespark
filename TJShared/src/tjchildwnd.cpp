#include "../include/tjshared.h"
using namespace Gdiplus;

ChildWnd::ChildWnd(const wchar_t* title, HWND parent, bool wantDoubleClick, bool useDB): Wnd(title,parent, wantDoubleClick?TJ_DEFAULT_CLASS_NAME:(L"TjWndClassNdbl"), useDB) {
	SetWindowLong(_wnd, GWL_STYLE, WS_CHILD);
	SetWindowLong(_wnd, GWL_EXSTYLE, 0);

	SetStyle(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	//Show(true);
}

void ChildWnd::LeaveHotkeyMode(wchar_t key) {
	Wnd::LeaveHotkeyMode(key);

	HWND parent = ::GetParent(_wnd);
	if(parent!=0) {
		RECT rc;
		GetClientRect(parent, &rc);
		InvalidateRect(parent, &rc, FALSE);
	}
}

void ChildWnd::Fill() {
	RECT r;
	GetClientRect(::GetParent(_wnd), &r);
	SetWindowPos(_wnd, 0, 0, 0, r.right-r.left, r.bottom-r.top, SWP_NOZORDER);
}