#include "../include/tjsharedui.h"
#include <math.h>

#ifdef TJ_OS_WIN
	// TODO: is windowsx.h used anyhwere?
	#include <windowsx.h>
#endif

using namespace tj::shared::graphics;
using namespace tj::shared;

const float SplitterWnd::KSnapMargin = 0.07f;
const Pixels SplitterWnd::KBarHeight = 6;

SplitterWnd::SplitterWnd(Orientation o): ChildWnd(L"Splitter"), _currentWidth(0), _resizeMode(ResizeModeEqually), _collapse(CollapseNone), _ratio(0.618f), _ratioBeforeDragging(0.618f), _dragging(false), _orientation(o) {
	#ifdef TJ_OS_WIN
		SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	#endif
	
	SetWantMouseLeave(true);
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

SplitterWnd::ResizeMode SplitterWnd::GetResizeMode() const {
	return _resizeMode;
}

void SplitterWnd::SetResizeMode(SplitterWnd::ResizeMode rm) {
	_resizeMode = rm;
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
		_ratioBeforeDragging = _ratio;
	}
	else if(cm==L"second") {
		_collapse = CollapseSecond;
		_ratioBeforeDragging = _ratio;
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
		strong<Theme> theme = ThemeManager::GetTheme();
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
		strong<Theme> theme = ThemeManager::GetTheme();
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
			int heightA = GetWidthOfFirstPane(rc.GetHeight());
			int heightB = GetWidthOfSecondPane(rc.GetHeight());

			if(_a) {
				_a->Move(0, 0, rc.GetWidth(), heightA-(KBarHeight/2));
			}

			if(_b) {
				_b->Move(0, heightA+(KBarHeight/2)-1, rc.GetWidth(), heightB-(KBarHeight/2));
			}
		}
		else if(_orientation==OrientationVertical) {
			int widthA = GetWidthOfFirstPane(rc.GetWidth());
			int widthB = GetWidthOfSecondPane(rc.GetWidth());

			if(_a) {
				_a->Move(0, 0, widthA-(KBarHeight/2), rc.GetHeight());
			}

			if(_b) {
				_b->Move(widthA+(KBarHeight/2), 0, widthB-3,rc.GetHeight());
			}
		}
	}
}

Pixels SplitterWnd::GetWidthOfFirstPane(Pixels totalWidth) {
	return (Pixels)floor(_ratio*totalWidth);
}

Pixels SplitterWnd::GetWidthOfSecondPane(Pixels totalWidth) {
	return totalWidth-GetWidthOfFirstPane(totalWidth);
}

void SplitterWnd::SetWidthOfFirstPane(Pixels paneWidth, Pixels totalWidth) {
	_ratio = float(paneWidth) / float(totalWidth);
}

void SplitterWnd::SetWidthOfSecondPane(Pixels paneHeight, Pixels totalWidth) {
	SetWidthOfFirstPane(totalWidth - paneHeight, totalWidth);
}

