#include "../../include/ui/tjui.h" 
#include <iomanip>
#include <windowsx.h>
using namespace tj::shared::graphics;
using namespace tj::shared;

// ToolbarWnd
ToolbarWnd::ToolbarWnd(): ChildWnd(L""), _tipIcon(Icons::GetIconPath(Icons::IconTip)), _in(false), _bk(false) {
	UnsetStyle(WS_TABSTOP);
	SetWantMouseLeave(true);
	_entryAnimation.SetLength(Time(250));
	_entryAnimation.SetEase(Animation::EaseQuadratic);
}

ToolbarWnd::~ToolbarWnd() {
}

void ToolbarWnd::Add(ref<ToolbarItem> item, bool alignRight) {
	if(alignRight) {
		_itemsRight.push_back(item);
	}
	else {
		_items.push_back(item);
	}
	item->Show(true);
	Layout();
}

void ToolbarWnd::Layout() {
	ref<Theme> theme = ThemeManager::GetTheme();
	Pixels bs = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	Area rc = GetClientArea();

	// Layout items on the left
	std::vector< ref<ToolbarItem> >::iterator it = _items.begin();
	std::vector< ref<ToolbarItem> >::iterator end = _items.end();
	Pixels x = 0;

	while(it!=end) {
		ref<ToolbarItem> ti = *it;
		if(ti && ti->IsShown()) {
			Area prefSize = ti->GetPreferredSize();
			ti->Move(x, 0, prefSize.GetWidth(), prefSize.GetHeight());
			x += prefSize.GetWidth();		
		}
		++it;
	}

	// Layout items on the right
	std::vector< ref<ToolbarItem> >::iterator rit = _itemsRight.begin();
	std::vector< ref<ToolbarItem> >::iterator rend = _itemsRight.end();
	Pixels xr = rc.GetRight();
	if(HasTip()) xr -= bs;

	while(rit!=rend) {
		ref<ToolbarItem> ti = *rit;
		if(ti && ti->IsShown()) {
			Area prefSize = ti->GetPreferredSize();

			if((xr-prefSize.GetWidth())<=x) {
				ti->Move(-1000,0,0,0);
			}
			else {
				ti->Move(xr-prefSize.GetWidth(), 0, prefSize.GetWidth(), prefSize.GetHeight());
				xr -= prefSize.GetWidth();	
			}
		}
		++rit;
	}

	_freeArea = Area(x+bs, 0, xr-x-bs, rc.GetHeight());
}

void ToolbarWnd::SetBackground(bool t) {
	_bk = t;
	Repaint();
}

void ToolbarWnd::SetBackgroundColor(graphics::Color c) {
	_bkColor = c;
	Repaint();
}

void ToolbarWnd::OnSize(const Area& ns) {
	Layout();
}

bool ToolbarWnd::HasTip() const {
	return _tip!=0;
}

void ToolbarWnd::SetTip(ref<Wnd> tipWindow) {
	_tip = tipWindow;
	Layout();
	Update();
}

void ToolbarWnd::Fill(LayoutFlags f, Area& r, bool direct) {
	ref<Theme> theme = ThemeManager::GetTheme();
	Pixels h = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	if(f==LayoutTop) {
		if(direct) Move(r.GetLeft(), r.GetTop(), r.GetWidth(), h);
		r.Narrow(0,h,0,0);
	}
	else if(f==LayoutBottom) {
		if(direct) Move(r.GetLeft(), (r.GetTop()+r.GetHeight())-h, r.GetWidth(), h);
		r.Narrow(0,0,0,h);
	}
	else {
		ChildWnd::Fill(f, r, direct);
	}
}

