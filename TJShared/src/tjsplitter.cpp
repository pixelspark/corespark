#include "../include/tjshared.h"
using namespace Gdiplus;

const float SplitterWnd::snapMargin = 0.07f;

SplitterWnd::SplitterWnd(HWND parent, ref<Wnd> a, ref<Wnd> b, Orientation o): ChildWnd(L"Splitter", parent, true, false) {
	assert(a!=0);
	assert(b!=0);
	SetStyle(WS_CLIPCHILDREN);
	SetStyle(WS_CLIPSIBLINGS);
	SetParent(a->GetWindow(),_wnd);
	SetParent(b->GetWindow(),_wnd);
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
	if(_a->GetPreferredHotkey()==key) {
		_a->EnterHotkeyMode();
	}
	else if(_b->GetPreferredHotkey()==key) {
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
		int heightB = (rc.bottom-rc.top)-heightA;

		SetWindowPos(_a->GetWindow(), 0, 0, 0, rc.right-rc.left, heightA-(barHeight/2),SWP_NOZORDER);
		SetWindowPos(_b->GetWindow(), 0, 0, heightA+(barHeight/2), rc.right-rc.left, heightB-(barHeight/2),SWP_NOZORDER);
	}
	else if(_orientation==OrientationVertical) {
		int widthA = (int)floor(_ratio*(rc.right-rc.left));
		int widthB = (rc.right-rc.left)-widthA;

		SetWindowPos(_a->GetWindow(), 0, 0, 0, widthA-(barHeight/2), rc.bottom-rc.top, SWP_NOZORDER);
		SetWindowPos(_b->GetWindow(), 0, widthA+(barHeight/2), 0, widthB-(barHeight/2),rc.bottom-rc.top,SWP_NOZORDER);
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

	RECT r;
	GetClientRect(_wnd,&r);
	if(_orientation==OrientationHorizontal) {
		int bH = int(_ratio * (r.bottom-r.top)-(barHeight/2)); // top of the bar

		LinearGradientBrush br(PointF(0, REAL(bH-1)),PointF(0, REAL(bH+barHeight+2)), theme->GetSplitterStartColor(), theme->GetSplitterEndColor());
		g.FillRectangle(&br, 0,bH-2,r.right-r.left, barHeight+4);
	}
	else if(_orientation==OrientationVertical) {
		int bH = int(_ratio * (r.right-r.left)-(barHeight/2)); // top of the bar

		LinearGradientBrush br(PointF(REAL(bH-1),0),PointF(REAL(bH+barHeight+2),0), theme->GetSplitterStartColor(), theme->GetSplitterEndColor());
		g.FillRectangle(&br, bH-2, 0, barHeight+4, r.bottom-r.top);
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
	}
	else if(msg==WM_LBUTTONUP) {
		_dragging = false;
		ReleaseCapture();
		SetCursor(LoadCursor(0,IDC_ARROW));
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
	_a->Update();
	_b->Update();
}

bool SplitterWnd::IsInHotkeyMode() {
	Wnd* parent = GetParent();
	if(parent==0) return false;
	return parent->IsInHotkeyMode();
}