void SplitterWnd::SetRatio(float f) {
	if(_ratio>(1.0f-KSnapMargin)) {
		Collapse(CollapseFirst);
	}
	else if(_ratio<KSnapMargin) {
		Collapse(CollapseSecond);
	}
	else {
		_ratio = f;
		_defaultRatio = f;

		// save ratio as a setting
		ref<Settings> st = GetSettings();
		if(st) {
			st->SetValue(L"ratio", Stringify(_ratio));
		}
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
	strong<Theme> theme = ThemeManager::GetTheme();
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

void SplitterWnd::Paint(Graphics& g, strong<Theme> theme) {
	Area rc = GetClientArea();
	
	if(_collapse==CollapseNone) {
		HWND root = GetAncestor(GetWindow(), GA_ROOT);
		graphics::Brush* abr = theme->GetApplicationBackgroundBrush(root, GetWindow());
		if(abr!=0) {
			g.FillRectangle(abr, rc);
		}
		delete abr;
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
		SolidBrush stbr(Theme::ChangeAlpha(theme->GetColor(Theme::ColorBackground),172));
		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);

		GraphicsContainer gc = g.BeginContainer();

		float offset = 0.0f;
		if(_orientation==OrientationVertical) {
			g.TranslateTransform(float(bar.GetLeft())+20.0f, float(bar.GetTop())+3.0f);
			g.RotateTransform(90.0f);
		}
		else {
			g.TranslateTransform(float(bar.GetLeft())+3.0f, float(bar.GetTop())+4.0f);
		}
		
		if(icon) {
			g.DrawImage(icon->GetBitmap(), RectF(0.0f, 0.0f,14.0f,14.0f));
			offset += 20.0f;
		}

		g.DrawString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), PointF(offset+0.8f, 0.8f), &sf, &stbr);
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

void SplitterWnd::OnSize(const Area& ns) {
	// calculate new ratio if resize mode is not ResizeModeEqually
	Pixels currentWidth = ((_orientation == OrientationVertical) ? ns.GetWidth() : ns.GetHeight());
	if(_a && _b && _collapse == CollapseNone) {
		if(_resizeMode==ResizeModeLeftOrTop) {
			// right/bottom stays same size
			SetWidthOfSecondPane(GetWidthOfSecondPane(_currentWidth), currentWidth);
		}
		else if(_resizeMode==ResizeModeRightOrBottom) {
			// left/top stays same size
			SetWidthOfFirstPane(GetWidthOfFirstPane(_currentWidth), currentWidth);
		}
	}
	
	_currentWidth = currentWidth;
	Layout();
	Repaint();
}

void SplitterWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventMove) {
		if(_dragging) {
			Area rc = GetClientArea();
			float limit = KSnapMargin / 2.0f;
			if(_orientation==OrientationHorizontal) {
				_ratio = Clamp(float(y)/float(rc.GetHeight()), 0.0f + limit, 1.0f - limit);
			}
			else if(_orientation==OrientationVertical) {
				_ratio = Clamp(float(x)/float(rc.GetWidth()), 0.0f + limit, 1.0f - limit);
			}

			Layout();

			#ifdef TJ_OS_WIN
				/** UpdateWindow calls the window procedure of _a and _b directly, so the repainting looks
				more smooth than using the (asynchronous) Repaint, which just does InvalidateRect() which sends
				a message to the window. Of course, the windows themselves already call Repaint when necessary
				from their Layout methods, so this is not really needed.
				
				See http://msdn2.microsoft.com/en-us/library/ms534874.aspx.
				**/
				UpdateWindow(_a->GetWindow());
				UpdateWindow(_b->GetWindow());
				UpdateWindow(GetWindow());
			#endif
		}
		else {
			if(_collapse==CollapseNone) {
				if(_orientation==OrientationHorizontal) {
					Mouse::Instance()->SetCursorType(CursorSizeNorthSouth);

				}
				else if(_orientation==OrientationVertical) {
					Mouse::Instance()->SetCursorType(CursorSizeEastWest);
				}
			}
			else {
				Mouse::Instance()->SetCursorType(CursorDefault);
			}
		}
	}
	else if(ev==MouseEventLDown) {
		if(_collapse==CollapseNone) {
			_ratioBeforeDragging = _ratio;
			Mouse::Instance()->SetCursorType(_orientation==OrientationHorizontal ? CursorSizeNorthSouth : CursorSizeEastWest);
			_capture.StartCapturing(Mouse::Instance(), ref<Wnd>(this));
			_dragging = true;
			Repaint();
		}
	}
	else if(ev==MouseEventLUp) {
		if(_collapse==CollapseNone) {
			_dragging = false;
			_capture.StopCapturing();
			Mouse::Instance()->SetCursorType(CursorDefault);

			// If we're close to the borders, collapse
			SetRatio(_ratio);
		}
		else {
			Expand();
		}
	}
	else if(ev==MouseEventLDouble && _collapse==CollapseNone) {
		Collapse(CollapseFirst);
	}
	else if(ev==MouseEventRDouble && _collapse==CollapseNone) {
		Collapse(CollapseSecond);
	}
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
	_ratio = _ratioBeforeDragging;

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

/** SidebarWnd **/
SidebarWnd::SidebarWnd(ref<Wnd> child): 
	_child(child),
	_closeIcon(Icons::GetIconPath(Icons::IconTabClose)), 
	_closeIconActive(Icons::GetIconPath(Icons::IconTabCloseActive)) {

}

SidebarWnd::~SidebarWnd() {
}

void SidebarWnd::OnCreated() {
	Add(_child);
	Layout();
}

