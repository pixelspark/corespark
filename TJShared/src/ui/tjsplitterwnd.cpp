#include "../../include/tjshared.h"
#include <math.h>
#include <windowsx.h>
using namespace Gdiplus;
using namespace tj::shared;

const float SplitterWnd::KSnapMargin = 0.07f;
const Pixels SplitterWnd::KBarHeight = 6;

SplitterWnd::SplitterWnd(HWND parent, ref<Wnd> a, ref<Wnd> b, Orientation o): ChildWnd(L"Splitter", parent, true, false) {
	SetStyle(WS_CLIPCHILDREN);
	SetStyle(WS_CLIPSIBLINGS);
	if(a) {
		SetParent(a->GetWindow(),GetWindow());
	}

	if(b) {
		SetParent(b->GetWindow(),GetWindow());
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

bool SplitterWnd::IsSplitter() {
	return true;
}

void SplitterWnd::Layout() {
	Area rc = GetClientArea();
	rc.Narrow(1,1,1,1);

	if(_orientation==OrientationHorizontal) {
		int heightA = (int)floor(_ratio*(rc.GetHeight()))-KBarHeight;
		int heightB = rc.GetHeight()-heightA;

		if(_a) {
			_a->Move(0, 0, rc.GetWidth(), heightA-KBarHeight);
		}

		if(_b) {
			_b->Move(0, heightA+KBarHeight-1, rc.GetWidth(), heightB-3);
		}
	}
	else if(_orientation==OrientationVertical) {
		int widthA = (int)floor(_ratio*(rc.GetWidth()))-KBarHeight;
		int widthB = rc.GetWidth()-widthA;

		if(_a) {
			_a->Move(0, 0, widthA, rc.GetHeight());
		}

		if(_b) {
			_b->Move(widthA+KBarHeight-1, 0, widthB-3,rc.GetHeight());
		}
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
	Area rc = GetClientArea();

	HWND root = GetAncestor(GetWindow(), GA_ROOT);
	Gdiplus::Brush* abr = theme->GetApplicationBackgroundBrush(root, GetWindow());
	if(abr!=0) {
		/*if(_orientation==OrientationHorizontal) {
			int bH = int(_ratio * rc.GetHeight()); // -(KBarHeight/2)); // top of the bar
			g.FillRectangle(abr, 0,bH-2,rc.GetWidth(), KBarHeight+4);
		}
		else if(_orientation==OrientationVertical) {
			int bH = int(_ratio * rc.GetWidth()); //-(KBarHeight/2)); // top of the bar
			g.FillRectangle(abr, bH-2, 0, KBarHeight+4, rc.GetHeight());
		}
		g.FillRectangle(abr, 0, rc.GetBottom()-5, rc.GetWidth(), 5);
		g.FillRectangle(abr, rc.GetRight()-5, 0, 5, rc.GetHeight());

		delete abr;*/
		g.FillRectangle(abr, rc);
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
			GetClientRect(GetWindow(),&rc);
			if(_orientation==OrientationHorizontal) {
				_ratio = float(y)/float(rc.bottom-rc.top);
			}
			else if(_orientation==OrientationVertical) {
				_ratio = float(x)/float(rc.right-rc.left);
			}

			// Snap and protect from going out of view
			float full = _orientation==OrientationHorizontal?float(rc.bottom-rc.top):float(rc.right-rc.left);
			float relativeBarSize = (float(KBarHeight) / full)/2.0f;

			if(_ratio>(1.0f-KSnapMargin)) _ratio = 1.0f-relativeBarSize;
			if(_ratio<KSnapMargin) _ratio = relativeBarSize;
			Layout();
			Repaint();
		}
		else {
			RECT rc;
			GetClientRect(GetWindow(),&rc);
			if(_orientation==OrientationHorizontal) {
				int bH = int(_ratio * (rc.bottom-rc.top)-(KBarHeight/2));
				if(y<bH+KBarHeight&&y>bH) {
					SetCursor(LoadCursor(0,IDC_SIZENS));
				}
				else {
					SetCursor(LoadCursor(0,IDC_ARROW));
				}
			}
			else if(_orientation==OrientationVertical) {
				int bH = int(_ratio * (rc.right-rc.left)-(KBarHeight/2));
				if(x<bH+KBarHeight&&x>bH) {
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
		SetCapture(GetWindow());
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
	GetClientRect(GetWindow(),&rc);

	float full = _orientation==OrientationHorizontal?float(rc.bottom-rc.top):float(rc.right-rc.left);
	float relativeBarSize = (float(KBarHeight) / full)/2.0f;

	_ratio = 1.0f-relativeBarSize;
	Layout();
	Repaint();
}

void SplitterWnd::Update() {
	if(_a) _a->Update();
	if(_b) _b->Update();
}
