#include "../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

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

void ChildWnd::Fill(LayoutFlags flags, tj::shared::Rectangle& rect) {
	switch(flags) {
		case LayoutFill:
		case LayoutTop:
		case LayoutBottom:
		case LayoutRight:
		case LayoutLeft:
			Move(rect.GetLeft(), rect.GetTop(), rect.GetWidth(), rect.GetHeight());
			rect.SetWidth(0);
			rect.SetHeight(0);
			break;
		case LayoutHide:
		default:
			Move(0,0,0,0);
	}	
}

void ChildWnd::Fill() {
	RECT rc;
	HWND parent = ::GetParent(_wnd);
	GetClientRect(parent, &rc);
	Move(rc.left, rc.top, rc.right, rc.bottom);
}