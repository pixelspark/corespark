#include "../../include/tjshared.h"
#include <iomanip>
#include <windowsx.h>
using namespace Gdiplus;
using namespace tj::shared;

// ToolbarWnd
ToolbarWnd::ToolbarWnd(): ChildWnd(L"") {
	SetWantMouseLeave(true);
	_in = false;
	_idx = -1;
	_bk = false;
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

void ToolbarWnd::SetBackground(bool t) {
	_bk = t;
	Repaint();
}

void ToolbarWnd::SetBackgroundColor(Gdiplus::Color c) {
	_bkColor = c;
	Repaint();
}

void ToolbarWnd::OnSize(const Area& ns) {
	Layout();
}

void ToolbarWnd::Fill(LayoutFlags f, Area& r) {
	ref<Theme> theme = ThemeManager::GetTheme();
	Pixels h = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	if(f==LayoutTop) {
		Move(r.GetLeft(), r.GetTop(), r.GetWidth(), h);
		r.Narrow(0,h,0,0);
	}
	else if(f==LayoutBottom) {
		Move(r.GetLeft(), r.GetTop()-r.GetHeight()-h, r.GetWidth(), h);
		r.Narrow(0,0,0,h);
	}
	else {
		ChildWnd::Fill(f, r);
	}
}

LRESULT ToolbarWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_TIMER) {
		Repaint();
	}
	return ChildWnd::Message(msg,wp,lp);
}

void ToolbarWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	///if(msg==WM_MOUSEMOVE||msg==WM_MOUSEHOVER||msg==WM_LBUTTONDOWN) {
	if(ev==MouseEventMove||ev==MouseEventLDown) {
		_in = true;

		if(!ISVKKEYDOWN(VK_LBUTTON)||ev==MouseEventLDown) {
			ref<Theme> theme = ThemeManager::GetTheme();
			Pixels bs = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
			_idx = x/bs;
		}
		// track leave event
		Repaint();
	}
	else if(ev==MouseEventLeave) {
		_in = false;
		_idx = -1;
		Repaint();
	}
	else if(ev==MouseEventLUp) {
		if(_idx>=0 && _idx < (int)_items.size()) {
			ref<ToolbarItem> item = _items.at(_idx);
			OnCommand(item->GetCommand());
		}
		Repaint();
	}
}

int ToolbarWnd::GetTotalButtonWidth() {
	ref<Theme> theme = ThemeManager::GetTheme();
	return int(_items.size())*theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
}

void ToolbarWnd::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();
	int buttonSize = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	
	if(_bk) {
		LinearGradientBrush lbl(PointF(0.0f, 0.0f), PointF(float(rc.GetWidth()/2)+2.0f,0.0f), theme->GetBackgroundColor(), _bkColor);
		g.FillRectangle(&lbl, RectF(float(rc.GetLeft()), float(rc.GetTop()), float(rc.GetWidth()/2.0f)+2.0f, float(rc.GetHeight())));

		LinearGradientBrush lbr(PointF(float(rc.GetWidth()/2)-1.0f, 0.0f), PointF(float(rc.GetWidth())+1.0f, 0.0f), _bkColor, theme->GetBackgroundColor());
		g.FillRectangle(&lbr, RectF(float(rc.GetLeft()+rc.GetWidth()/2.0f)+1.0f, float(rc.GetTop()), float(rc.GetWidth()/2.0f), float(rc.GetHeight())));
	}
	else {
		SolidBrush zwart(theme->GetBackgroundColor());
		g.FillRectangle(&zwart, rc);

		LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetToolbarColorStart(), theme->GetToolbarColorEnd());
		SolidBrush dbr(theme->GetDisabledOverlayColor());
		g.FillRectangle(&br, rc);
		g.FillRectangle(&dbr, rc);
	}

	LinearGradientBrush glas(PointF(0.0f,0.0f), PointF(0.0f,float(rc.GetHeight())/2.0f), theme->GetGlassColorStart(), theme->GetGlassColorEnd());
	g.FillRectangle(&glas, RectF(0.0f, 0.0f, float(rc.GetWidth()), float(rc.GetHeight())/2.0f));

	Pen pn(theme->GetActiveEndColor(), 1.0f);
	g.DrawLine(&pn, PointF(0.0f, float(rc.GetHeight()-1.0f)), PointF(float(rc.GetWidth()), float(rc.GetHeight()-1.0f)));

	std::vector< ref<ToolbarItem> >::iterator it = _items.begin();
	int x = 0;
	int idx = 0;
	while(it!=_items.end()) {
		ref<ToolbarItem> item = *it;
		if(idx==_idx && _in) {
			if(ISVKKEYDOWN(VK_LBUTTON)) {
				LinearGradientBrush active(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetHighlightColorStart(), theme->GetHighlightColorEnd());
				g.FillRectangle(&active, RectF(float(x)+1.0f, 1.0f, 22.0f, 21.0f));
			}
			else {
				LinearGradientBrush active(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetActiveStartColor(), theme->GetActiveEndColor());
				g.FillRectangle(&active, RectF(float(x)+1.0f, 1.0f, 22.0f, 21.0f));
			}
			g.FillRectangle(&glas, RectF(float(x)+1.0f, 1.0f, float(buttonSize)-2.0f, float(rc.GetHeight())-3.0f));
		}

		g.DrawImage(item->GetIcon(), RectF(float(x)+4.0f, 4.0f, 16.0f, 16.0f));

		if(item->IsSeparator()) {
			Pen pn(theme->GetActiveStartColor());
			g.DrawLine(&pn, PointF(float(x)+24.0f, 4.0f), PointF(float(x)+24.0f, float(rc.GetHeight())-4.0f));
		}

		x += buttonSize;
		++it;
		idx++;
	}

	// draw description text if in & selected
	if(_in && _idx >=0 && _idx < int(_items.size())) {
		int lx = int(_items.size())*buttonSize;
		ref<ToolbarItem> item = _items.at(_idx);
		std::wstring text = item->GetText();
		SolidBrush br(theme->GetActiveEndColor());
		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);
		sf.SetLineAlignment(StringAlignmentCenter);
		g.DrawString(text.c_str(), (int)text.length(), theme->GetGUIFont(), RectF(float(lx), 0.0f, float(rc.GetWidth()), 24.0f), &sf, &br);
	}
}