Pixels ToolbarWnd::GetButtonX(int command) {
	std::vector< ref<ToolbarItem> >::const_iterator it = _items.begin();
	std::vector< ref<ToolbarItem> >::const_iterator end = _items.end();

	while(it!=end) {
		ref<ToolbarItem> ti = *it;
		if(ti && ti->IsShown() && ti->GetCommand()==command) {
			return ti->GetClientArea().GetX();
		}
		++it;
	}

	std::vector< ref<ToolbarItem> >::const_iterator rit = _itemsRight.begin();
	std::vector< ref<ToolbarItem> >::const_iterator rend = _itemsRight.end();

	while(rit!=rend) {
		ref<ToolbarItem> ti = *rit;
		if(ti && ti->IsShown() && ti->GetCommand()==command) {
			return ti->GetClientArea().GetX();
		}
		++rit;
	}

	return 0;
}

void ToolbarWnd::OnTimer(unsigned int id) {
	if(!_entryAnimation.IsAnimating()) {
		StopTimer(id);
	}
	Repaint();
}

void ToolbarWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	ref<Theme> theme = ThemeManager::GetTheme();
	Pixels bs = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);

	if(ev==MouseEventMove||ev==MouseEventLDown) {
		if(!_in) {
			StartTimer(Time(50), 1);
			_entryAnimation.SetReversed(false);
			_entryAnimation.Start();
		}
		_in = true;

		if(!IsKeyDown(KeyMouseLeft)||ev==MouseEventLDown) {
			_over = Elements::GetElementAt(_items, x, y);
			if(!_over) {
				_over = Elements::GetElementAt(_itemsRight, x, y);
			}
		}

		// track leave event
		Repaint();
	}
	else if(ev==MouseEventLeave) {
		_entryAnimation.SetReversed(true);
		_entryAnimation.Start();
		StartTimer(Time(50),1);
		_in = false;
		Repaint();
	}
	else if(ev==MouseEventLUp) {
		Area rc = GetClientArea();

		if(_over && _over->IsEnabled()) {
			OnCommand(_over);
		}
		else if(x>rc.GetRight()-bs) {
			// Tip thing
			if(HasTip()) {
				HWND twnd = _tip->GetWindow();
				POINT pt;
				pt.x = int((rc.GetRight()-bs)*theme->GetDPIScaleFactor());
				pt.y = int(rc.GetHeight()*theme->GetDPIScaleFactor());
				ClientToScreen(GetWindow(), &pt);
				SetWindowPos(twnd, 0, pt.x, pt.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
				_tip->Show(true);
				UpdateWindow(GetWindow());
			}
		}
		Repaint();
	}
}

Area ToolbarWnd::GetFreeArea() const {
	return _freeArea;
}

