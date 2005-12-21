#include "../include/tjshared.h"
using namespace Gdiplus;

ChildWnd::ChildWnd(const wchar_t* title, HWND parent, bool wantDoubleClick, bool useDB): Wnd(title,parent, wantDoubleClick?TJ_DEFAULT_CLASS_NAME:(L"TjWndClassNdbl"), useDB) {
	SetWindowLong(_wnd,GWL_STYLE,WS_CHILD);
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

LRESULT ChildWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_PAINT) {
		Wnd* parent = GetParent();
		if(parent!=0) {
			if(parent->IsInHotkeyMode()) {
				ref<Theme> theme = ThemeManager::GetTheme();

				HDC dc = GetWindowDC(_wnd);

				RECT rc;
				GetClientRect(_wnd, &rc);
				
				{ 
					Graphics g(dc);
					std::wostringstream os;
					os << GetPreferredHotkey();
					std::wstring hk = os.str();
					DrawHotkey(&g, hk.c_str(), (rc.right-rc.left)/2, (rc.bottom-rc.top)/2);
				}
				
				ReleaseDC(_wnd,dc);
			}
		}
	}

	return Wnd::Message(msg,wp,lp);
}

void ChildWnd::Fill() {
	RECT r;
	GetClientRect(::GetParent(_wnd), &r);
	SetWindowPos(_wnd, 0, 0, 0, r.right-r.left, r.bottom-r.top, SWP_NOZORDER);
	Layout();
}