// ToolbarItem
ToolbarItem::ToolbarItem(int command, Gdiplus::Bitmap* bmp, std::wstring text, bool separator): _icon(bmp) {
	_separator = separator;
	_command = command;
	_text = text;
}

ToolbarItem::ToolbarItem(int command, std::wstring rid, std::wstring text, bool separator): _icon(rid) {
	_separator = separator;
	std::wstring path = ResourceManager::Instance()->Get(rid);
	_command = command;
	_text = text;
}

ToolbarItem::~ToolbarItem() {
};

std::wstring ToolbarItem::GetText() const {
	return _text;
}

bool ToolbarItem::IsSeparator() const {
	return _separator;
}

void ToolbarItem::SetSeparator(bool s) {
	_separator = s;
}

Icon& ToolbarItem::GetIcon() {
	return _icon;
}

int ToolbarItem::GetCommand() const {
	return _command;
}

// StateToolbarItem
StateToolbarItem::StateToolbarItem(int c, std::wstring on, std::wstring off, std::wstring text): ToolbarItem(c, off,text), _onIcon(on) {
	_on = false;
}

StateToolbarItem::~StateToolbarItem() {
}

void StateToolbarItem::SetState(bool on) {
	_on = on;
}

bool StateToolbarItem::IsOn() const {
	return _on;
}

Icon& StateToolbarItem::GetIcon() {
	return _on?_onIcon:(ToolbarItem::GetIcon());
}

/* SearchToolbarWnd */
SearchToolbarWnd::SearchToolbarWnd(): _searchIcon(L"icons/search.png") {
	_rightMargin = 0;
	_searchWidth = KDefaultBoxWidth;
	_searchHeight = KDefaultBoxHeight;

	ref<Listener> lw = GC::Hold(new ListenerWrapper(this));
	_edit = GC::Hold(new EditWnd());
	_edit->SetListener(lw);
	_edit->SetCue(TL(search_banner));
	ChildWnd::Add(_edit,true);
	Layout();
}

SearchToolbarWnd::~SearchToolbarWnd() {
}

void SearchToolbarWnd::OnSearchChange(const std::wstring& q) {
}

void SearchToolbarWnd::Layout() { // also called by ToolbarWnd::OnSize
	Area search = GetSearchBoxArea();
	_edit->Fill(LayoutFill, search);

	ToolbarWnd::Layout();
}

Area SearchToolbarWnd::GetSearchBoxArea() const {
	Area rc = GetClientArea();
	Pixels margin = (rc.GetHeight()-_searchHeight)/2;
	Area search(rc.GetRight()-_searchWidth-margin-_rightMargin, rc.GetTop()+margin-1, _searchWidth, _searchHeight);
	return search;
}

void SearchToolbarWnd::SetSearchBoxRightMargin(Pixels r) {
	_rightMargin = r;
	Layout();
}

void SearchToolbarWnd::SetSearchBoxSize(Pixels w, Pixels h) {
	_searchWidth = w;
	_searchHeight = h;
	Layout();
}

void SearchToolbarWnd::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
	ToolbarWnd::Paint(g,theme);
	Area search = GetSearchBoxArea();
	search.Widen(1,1,1,1);

	SolidBrush border(theme->GetActiveStartColor());
	Pen borderPen(&border,1.0f);
	g.DrawRectangle(&borderPen, search);

	g.DrawImage(_searchIcon.GetBitmap(), Area(search.GetLeft()-20, search.GetTop(), _searchHeight, _searchHeight));
}

void SearchToolbarWnd::Notify(Wnd* src, Notification n) {
	if(src==_edit.GetPointer() && n==NotificationChanged) {
		OnSearchChange(_edit->GetText());
	}
}