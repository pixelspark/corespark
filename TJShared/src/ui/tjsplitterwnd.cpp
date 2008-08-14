#include "../../include/ui/tjui.h" 
#include <math.h>
#include <windowsx.h>
using namespace tj::shared::graphics;
using namespace tj::shared;

const float SplitterWnd::KSnapMargin = 0.07f;
const Pixels SplitterWnd::KBarHeight = 6;

SplitterWnd::SplitterWnd(Orientation o): ChildWnd(L"Splitter", true, true) {
	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	SetStyleEx(WS_EX_CONTROLPARENT);
	SetWantMouseLeave(true);
	_collapse = CollapseNone;

	_ratio = 0.618f; // Fibonacci :-)
	_ratioBeforeDragging = 0.5f; // Gets changed when dragging
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

bool SplitterWnd::IsCollapsed() const {
	return _collapse != CollapseNone;
}

void SplitterWnd::OnSettingsChanged() {
	ref<Settings> st = GetSettings();

	if(_a) _a->SetSettings(st->GetNamespace(L"first"));
	if(_b) _b->SetSettings(st->GetNamespace(L"second"));

	_ratio = StringTo<float>(st->GetValue(L"ratio", Stringify(_ratio)), _ratio);
	_defaultRatio = _ratio;

	// Load collapse mode
	std::wstring cm = st->GetValue(L"collapse", L"");
	if(cm==L"first") {
		_collapse = CollapseFirst;
	}
	else if(cm==L"second") {
		_collapse = CollapseSecond;
	}
	else {
		_collapse = CollapseNone;
	}

	Layout();
	Repaint();
}

void SplitterWnd::Layout() {
	Area rc = GetClientArea();
	
	if(_collapse==CollapseFirst) {
		ref<Theme> theme = ThemeManager::GetTheme();
		if(_orientation==OrientationVertical) {
			rc.Narrow(0,0,theme->GetMeasureInPixels(Theme::MeasureToolbarHeight),0);
		}
		else {
			rc.Narrow(0,0,0,theme->GetMeasureInPixels(Theme::MeasureToolbarHeight));
		}

		if(_a) _a->Fill(LayoutFill, rc);
		if(_b) _b->Show(false);
	}
	else if(_collapse==CollapseSecond) {
		ref<Theme> theme = ThemeManager::GetTheme();
		if(_orientation==OrientationVertical) {
			rc.Narrow(theme->GetMeasureInPixels(Theme::MeasureToolbarHeight),0,0,0);
		}
		else {
			rc.Narrow(0,theme->GetMeasureInPixels(Theme::MeasureToolbarHeight),0,0);
		}

		if(_b) _b->Fill(LayoutFill, rc);
		if(_a) _a->Show(false);
	}
	else {
		if(_a) _a->Show(true);
		if(_b) _b->Show(true);
		if(_orientation==OrientationHorizontal) {
			int heightA = (int)floor(_ratio*(rc.GetHeight()));
			int heightB = rc.GetHeight()-heightA;

			if(_a) {
				_a->Move(0, 0, rc.GetWidth(), heightA-(KBarHeight/2));
			}

			if(_b) {
				_b->Move(0, heightA+(KBarHeight/2)-1, rc.GetWidth(), heightB-(KBarHeight/2));
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
	Collapse(CollapseNone);
	Layout();
	Repaint();
}

Area SplitterWnd::GetBarArea() {
	Area bar;
	Area rc = GetClientArea();
	ref<Theme> theme = ThemeManager::GetTheme();
	Pixels barWidth = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	if(_collapse==CollapseFirst) { // First one is visible, so bar on the right
		if(_orientation==OrientationVertical) {
			bar = Area(rc.GetWidth()-barWidth+1, 0, barWidth, rc.GetHeight());
		}
		else {
			bar = Area(0, rc.GetHeight()-barWidth, rc.GetWidth(), barWidth);
		}
	}
	else { // second one visible, bar on the left
		if(_orientation==OrientationVertical) {
			bar = Area(0, 0, barWidth, rc.GetHeight());
		}
		else {
			bar = Area(0, 0, rc.GetWidth(), barWidth);
		}
	}

	return bar;
}

void SplitterWnd::Paint(Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();
	
	if(_collapse==CollapseNone) {
		HWND root = GetAncestor(GetWindow(), GA_ROOT);
		graphics::Brush* abr = theme->GetApplicationBackgroundBrush(root, GetWindow());
		if(abr!=0) {
			g.FillRectangle(abr, rc);
		}
	}
	else {
		Area bar = GetBarArea();
		bar.Narrow(-1,-1,-1,-1);
		PointF start, end;
		if(_orientation==OrientationVertical) {
			end = PointF(float(bar.GetLeft()),0.0f);
			start = PointF(float(bar.GetRight()), 0.0f);
		}
		else {
			end = PointF(0.0f, float(bar.GetTop()));
			start = PointF(0.0f, float(bar.GetBottom()));
		}

		LinearGradientBrush lbr(start, end, theme->GetColor(Theme::ColorProgressBackgroundStart), theme->GetColor(Theme::ColorProgressBackgroundEnd));
		SolidBrush back(theme->GetColor(Theme::ColorBackground));
		SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
		g.FillRectangle(&back, bar);
		g.FillRectangle(&lbr,bar);
		//g.FillRectangle(&disabled,bar);

		// Draw a border
		Pen borderPen(theme->GetColor(Theme::ColorActiveEnd), 1.0f);
		if(_orientation==OrientationVertical) {
			// to the left of the bar
			g.DrawLine(&borderPen, float(bar.GetLeft()), float(bar.GetTop()), float(bar.GetLeft()), float(bar.GetBottom()));
		}
		else {
			// to the bottom of the bar
			g.DrawLine(&borderPen, float(bar.GetLeft()), float(bar.GetBottom()), float(bar.GetRight()), float(bar.GetBottom()));
		}

		// Draw text
		std::wstring title;
		ref<Icon> icon;
		if(_collapse==CollapseFirst && _b) {
			title = _b->GetTabTitle();
			icon = _b->GetTabIcon();
		}
		else if(_collapse==CollapseSecond && _a) {
			title = _a->GetTabTitle();
			icon = _a->GetTabIcon();
		}

		SolidBrush tbr(theme->GetColor(Theme::ColorActiveStart));
		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);

		GraphicsContainer gc = g.BeginContainer();

		float offset = 0.0f;
		if(_orientation==OrientationVertical) {
			g.TranslateTransform(float(bar.GetLeft())+20.0f, float(bar.GetTop())+3.0f);
			g.RotateTransform(90.0f);
		}
		else {
			g.TranslateTransform(float(bar.GetLeft())+3.0f, float(bar.GetTop())+3.0f);
		}
		
		if(icon) {
			g.DrawImage(icon->GetBitmap(), RectF(0.0f, 0.0f,14.0f,14.0f));
			offset += 20.0f;
		}

		g.DrawString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), PointF(offset, 0.0f), &sf, &tbr);
		g.EndContainer(gc);
	}
}

std::wstring SplitterWnd::GetTabTitle() const {
	std::wstring title;
	bool more = false;
	if(_a) {
		title += _a->GetTabTitle();
		more = true;
	}

	if(_b) {
		if(more) {
			title += L" & ";
		}
		title += _b->GetTabTitle();
	}

	return title;
}

LRESULT SplitterWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE) {
		Layout();
		Repaint();
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

			/** UpdateWindow calls the window procedure of _a and _b directly, so the repainting looks
			more smooth than using the (asynchronous) Repaint, which just does InvalidateRect() which sends
			a message to the window.
			
			See http://msdn2.microsoft.com/en-us/library/ms534874.aspx.
			**/
			Layout();
			UpdateWindow(_a->GetWindow());
			UpdateWindow(_b->GetWindow());
			UpdateWindow(GetWindow());
		}
		else {
			if(_collapse==CollapseNone) {
				if(_orientation==OrientationHorizontal) {
					SetCursor(LoadCursor(0,IDC_SIZENS));

				}
				else if(_orientation==OrientationVertical) {
					SetCursor(LoadCursor(0,IDC_SIZEWE));
				}
			}
			else {
				SetCursor(LoadCursor(0, IDC_ARROW));
			}
		}
	}
	else if(msg==WM_LBUTTONDOWN) {
		if(_collapse==CollapseNone) {
			SetCursor(LoadCursor(0,_orientation==OrientationHorizontal?IDC_SIZENS:IDC_SIZEWE));
			SetCapture(GetWindow());
			_ratioBeforeDragging = _ratio;
			_dragging = true;
			Repaint();
		}
	}
	else if(msg==WM_LBUTTONUP) {
		if(_collapse==CollapseNone) {
			_dragging = false;
			ReleaseCapture();
			SetCursor(LoadCursor(0,IDC_ARROW));

			// If we're close to the borders, collapse
			if(_ratio>(1.0f-KSnapMargin)) {
				_ratio = _ratioBeforeDragging;
				Collapse(CollapseFirst);
				_dragging = false;
				ReleaseCapture();
			}
			else if(_ratio<KSnapMargin) {
				_ratio = _ratioBeforeDragging;
				Collapse(CollapseSecond);
				_dragging = false;
				ReleaseCapture();
			}
			else {
				// save ratio as a setting
				ref<Settings> st = GetSettings();
				if(st) {
					st->SetValue(L"ratio", Stringify(_ratio));
				}
			}
		}
		else {
			Collapse(CollapseNone);
		}
	}
	else if(msg==WM_LBUTTONDBLCLK && _collapse==CollapseNone) {
		Collapse(CollapseFirst);
	}
	else if(msg==WM_RBUTTONDBLCLK && _collapse==CollapseNone) {
		Collapse(CollapseSecond);
	}

	return ChildWnd::Message(msg,wp,lp);
}

void SplitterWnd::Collapse(CollapseMode cm) {
	_collapse = cm;

	// Stringify collapse mode
	std::wstring cms = L"none";
	if(cm==CollapseFirst) {
		cms = L"first";
	}
	else if(cm==CollapseSecond) {
		cms = L"second";
	}

	// Save it to settings
	ref<Settings> st = GetSettings();
	if(st) {
		st->SetValue(L"collapse", cms);
	}

	Layout();
	Repaint();
}

void SplitterWnd::Update() {
	if(_a) _a->Update();
	if(_b) _b->Update();
}
