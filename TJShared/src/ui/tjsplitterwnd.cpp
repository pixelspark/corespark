#include "../../include/tjshared.h"
#include <math.h>
#include <windowsx.h>
using namespace Gdiplus;
using namespace tj::shared;

const float SplitterWnd::KSnapMargin = 0.07f;
const Pixels SplitterWnd::KBarHeight = 6;

SplitterWnd::SplitterWnd(Orientation o): ChildWnd(L"Splitter", true, false) {
	SetStyle(WS_CLIPCHILDREN);
	SetStyle(WS_CLIPSIBLINGS);
	_collapse = CollapseNone;

	_ratio = 0.618f; // Fibonacci :-)
	_defaultRatio = 0.618f;
	_dragging = false;
	_orientation = o;
	Layout();
}

SplitterWnd::~SplitterWnd() {
}

Orientation SplitterWnd::GetOrientation() const {
	return _orientation;
}

void SplitterWnd::SetOrientation(Orientation o) {
	_orientation = o;
	Layout();
}

// Do not use, use SetFirst and SetSecond instead (Add will throw an exception)
void SplitterWnd::Add(ref<Wnd> child) {
	Throw(L"You cannot use Add() on a splitter; use SetFirst and SetSecond instead", ExceptionTypeSevere);
}

void SplitterWnd::SetFirst(ref<Wnd> child) {
	_a = child;
	SetParent(_a->GetWindow(), GetWindow());

	_a->Show(true);
	Layout();
}

void SplitterWnd::SetSecond(ref<Wnd> child) {
	_b = child;
	SetParent(_b->GetWindow(), GetWindow());
	_b->Show(true);
	Layout();
}

bool SplitterWnd::IsSplitter() {
	return true;
}

ref<Wnd> SplitterWnd::GetFirst() {
	return _a;
}

ref<Wnd> SplitterWnd::GetSecond() {
	return _b;
}

const ref<Wnd> SplitterWnd::GetFirst() const {
	return _a;
}

const ref<Wnd> SplitterWnd::GetSecond() const {
	return _b;
}

void SplitterWnd::OnSettingsChanged() {
	ref<Settings> st = GetSettings();

	if(_a) _a->SetSettings(st->GetNamespace(L"first"));
	if(_b) _b->SetSettings(st->GetNamespace(L"second"));

	_ratio = StringTo<float>(st->GetValue(L"ratio", Stringify(_ratio)), _ratio);
	_defaultRatio = _ratio;
	Layout();
	Repaint();
}

void SplitterWnd::Layout() {
	Area rc = GetClientArea();
	
	if(_collapse==CollapseFirst) {
		if(_a) _a->Fill(LayoutFill, rc);
		if(_b) _b->Show(false);
	}
	else if(_collapse==CollapseSecond) {
		if(_b) _b->Fill(LayoutFill, rc);
		if(_a) _a->Show(false);
	}
	else {
		rc.Narrow(1,1,1,1);
		if(_a) _a->Show(true);
		if(_b) _b->Show(true);
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
}

void SplitterWnd::SetRatio(float f) {
	_collapse = CollapseNone;
	_ratio = f;
	_defaultRatio = f;

	ref<Settings> st = GetSettings();
	if(st) {
		st->SetValue(L"ratio", Stringify(_ratio));
	}

	Layout();
	Repaint();
}

void SplitterWnd::Expand() {
	_collapse = CollapseNone;
	Layout();
	Repaint();
}

void SplitterWnd::Paint(Graphics& g) {
	if(_collapse==CollapseNone) {
		ref<Theme> theme = ThemeManager::GetTheme();
		Area rc = GetClientArea();

		HWND root = GetAncestor(GetWindow(), GA_ROOT);
		Gdiplus::Brush* abr = theme->GetApplicationBackgroundBrush(root, GetWindow());
		if(abr!=0) {
			g.FillRectangle(abr, rc);
		}
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
				SetCursor(LoadCursor(0,IDC_SIZENS));

			}
			else if(_orientation==OrientationVertical) {
				int bH = int(_ratio * (rc.right-rc.left)-(KBarHeight/2));
				SetCursor(LoadCursor(0,IDC_SIZEWE));
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

		// save ratio as a setting
		ref<Settings> st = GetSettings();
		if(st) {
			st->SetValue(L"ratio", Stringify(_ratio));
		}
	}
	else if(msg==WM_LBUTTONDBLCLK) {
		Collapse();
	}

	return ChildWnd::Message(msg,wp,lp);
}

void SplitterWnd::Collapse(CollapseMode cm) {
	_collapse = cm;
	Layout();
	Repaint();
}

void SplitterWnd::Update() {
	if(_a) _a->Update();
	if(_b) _b->Update();
}
