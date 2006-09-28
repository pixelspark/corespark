#include "../include/tjshared.h"
#include <math.h>
#include <windowsx.h>
using namespace Gdiplus;
using namespace tj::shared;

const float SplitterWnd::snapMargin = 0.07f;

SplitterWnd::SplitterWnd(HWND parent, ref<Wnd> a, ref<Wnd> b, Orientation o): ChildWnd(L"Splitter", parent, true, false) {
	SetStyle(WS_CLIPCHILDREN);
	SetStyle(WS_CLIPSIBLINGS);
	if(a) {
		SetParent(a->GetWindow(),_wnd);
	}

	if(b) {
		SetParent(b->GetWindow(),_wnd);
	}

	_a = a;
	_b = b;
	Layout();
	_ratio = 0.618f; // Fibonacci :-)
	_defaultRatio = 0.618f;
	_dragging = false;
	_orientation = o;
	Show(true);
}

SplitterWnd::~SplitterWnd() {
}


void SplitterWnd::EnterHotkeyMode() {
	Wnd* parent = GetParent();
	if(parent!=0) parent->EnterHotkeyMode();
}

void SplitterWnd::LeaveHotkeyMode(wchar_t key) {
	/*Wnd* parent = GetParent();
	if(parent!=0) parent->LeaveHotkeyMode(key);*/
	if(_a && _a->GetPreferredHotkey()==key) {
		_a->EnterHotkeyMode();
	}
	else if(_b && _b->GetPreferredHotkey()==key) {
		_b->EnterHotkeyMode();
	}
}

bool SplitterWnd::IsSplitter() {
	return true;
}

void SplitterWnd::Layout() {
	RECT rc;
	GetClientRect(_wnd, &rc);

	if(_orientation==OrientationHorizontal) {
		int heightA = (int)floor(_ratio*(rc.bottom-rc.top));
		int heightB = (rc.bottom-rc.top)-heightA-1;

		if(_a) SetWindowPos(_a->GetWindow(), 0, 0, 0, rc.right-rc.left, heightA-(barHeight/2),SWP_NOZORDER);
		if(_b) SetWindowPos(_b->GetWindow(), 0, 0, heightA+(barHeight/2), rc.right-rc.left-1, heightB-(barHeight/2),SWP_NOZORDER);
	}
	else if(_orientation==OrientationVertical) {
		int widthA = (int)floor(_ratio*(rc.right-rc.left));
		int widthB = (rc.right-rc.left)-widthA-1;

		if(_a) SetWindowPos(_a->GetWindow(), 0, 0, 0, widthA-(barHeight/2), rc.bottom-rc.top, SWP_NOZORDER);
		if(_b) SetWindowPos(_b->GetWindow(), 0, widthA+(barHeight/2), 0, widthB-(barHeight/2),rc.bottom-rc.top-1,SWP_NOZORDER);
	}
}

void SplitterWnd::SetRatio(float f) {
	_ratio = f;
	_defaultRatio = f;
	Layout();
	Repaint();
}

void SplitterWnd::Expand() {
	_ratio = _defaultRatio;
	Layout();
	Repaint();
}

void SplitterWnd::Paint(Graphics& g) {
	ref<Theme> theme = ThemeManager::GetTheme();
	RECT rc;
	GetClientRect(_wnd, &rc);

	HWND root = GetAncestor(_wnd, GA_ROOT);
	Gdiplus::Brush* abr = theme->GetApplicationBackgroundBrush(root, _wnd);
	if(abr!=0) {
		if(_orientation==OrientationHorizontal) {
			int bH = int(_ratio * (rc.bottom-rc.top)-(barHeight/2)); // top of the bar
			g.FillRectangle(abr, 0,bH-2,rc.right-rc.left, barHeight+4);
		}
		else if(_orientation==OrientationVertical) {
			int bH = int(_ratio * (rc.right-rc.left)-(barHeight/2)); // top of the bar
			g.FillRectangle(abr, bH-2, 0, barHeight+4, rc.bottom-rc.top);
		}
		g.FillRectangle(abr, 0, rc.bottom-5, rc.right-rc.left, 5);
		g.FillRectangle(abr, rc.right-5, 0, 5, rc.bottom-rc.top);

		delete abr;
	}
}

LRESULT SplitterWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE) {
		Layout();
	}
	else if(msg==WM_MOUSEMOVE) {
		int x = GET_X_LPARAM(lp);
		int y = GET_Y_LPARAM(lp);

		if(_dragging) {
			RECT rc;
			GetClientRect(_wnd,&rc);
			if(_orientation==OrientationHorizontal) {
				_ratio = float(y)/float(rc.bottom-rc.top);
			}
			else if(_orientation==OrientationVertical) {
				_ratio = float(x)/float(rc.right-rc.left);
			}

			// Snap and protect from going out of view
			float full = _orientation==OrientationHorizontal?float(rc.bottom-rc.top):float(rc.right-rc.left);
			float relativeBarSize = (float(barHeight) / full)/2.0f;

			if(_ratio>(1.0f-snapMargin)) _ratio = 1.0f-relativeBarSize;
			if(_ratio<snapMargin) _ratio = relativeBarSize;
			Layout();
			Repaint();
		}
		else {
			RECT rc;
			GetClientRect(_wnd,&rc);
			if(_orientation==OrientationHorizontal) {
				int bH = int(_ratio * (rc.bottom-rc.top)-(barHeight/2));
				if(y<bH+barHeight&&y>bH) {
					SetCursor(LoadCursor(0,IDC_SIZENS));
				}
				else {
					SetCursor(LoadCursor(0,IDC_ARROW));
				}
			}
			else if(_orientation==OrientationVertical) {
				int bH = int(_ratio * (rc.right-rc.left)-(barHeight/2));
				if(x<bH+barHeight&&x>bH) {
					SetCursor(LoadCursor(0,IDC_SIZEWE));
				}
				else {
					SetCursor(LoadCursor(0,IDC_ARROW));
				}
			}

		}
	}
	else if(msg==WM_LBUTTONDOWN) {
		SetCursor(LoadCursor(0,_orientation==OrientationHorizontal?IDC_SIZENS:IDC_SIZEWE));
		SetCapture(_wnd);
		_dragging = true;
		Repaint();
	}
	else if(msg==WM_LBUTTONUP) {
		_dragging = false;
		ReleaseCapture();
		SetCursor(LoadCursor(0,IDC_ARROW));
		Repaint();
	}
	else if(msg==WM_LBUTTONDBLCLK) {
		Collapse();
	}

	return ChildWnd::Message(msg,wp,lp);
}

void SplitterWnd::Collapse() {
	RECT rc;
	GetClientRect(_wnd,&rc);

	float full = _orientation==OrientationHorizontal?float(rc.bottom-rc.top):float(rc.right-rc.left);
	float relativeBarSize = (float(barHeight) / full)/2.0f;

	_ratio = 1.0f-relativeBarSize;
	Layout();
	Repaint();
}

wchar_t SplitterWnd::GetPreferredHotkey() {
	return L'\0';
}

void SplitterWnd::Update() {
	if(_a) _a->Update();
	if(_b) _b->Update();
}

bool SplitterWnd::IsInHotkeyMode() {
	Wnd* parent = GetParent();
	if(parent==0) return false;
	return parent->IsInHotkeyMode();
}