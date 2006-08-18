#include "../include/tjshared.h"
#include <iomanip>
#include <windowsx.h>
using namespace Gdiplus;
using namespace tj::shared;

ToolbarItem::ToolbarItem(int command, Gdiplus::Bitmap* bmp) {
	_separator = false;
	_icon = bmp;
	_command = command;
}

ToolbarItem::~ToolbarItem() {
	delete _icon;
};

bool ToolbarItem::IsSeparator() const {
	return _separator;
}

void ToolbarItem::SetSeparator(bool s) {
	_separator = s;
}

ToolbarWnd::ToolbarWnd(HWND parent): ChildWnd(L"", parent) {
	SetWantMouseLeave(true);
	_in = false;
}

ToolbarWnd::~ToolbarWnd() {
}

void ToolbarWnd::Add(ref<ToolbarItem> item) {
	_items.push_back(item);
}

void ToolbarWnd::OnCommand(int c) {
}

void ToolbarWnd::Layout() {
}

void ToolbarWnd::Fill(LayoutFlags f, tj::shared::Rectangle& r) {
	if(f==LayoutTop) {
		SetWindowPos(_wnd, 0, r.GetLeft(), r.GetTop(), r.GetWidth(), ButtonSize, SWP_NOZORDER);
		r.Narrow(0,ButtonSize,0,0);
	}
	else if(f==LayoutBottom) {
		SetWindowPos(_wnd, 0, r.GetLeft(), r.GetTop()+r.GetHeight()-ButtonSize, r.GetWidth(), ButtonSize, SWP_NOZORDER);
		r.Narrow(0,0,0,ButtonSize);
	}
	else {
		ChildWnd::Fill(f, r);
	}
}

wchar_t ToolbarWnd::GetPreferredHotkey() {
	return L'T';
}

LRESULT ToolbarWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_MOUSEMOVE||msg==WM_MOUSEHOVER||msg==WM_LBUTTONDOWN) {
		_in = true;
		// track leave event
		Repaint();
	}
	else if(msg==WM_MOUSELEAVE) {
		_in = false;
		Repaint();
	}
	else if(msg==WM_LBUTTONUP) {
		int idx = GET_X_LPARAM(lp)/ButtonSize;
		if(idx>=0 && idx < (int)_items.size()) {
			ref<ToolbarItem> item = _items.at(idx);
			OnCommand(item->_command);
		}
		Repaint();
	}
	else if(msg==WM_TIMER) {
		Repaint();
	}
	return ChildWnd::Message(msg,wp,lp);
}

void ToolbarWnd::Paint(Gdiplus::Graphics& g) {
	POINT cp;
	RECT wrc;
	GetWindowRect(_wnd, &wrc);
	GetCursorPos(&cp);
	cp.x -= wrc.left; cp.y -= wrc.top;
	int mouseOver = cp.x/ButtonSize;

	tj::shared::Rectangle rc = GetClientRectangle();
	ref<Theme> theme = ThemeManager::GetTheme();
	
	SolidBrush zwart(theme->GetBackgroundColor());
	g.FillRectangle(&zwart, rc);

	LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetToolbarColorStart(), theme->GetToolbarColorEnd());
	SolidBrush dbr(theme->GetDisabledOverlayColor());
	g.FillRectangle(&br, rc);
	g.FillRectangle(&dbr, rc);

	LinearGradientBrush glas(PointF(0.0f,0.0f), PointF(0.0f,float(rc.GetHeight())/2.0f), theme->GetGlassColorStart(), theme->GetGlassColorEnd());
	g.FillRectangle(&glas, RectF(0.0f, 0.0f, float(rc.GetWidth()), float(rc.GetHeight())/2.0f));

	Pen pn(theme->GetActiveEndColor(), 1.0f);
	g.DrawLine(&pn, PointF(0.0f, float(rc.GetHeight()-1.0f)), PointF(float(rc.GetWidth()), float(rc.GetHeight()-1.0f)));

	std::vector< ref<ToolbarItem> >::iterator it = _items.begin();
	int x = 0;
	int idx = 0;
	while(it!=_items.end()) {
		ref<ToolbarItem> item = *it;
		if(mouseOver==idx && _in) {
			if(ISVKKEYDOWN(VK_LBUTTON)) {
				LinearGradientBrush active(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetHighlightColorStart(), theme->GetHighlightColorEnd());
				g.FillRectangle(&active, RectF(float(x)+1.0f, 1.0f, 22.0f, 21.0f));
			}
			else {
				LinearGradientBrush active(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetActiveStartColor(), theme->GetActiveEndColor());
				g.FillRectangle(&active, RectF(float(x)+1.0f, 1.0f, 22.0f, 21.0f));
			}
			g.FillRectangle(&glas, RectF(float(x)+1.0f, 1.0f, float(ButtonSize)-2.0f, float(rc.GetHeight())-3.0f));
		}

		g.DrawImage(item->_icon, RectF(float(x)+4.0f, 4.0f, 16.0f, 16.0f));

		if(item->_separator) {
			Pen pn(theme->GetActiveStartColor());
			g.DrawLine(&pn, PointF(float(x)+24.0f, 4.0f), PointF(float(x)+24.0f, float(rc.GetHeight())-4.0f));
		}

		x += ButtonSize;
		it++;
		idx++;
	}
}