void ToolbarWnd::Paint(graphics::Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();
	Pixels buttonSize = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	
	if(_bk) {
		LinearGradientBrush lbl(PointF(0.0f, 0.0f), PointF(float(rc.GetWidth()/2)+2.0f,0.0f), theme->GetColor(Theme::ColorBackground), _bkColor);
		g.FillRectangle(&lbl, RectF(float(rc.GetLeft()), float(rc.GetTop()), float(rc.GetWidth()/2.0f)+2.0f, float(rc.GetHeight())));

		LinearGradientBrush lbr(PointF(float(rc.GetWidth()/2)-1.0f, 0.0f), PointF(float(rc.GetWidth())+1.0f, 0.0f), _bkColor, theme->GetColor(Theme::ColorBackground));
		g.FillRectangle(&lbr, RectF(float(rc.GetLeft()+rc.GetWidth()/2.0f)+1.0f, float(rc.GetTop()), float(rc.GetWidth()/2.0f), float(rc.GetHeight())));
	}
	else {
		SolidBrush zwart(theme->GetColor(Theme::ColorBackground));
		g.FillRectangle(&zwart, rc);

		LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetColor(Theme::ColorToolbarStart), theme->GetColor(Theme::ColorToolbarEnd));
		SolidBrush dbr(theme->GetColor(Theme::ColorDisabledOverlay));
		g.FillRectangle(&br, rc);
		g.FillRectangle(&dbr, rc);
	}

	LinearGradientBrush glas(PointF(0.0f,0.0f), PointF(0.0f,float(rc.GetHeight())/2.0f), theme->GetColor(Theme::ColorGlassStart), theme->GetColor(Theme::ColorGlassEnd));
	g.FillRectangle(&glas, RectF(0.0f, 0.0f, float(rc.GetWidth()), float(rc.GetHeight())/2.0f));

	Pen pn(theme->GetColor(Theme::ColorActiveEnd), 1.0f);
	g.DrawLine(&pn, PointF(0.0f, float(rc.GetHeight()-1.0f)), PointF(float(rc.GetWidth()), float(rc.GetHeight()-1.0f)));

	// Draw toolbar buttons (left)
	std::vector< ref<ToolbarItem> >::iterator it = _items.begin();
	std::vector< ref<ToolbarItem> >::iterator end = _items.end();

	while(it!=end) {
		ref<ToolbarItem> item = *it;
		if(item && item->IsShown()) {
			bool over = _in && (_over==item);
			item->Paint(g, theme, over, IsKeyDown(KeyMouseLeft), _entryAnimation.GetFraction());
		}
		++it;
	}

	// Draw toolbar buttons (right)
	std::vector< ref<ToolbarItem> >::iterator rit = _itemsRight.begin();
	std::vector< ref<ToolbarItem> >::iterator rend = _itemsRight.end();

	while(rit!=rend) {
		ref<ToolbarItem> item = *rit;
		if(item && item->IsShown()) {
			bool over = (_in || _entryAnimation.IsAnimating()) && (_over==item);
			item->Paint(g, theme, over, IsKeyDown(KeyMouseLeft), _entryAnimation.GetFraction());
		}
		++rit;
	}

	// Draw tip window
	// TODO: make this an element too and determine if it's shown in Layout()
	if(HasTip()) {
		Area tiprc = rc;
		rc.SetX(rc.GetRight()-buttonSize);
		ToolbarItem::DrawToolbarButton(g, _tipIcon, rc, theme, _tip->IsShown(), _tip->IsShown(), false, true);
	}

	// draw description text if in & selected
	if(CanShowHints() && _over) {
		Pixels lx = int(_items.size())*buttonSize;
		std::wstring text = _over->GetText();
		SolidBrush br(Theme::ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), _entryAnimation.GetFraction()));
		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);
		sf.SetLineAlignment(StringAlignmentCenter);
		g.DrawString(text.c_str(), (int)text.length(), theme->GetGUIFont(), RectF(float(lx), 0.0f, float(rc.GetWidth()), 24.0f), &sf, &br);
	}
}

void ToolbarItem::DrawToolbarButton(graphics::Graphics& g, const Area& rc, ref<Theme> theme, bool over, bool down, bool separator, float alpha) {
	Pixels buttonSize = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	Pixels x = rc.GetX();

	if(over) {
		Area wrapped = rc;
		wrapped.Narrow(1,1,2,2);
		if(down) {
			LinearGradientBrush active(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetColor(Theme::ColorHighlightStart), theme->GetColor(Theme::ColorHighlightEnd));
			g.FillRectangle(&active, wrapped);
		}
		else {
			LinearGradientBrush active(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), Theme::ChangeAlpha(theme->GetColor(Theme::ColorActiveStart),int(alpha*255.0f)), Theme::ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), int(alpha*255.0f)));
			g.FillRectangle(&active, wrapped);
		}

		theme->DrawHighlightEllipse(g, rc, down ? 1.0f : 0.5f);

		LinearGradientBrush glas(PointF(0.0f,0.0f), PointF(0.0f,float(rc.GetHeight())/2.0f), theme->GetColor(Theme::ColorGlassStart), theme->GetColor(Theme::ColorGlassEnd));
		g.FillRectangle(&glas, wrapped);
	}

	if(separator) {
		Pen pn(theme->GetColor(Theme::ColorActiveStart));
		g.DrawLine(&pn, PointF((float)rc.GetRight()-1.0f, 4.0f), PointF((float)rc.GetRight()-1.0f, float(rc.GetHeight())-4.0f));
	}
}

