#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

PopupWnd::PopupWnd(): Wnd(L"", 0L, TJ_DROPSHADOW_CLASS_NAME, true, 0) {
	SetStyle(WS_BORDER);
	UnsetStyle(WS_CAPTION);
	SetSize(200,100); // default size for popups?
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
		Show(false);
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
	SetWindowPos(GetWindow(), 0, pt.x, pt.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	Show(true);
}

void PopupWnd::PopupAtMouse() {
	POINT pt;
	GetCursorPos(&pt);
	SetWindowPos(GetWindow(), 0, pt.x, pt.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	Show(true);
}