void SidebarWnd::Layout() {
	Area rc = GetClientArea();
	strong<Theme> theme = ThemeManager::GetTheme();
	Pixels barHeight = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);

	ref<Wnd> parent = GetParent();
	if(parent && parent.IsCastableTo<SplitterWnd>()) {
		ref<SplitterWnd> splitter = parent;
		if(splitter) {
			rc.Narrow(0,barHeight,0,0);
		}
	}

	if(_child) {
		_child->Fill(LayoutFill, rc, true);
	}
}

std::wstring SidebarWnd::GetTabTitle() const {
	if(_child) {
		return _child->GetTabTitle();
	}
	return L"";
}

ref<Icon> SidebarWnd::GetTabIcon() const {
	if(_child) {
		return _child->GetTabIcon();
	}
	return null;
}

void SidebarWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLUp) {
		ref<Wnd> parent = GetParent();
		if(parent && parent.IsCastableTo<SplitterWnd>()) {
			ref<SplitterWnd> splitter = parent;
			if(splitter) {
				if(splitter->_a==ref<Wnd>(this)) {
					splitter->Collapse(SplitterWnd::CollapseSecond);
				}
				else if(splitter->_b==ref<Wnd>(this)) {
					splitter->Collapse(SplitterWnd::CollapseFirst);
				}
			}
		}
	}
	else if(ev==MouseEventMove||ev==MouseEventLDown) {
		Repaint();
		SetWantMouseLeave(true);
	}
	else if(ev==MouseEventLeave) {
		Repaint();
	}
	ChildWnd::OnMouse(ev,x,y);
}

void SidebarWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	Pixels barHeight = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	Area rc = GetClientArea();
	Area bar = rc;
	bar.SetHeight(barHeight);

	bool leftDown = Mouse::Instance()->IsButtonDown(Mouse::ButtonLeft);

	// Draw background
	LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, barHeight+1.0f), theme->GetColor(leftDown ? Theme::ColorActiveStart : Theme::ColorActiveEnd), theme->GetColor(leftDown ? Theme::ColorActiveEnd : Theme::ColorActiveStart));
	g.FillRectangle(&lbr, bar);
	Area dbrRC = bar;
	dbrRC.Narrow(0,0,0,1);
	SolidBrush dbr(theme->GetColor(Theme::ColorDisabledOverlay));
	g.FillRectangle(&dbr, dbrRC);

	if(_child) {
		// Draw icon
		Area titleRC = bar;
		titleRC.Narrow(4,4,4,4);
		
		ref<Icon> tabIcon = _child->GetTabIcon();
		if(tabIcon) {
			tabIcon->Paint(g, Area(rc.GetLeft()+2, rc.GetTop()+2, 14, 14));
			titleRC.Narrow(20,0,0,0);
		}

		// Draw title
		std::wstring title = GetTabTitle();
		SolidBrush tbr(theme->GetColor(Theme::ColorText));
		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);
		sf.SetTrimming(StringTrimmingEllipsisPath);
		AreaF titleShadowRC = titleRC;
		titleShadowRC.Translate(0.8f, 0.8f);
		SolidBrush shadowBrush(Theme::ChangeAlpha(theme->GetColor(Theme::ColorBackground),172));
		g.DrawString(title.c_str(), (size_t)title.length(), theme->GetGUIFontBold(), titleShadowRC, &sf, &shadowBrush);
		g.DrawString(title.c_str(), (size_t)title.length(), theme->GetGUIFontBold(), titleRC, &sf, &tbr);
	}

	// Draw close icon
	Area iconRC = bar;
	iconRC.Narrow(bar.GetWidth()-19, 0, 0, 0);
	iconRC.SetHeight(19);
	iconRC.SetWidth(19);

	Coord mouse = Mouse::Instance()->GetCursorPosition(this);
	if(IsMouseOver() && mouse._y < barHeight) {
		_closeIconActive.Paint(g,iconRC);
	}
	else {
		_closeIcon.Paint(g, iconRC);
	}
}

void SidebarWnd::OnSize(const Area& ns) {
	Layout();
	Repaint();
}

void SidebarWnd::OnSettingsChanged() {
	ref<Settings> st = GetSettings();
	if(_child) {
		_child->SetSettings(st);
	}
}