void ToolbarItem::DrawToolbarButton(Graphics& g, Icon& icon, const Area& rc, ref<Theme> theme, bool over, bool down, bool separator, bool enabled, float alpha) {
	Pixels buttonSize = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	Pixels x = rc.GetX();

	if(over) {
		Area wrapped = rc;
		wrapped.Narrow(1,1,2,2);
		if(down) {
			LinearGradientBrush active(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetColor(Theme::ColorHighlightStart), theme->GetColor(Theme::ColorHighlightEnd));
			g.FillRectangle(&active, wrapped);
		}
		else {
			LinearGradientBrush active(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), Theme::ChangeAlpha(theme->GetColor(Theme::ColorActiveStart),int(alpha*255.0f)), Theme::ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), int(alpha*255.0f)));
			g.FillRectangle(&active, wrapped);
		}

		theme->DrawHighlightEllipse(g, rc, down ? 1.0f : 0.5f);

		LinearGradientBrush glas(PointF(0.0f,0.0f), PointF(0.0f,float(rc.GetHeight())/2.0f), theme->GetColor(Theme::ColorGlassStart), theme->GetColor(Theme::ColorGlassEnd));
		g.FillRectangle(&glas, wrapped);
	}

	icon.Paint(g, Area(x+3, 4, 16, 16), enabled);

	if(separator) {
		Pen pn(theme->GetColor(Theme::ColorActiveStart));
		g.DrawLine(&pn, PointF((float)rc.GetRight()-1.0f, 4.0f), PointF((float)rc.GetRight()-1.0f, float(rc.GetHeight())-4.0f));
	}
}

bool ToolbarWnd::CanShowHints() {
	return GetFreeArea().GetWidth() > 100;
}

// ToolbarItem
ToolbarItem::ToolbarItem(int command, graphics::Bitmap* bmp, std::wstring text, bool separator): _icon(bmp) , _enabled(true), _active(true) {
	_separator = separator;
	_command = command;
	_text = text;
	_preferredWidth = -1;
	_preferredHeight = -1;
}

ToolbarItem::ToolbarItem(int command, std::wstring rid, std::wstring text, bool separator): _icon(rid), _enabled(true), _active(true) {
	_separator = separator;
	std::wstring path = ResourceManager::Instance()->Get(rid);
	_command = command;
	_text = text;
	_preferredWidth = -1;
	_preferredHeight = -1;
}

ToolbarItem::~ToolbarItem() {
};

void ToolbarItem::SetActive(bool a) {
	_active = a;
}

bool ToolbarItem::IsActive() const {
	return _active && _enabled;
}

void ToolbarItem::SetEnabled(bool e) {
	_enabled = e;
}

bool ToolbarItem::IsEnabled() const {
	return _enabled;
}

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

void ToolbarItem::Paint(Gdiplus::Graphics &g, tj::shared::ref<Theme> theme) {
	Paint(g,theme,false, false);
}

void ToolbarItem::Paint(Gdiplus::Graphics &g, tj::shared::ref<Theme> theme, bool over, bool down, float alpha) {
	Area rc = GetClientArea();
	bool active = IsActive();
	bool enabled = IsEnabled();

	DrawToolbarButton(g, GetIcon(), rc, theme, enabled && over, enabled && down, _separator, active, alpha);
}

Area ToolbarItem::GetPreferredSize() const {
	if(_preferredHeight>=0 && _preferredWidth>=0) {
		return Area(0,0,_preferredWidth, _preferredHeight);
	}
	else {
		ref<Theme> theme = ThemeManager::GetTheme();
		Pixels bs = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
		return Area(0,0,IsSeparator() ? Pixels(bs+1) : bs,bs);
	}
}

void ToolbarItem::SetPreferredSize(Pixels w, Pixels h) {
	_preferredWidth = w;
	_preferredHeight = h;
}

// StateToolbarItem
StateToolbarItem::StateToolbarItem(int c, const std::wstring& icon, const std::wstring& text): ToolbarItem(c, icon, text), _on(false) {
}

StateToolbarItem::~StateToolbarItem() {
}

void StateToolbarItem::SetState(bool on) {
	_on = on;
}

bool StateToolbarItem::IsOn() const {
	return _on;
}

void StateToolbarItem::Paint(graphics::Graphics& g, ref<Theme> theme, bool over, bool down, float backgroundAlpha) {
	if(_on) {
		SolidBrush green(theme->GetColor(Theme::ColorProgress));
		Area rc = GetClientArea();
		rc.Narrow(1,2, IsSeparator() ? 4 : 3, 3);
		g.FillRectangle(&green, rc);

		LinearGradientBrush lbr(PointF(0.0f, float(rc.GetTop()-1)), PointF(0.0f, float(rc.GetBottom()+1)), Color::Transparent, theme->GetColor(Theme::ColorDisabledOverlay));
		g.FillRectangle(&lbr, rc);
	}

	ToolbarItem::Paint(g,theme,over,down,backgroundAlpha);
}

/* SearchToolbarWnd */
SearchToolbarWnd::SearchToolbarWnd(): _searchIcon(Icons::GetIconPath(Icons::IconSearch)), _rightMargin(0), _searchWidth(KDefaultBoxWidth), _searchHeight(KDefaultBoxHeight) {
	_edit = GC::Hold(new EditWnd());
	_edit->SetCue(TL(search_banner));
	ChildWnd::Add(_edit,true);
	Layout();
}

void SearchToolbarWnd::OnCreated() {
	_edit->EventTextChanged.AddListener(ref<Listener<EditWnd::NotificationTextChanged> >(this));
}

SearchToolbarWnd::~SearchToolbarWnd() {
}

void SearchToolbarWnd::OnSearchChange(const std::wstring& q) {
}

void SearchToolbarWnd::Layout() { // also called by ToolbarWnd::OnSize
	ToolbarWnd::Layout();

	Area search = GetSearchBoxArea();

	if(IsSearchBoxVisible()) {
		_edit->Fill(LayoutFill, search);
		_edit->Show(true);
	}
	else {
		// Hide search things, because we overlap with toolbar buttons
		_edit->Show(false);
	}
}

bool SearchToolbarWnd::IsSearchBoxVisible() const {
	ref<Theme> theme = ThemeManager::GetTheme();
	Pixels buttonSize = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);

	return GetSearchBoxArea().GetWidth()>=(2*buttonSize);
}

Area SearchToolbarWnd::GetSearchBoxArea() const {
	Area rc = GetFreeArea();
	rc.Narrow(0,0,_rightMargin,0);
	Pixels margin = (rc.GetHeight()-_searchHeight)/2;
	Pixels sw = min(rc.GetWidth()-margin,_searchWidth);
	Area search(rc.GetRight()-sw-margin, rc.GetTop()+margin-1, sw, _searchHeight);
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

void SearchToolbarWnd::Paint(graphics::Graphics& g, ref<Theme> theme) {
	ToolbarWnd::Paint(g,theme);
	Area search = GetSearchBoxArea();

	// Only paint search box if we do not overlap with the toolbar buttons
	if(IsSearchBoxVisible()) {
		search.Widen(1,1,1,1);

		SolidBrush border(theme->GetColor(Theme::ColorActiveStart));
		Pen borderPen(&border,1.0f);
		g.DrawRectangle(&borderPen, search);

		g.DrawImage(_searchIcon.GetBitmap(), Area(search.GetLeft()-20, search.GetTop(), _searchHeight, _searchHeight));
	}
}

void SearchToolbarWnd::Notify(ref<Object> src, const EditWnd::NotificationTextChanged& data) {
	if(src==ref<Object>(_edit)) {
		OnSearchChange(_edit->GetText());
	}
}

bool SearchToolbarWnd::CanShowHints() {
	Area rc = GetClientArea();
	return (GetFreeArea().GetWidth() - GetSearchBoxArea().GetWidth()) > 100;
}