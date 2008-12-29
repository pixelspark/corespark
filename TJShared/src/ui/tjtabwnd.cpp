#include "../../include/ui/tjui.h" 
#include <windowsx.h> 
using namespace tj::shared::graphics;
using namespace tj::shared;

TabWnd::TabWnd(ref<WindowManager> root, const std::wstring& id): ChildWnd(L"TabWnd"), 
	_closeIcon(Icons::GetIconPath(Icons::IconTabClose)),
	_addIcon(Icons::GetIconPath(Icons::IconTabAdd)),
	_offset(0) {

	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	SetStyleEx(WS_EX_CONTROLPARENT);
	_headerHeight = defaultHeaderHeight;
	_root = root;
	_detachAttachAllowed = true;
	_childStyle = false;
	_id = id;
	_in = false;
	_entryAnimation.SetLength(Time(250));
	_entryAnimation.SetReversed(true);
	_entryAnimation.Start();
	Layout();
}

TabWnd::~TabWnd() {
}

TabWnd::TabPane::TabPane(strong<Pane> pane): _pane(pane), _width(-1) {
}

TabWnd::TabPane::~TabPane() {
}

ref<Pane> TabWnd::TabPane::GetPane() {
	return _pane;
}

Pixels TabWnd::TabPane::GetWidth() const {
	if(_appearAnimation.IsAnimating()) {
		return Pixels(_width * _appearAnimation.GetFraction()) + ((_pane && _pane->HasIcon()) ? TabWnd::KIconWidth : 0);
	}

	if(_pane && _width>0) {
		return _width + (_pane->HasIcon() ? TabWnd::KIconWidth : 0);
	}

	return 0;
}

void TabWnd::TabPane::Layout(strong<Theme> theme) {
	if(_pane) {
		Area txt = theme->MeasureText(_pane->GetTitle(), theme->GetGUIFontBold());
		_width = Pixels(txt.GetWidth() *  theme->GetDPIScaleFactor());
	}
}

void TabWnd::TabPane::Close() {
	_pane = null;
	_appearAnimation.Start(Time(200), true);
}

bool TabWnd::TabPane::IsDestroyed() const {
	return !_pane && !_appearAnimation.IsAnimating();
}

void TabWnd::OnTimer(unsigned int id) {
	if(!_entryAnimation.IsAnimating() && id==1) {
		StopTimer(id);
	}

	if(!_tabAppearAnimation.IsAnimating() && id==2) {
		StopTimer(id);
	}
	Layout();
	Repaint();
}

void TabWnd::SetDetachAttachAllowed(bool allow) {
	_detachAttachAllowed = allow;
	Repaint();
}

void TabWnd::Add(ref<Wnd> child) {
	Throw(L"You cannot call Add on a TabWnd, use AddPane instead", ExceptionTypeSevere);
}

std::wstring TabWnd::GetTabTitle() const {
	if(_current) {
		return _current->GetTitle();
	}

	return L"";
}

ref<Icon> TabWnd::GetTabIcon() const {
	if(_current) {
		return _current->GetIcon();
	}
	return null;
}

void TabWnd::Rename(ref<Wnd> wnd, std::wstring name) {
	std::vector< TabPane >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		TabPane& tp = *it;
		ref<Pane> p = tp.GetPane();
		if(p && p->GetWindow() == wnd) {
			p->SetTitle(name);
			Update();
			return;
		}
		++it;
	}
}

void TabWnd::SetDraggingPane(ref<Pane> pane) {
	_dragging = pane;
	Update();
}

void TabWnd::Paint(Graphics& g, strong<Theme> theme) {
	g.SetSmoothingMode(SmoothingModeDefault);
	g.SetCompositingQuality(CompositingQualityDefault);
	Area rect = GetClientArea();

	Pixels totalTabsWidth = GetTotalTabWidth();
	bool showScroller = (totalTabsWidth > (rect.GetWidth()-2*KIconWidth));
	Pixels scrollerHeight = (showScroller && !_childStyle) ?KScrollerHeight:0;
	Pixels offset = showScroller ? _offset : 0;

	if(_headerHeight>0) {		
		// draw background
		if(_childStyle) {
			Area backrc = rect;
			backrc.SetHeight(_headerHeight);
			SolidBrush white(theme->GetColor(Theme::ColorBackground));
			LinearGradientBrush bbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 50), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 0));
			g.FillRectangle(&white, backrc);
			g.FillRectangle(&bbr, backrc);
		}
		else {
			HWND root = GetAncestor(GetWindow(), GA_ROOT);
			graphics::Brush* abr = theme->GetApplicationBackgroundBrush(root, GetWindow());
			if(abr!=0) {
				g.FillRectangle(abr, Rect(rect.GetLeft(), rect.GetTop(), rect.GetRight(), _current?_headerHeight:(rect.GetHeight())));
				Pen back(abr, 2.0f);
				g.DrawRectangle(&back, RectF(0.0f, float(_headerHeight), float(rect.GetWidth()-1), float(rect.GetHeight()-_headerHeight+1)));
			
				delete abr;
			}
		}

		ref<WindowManager> root = _root;
		if(root) {
			ref<TabWnd> dt =  root->GetDragTarget();
			if(dt == ref<TabWnd>(this)) {
				LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f,float(_headerHeight)), theme->GetColor(Theme::ColorHighlightStart), theme->GetColor(Theme::ColorHighlightEnd));
				g.FillRectangle(&br, Rect(rect.GetLeft()+1, rect.GetTop(), rect.GetRight(), _headerHeight));
			}
		}
		
		Pen border(theme->GetColor(Theme::ColorActiveEnd), 1.0f);
		g.DrawRectangle(&border, RectF(0, float(_headerHeight)-1, float(rect.GetWidth())-1, float(rect.GetHeight()-_headerHeight)));

		std::vector<TabPane>::iterator it = _panes.begin();
		SolidBrush textBrush = theme->GetColor(Theme::ColorText);
		SolidBrush inactiveTextBrush = Theme::ChangeAlpha(theme->GetColor(Theme::ColorText), int(127.0f + 127.0f * _entryAnimation.GetFraction()));
		Pixels left = -offset;
		int idx = 0;

		while(it!=_panes.end()) {
			TabPane& tp = *it;
			ref<Pane> pane = tp.GetPane();
			if(pane && pane->_detached) {
				++it;
				continue;
			}
			
			Pixels currentTabLeft = left;
			if(_dragging && _dragging==ref<Pane>(pane)) {
				POINT lp;
				GetCursorPos(&lp);
				if(ScreenToClient(GetWindow(), &lp)) {
					lp.x = long(lp.x / theme->GetDPIScaleFactor());
					lp.y = long(lp.y / theme->GetDPIScaleFactor());
					currentTabLeft = lp.x - tp.GetWidth()/2;
				}
			}

			// border
			if(_current && ref<Pane>(pane)==_current) {
				if(!_childStyle) {
					LinearGradientBrush lbr(PointF(0.0f, (float)scrollerHeight), PointF(0.0f, float(_headerHeight)), theme->GetColor(Theme::ColorActiveEnd), theme->GetColor(Theme::ColorActiveEnd));
					g.FillRectangle(&lbr, RectF(float(currentTabLeft), scrollerHeight+2.0f, float(tp.GetWidth()+2), float(_headerHeight-scrollerHeight)));
					SolidBrush backBrush(theme->GetColor(Theme::ColorTabButtonBackground));
					g.FillRectangle(&backBrush, RectF(float(currentTabLeft+1.0f), scrollerHeight+3.0f, float(tp.GetWidth()), float(_headerHeight-scrollerHeight)));
				}
				else {
					LinearGradientBrush lbr(PointF(0.0f, (float)scrollerHeight), PointF(0.0f, float(_headerHeight-2-scrollerHeight)), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 0), theme->GetColor(Theme::ColorActiveEnd));
					LinearGradientBrush bbr(PointF(0.0f, (float)scrollerHeight), PointF(0.0f, float(_headerHeight-2-scrollerHeight)), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 0), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 127));
					Pen pn(&lbr, 1.0f);
					
					g.DrawRectangle(&pn, RectF(float(currentTabLeft), (float)scrollerHeight, float(tp.GetWidth()+2), float(_headerHeight-3-scrollerHeight)));
					g.FillRectangle(&bbr, RectF(float(currentTabLeft+1.0f), (float)scrollerHeight, float(tp.GetWidth()), float(_headerHeight-3-scrollerHeight)));
				}
			}
			
			// gradient achter actieve tab of dragging tab (niet border)
			if((ref<Pane>(pane)==_dragging && _dragging) || (ref<Pane>(pane)==_current && _current)) {
				Color start = theme->GetColor(Theme::ColorTabButtonStart);
				Color end = theme->GetColor(Theme::ColorTabButtonEnd);

				if(!_childStyle) {
					LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), start, end);
					g.FillRectangle(&lbr, RectF(float(currentTabLeft), scrollerHeight+2.0f, float(tp.GetWidth()+2), float(_headerHeight-2-scrollerHeight)));
				
					LinearGradientBrush gbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)/2.0f), theme->GetColor(Theme::ColorGlassStart), theme->GetColor(Theme::ColorGlassEnd));
					g.FillRectangle(&gbr, RectF(float(currentTabLeft), scrollerHeight+2.0f, float(tp.GetWidth()+2), float(_headerHeight/2.0f-2-scrollerHeight)));
				}
			}

			bool isCurrent = _current && ref<Pane>(pane)==_current;
			if(pane) {
				std::wstring title = pane->GetTitle();
				if(pane->HasIcon()) {
					ref<Icon> icon = pane->GetIcon();
					if(icon) {
						icon->Paint(g, Area(currentTabLeft+4, scrollerHeight+4, KRealIconWidth, KRealIconWidth), isCurrent ? 1.0f : (0.5f+(_entryAnimation.GetFraction()/2.0f)));
					}
				}

				Area labelArea(currentTabLeft+1+(pane->HasIcon() ? KIconWidth : 0), scrollerHeight+4, tp.GetWidth()+2, _headerHeight-2-scrollerHeight);
				StringFormat sf;
				sf.SetFormatFlags(StringFormatFlagsLineLimit);
				sf.SetAlignment(StringAlignmentNear);
				g.DrawString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), labelArea, &sf, isCurrent ? &textBrush : &inactiveTextBrush);
			}

			left += tp.GetWidth() + 4;
			++it;
			idx++;
		}
 
		if(!_childStyle && _detachAttachAllowed && _addIcon!=0 && _closeIcon!=0) {
			Pixels buttonsLeft = rect.GetWidth()-2*_headerHeight;

			if(!(left<(rect.GetWidth()-2*_headerHeight))) {
				//LinearGradientBrush disabled(PointF(float(buttonsLeft), 0.0f), PointF(rect.GetWidth(), 0.0f), Color(0,255,255,255),theme->GetColor(Theme::ColorDisabledOverlay));
				HWND root = GetAncestor(GetWindow(), GA_ROOT);
				graphics::Brush* abr = theme->GetApplicationBackgroundBrush(root, GetWindow());

				g.FillRectangle(abr, RectF((float)buttonsLeft, 0.0f, float(rect.GetWidth()-buttonsLeft), float(_headerHeight)-1.0f));
			}

			_addIcon.Paint(g, Area(buttonsLeft, 0, _headerHeight-2, _headerHeight-2), 0.5f + (_entryAnimation.GetFraction()/2.0f));
			_closeIcon.Paint(g, Area(rect.GetWidth()-_headerHeight, 0, _headerHeight-2, _headerHeight-2), 0.5f + (_entryAnimation.GetFraction()/2.0f));
		}

		// Draw scroller
		if(showScroller) {
			Pixels w = rect.GetWidth() - 3*KIconWidth;
			Pixels scrollerWidth = Pixels((float(w)/float(totalTabsWidth))*w);
			Pixels scrollerOffset = Pixels((float(_offset)/float(totalTabsWidth))*w);
			LinearGradientBrush scrollerBrush(PointF(0.0f, (float)rect.GetTop()), PointF(0.0f, float(rect.GetTop()+scrollerHeight)), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
			g.FillRectangle(&scrollerBrush, Area(rect.GetLeft()+scrollerOffset, rect.GetTop()+1, scrollerWidth, KScrollerHeight-1));
		}
	}
}

void TabWnd::FixScrollerOffset() {
	Pixels totalWidth = GetTotalTabWidth();
	Area rc = GetClientArea();
	Pixels w = rc.GetWidth() - 3 * KIconWidth;

	if(_offset > (totalWidth - w)) {
		_offset = totalWidth - w;
	}
	else if(_offset < 0) {
		_offset = 0;
	}
}

void TabWnd::SetChildStyle(bool c) {
	_childStyle = c;
	Repaint();
}

ref<Wnd> TabWnd::GetCurrentPane() {
	return _current->GetWindow();
}

void TabWnd::Clear() {
	_panes.clear();
}

const std::wstring& TabWnd::GetID() const {
	return _id;
}

ref<Pane> TabWnd::AddPane(ref<Pane> pane, bool select) {
	if(pane && pane->GetWindow()) {
		SetParent(pane->GetWindow()->GetWindow(), GetWindow());
		pane->GetWindow()->Show(false);
		_panes.push_back(TabPane(pane));
		TabPane& tp = *(_panes.rbegin());

		if(IsShown()) {
			tp._appearAnimation.Start(Time(200));
			_tabAppearAnimation.Start(Time(210));
			StartTimer(Time(10), 2);
		}

		Placement np;
		np._type = Placement::Tab;
		np._container = GetID();
		pane->OnPlacementChange(np);
		Layout();

		if(_panes.size()==1) {
			SelectPane(0);
		}
		else if(select) {
			SelectPane(pane);
		}

		Repaint();
	}
	return pane;
}

ref<Pane> TabWnd::GetPane(int index) {
	try {
		return _panes.at(index)._pane;
	}
	catch(...) {
		return null;
	}
}

void TabWnd::RemovePane(ref<Wnd> wnd) {
	assert(wnd);

	wnd->Show(false);
	if(_current) {
		ref<Wnd> cw = _current->GetWindow();
		if(cw) {
			cw->Show(false);
		}
	}
	_offset = 0;

	bool removedWasCurrent = false;
	std::vector<TabPane>::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		TabPane& tp = *it;
		strong<Pane> pane = tp._pane;
		if(pane->GetWindow() == wnd) {
			if(_current==ref<Pane>(pane)) {
				removedWasCurrent = true;
			}
			_panes.erase(it);
			break;
		}
		++it;
	}

	if(removedWasCurrent) {
		_current = null;
		SelectPane(0);
	}
}

void TabWnd::SelectPane(unsigned int index) {
	try {
		TabPane& tp = _panes.at(index);
		if(!tp.IsDestroyed() && tp.GetPane()) {
			SelectPane(tp.GetPane());
		}
	}
	catch(...) {
	}
}

void TabWnd::SelectPane(ref<Wnd> wnd) {
	std::vector<TabPane>::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		TabPane& tp = *it;
		if(!tp.IsDestroyed()) {
			ref<Pane> pane = tp._pane;
			if(pane && pane->GetWindow()==wnd) {
				SelectPane(ref<Pane>(pane));
				return;
			}
		}
		++it;
	}
}

void TabWnd::SelectPane(ref<Pane> pane) {
	if(_current) {
		_current->GetWindow()->Show(false);
	}

	if(pane) {
		pane->GetWindow()->Show(true);
	}

	_current = pane;
	
	if(_current) {
		SetFocus(_current->GetWindow()->GetWindow());
	}
	Layout();
}

void TabWnd::Update() {
	Repaint();
}

void TabWnd::OnSize(const Area& ns) {
	Layout();
	Repaint();
}

void TabWnd::Layout() {
	Area rc = GetClientArea();
	strong<Theme> theme = ThemeManager::GetTheme();

	if(_current) {	
		_current->GetWindow()->Move(_childStyle?0:1, rc.GetTop()+_headerHeight, rc.GetWidth()-(_childStyle?0:1), rc.GetHeight()-_headerHeight-1);
	}

	std::vector<TabPane>::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		TabPane& tp = *it;
		
		if(tp.IsDestroyed()) {
			it = _panes.erase(it);
		}
		else {
			tp.Layout(theme);

			ref<Pane> pane = tp._pane;
			if(pane) {
				pane->GetWindow()->Move(_childStyle?0:1, rc.GetTop()+_headerHeight, rc.GetWidth()-(_childStyle?0:1), rc.GetHeight()-_headerHeight-(_childStyle?0:1));
			}
			++it;
		}
	}

	FixScrollerOffset();
	Update();
}

void TabWnd::OnFocus(bool focus) {
	if(!focus) {
		Repaint();
	}
}

LRESULT TabWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_MOUSEWHEEL) {
		int delta = GET_WHEEL_DELTA_WPARAM(wp);

		if(delta<0) {
			_offset += 5;
		}
		else {
			_offset -= 5;
		}
		FixScrollerOffset();
		Repaint();
	}
	return ChildWnd::Message(msg,wp,lp);
}

Pixels TabWnd::GetTotalTabWidth() {
	std::vector<TabPane>::iterator it = _panes.begin();
	Graphics g(GetWindow());
	Pixels size = 0;
	strong<Theme> theme = ThemeManager::GetTheme();

	while(it!=_panes.end()) {
		TabPane& tp = *it;
		ref<Pane> pane = tp._pane;

		if(pane && pane->_detached) {
			++it;
			continue;
		}			
		
		size += tp.GetWidth() + 4;
		++it;
	}

	return size;
}

void TabWnd::ClosePane(ref<Pane> current) {
	ref<WindowManager> root = _root;
	if(current) {
		std::vector<TabPane>::iterator it = _panes.begin();
		while(it!=_panes.end()) {
			TabPane& tp = *it;
			ref<Pane> pn = tp.GetPane();

			if(pn==current) {
				tp.Close();
				StartTimer(Time(10), 2);
				_tabAppearAnimation.Start(350, true);
				break;
			}
			++it;
		}

		ref<Wnd> wnd = current->GetWindow();
		if(wnd) wnd->Show(false);

		if(!current->IsClosable() && root) {
			root->AddOrphanPane(current);
		}

		if(_dragging==current) {
			_dragging = null;
		}

		if(_current==current) {
			_current = null;
			SelectPane(0);
		}
		Update();
	}
}

void TabWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown || ev==MouseEventLUp || ev==MouseEventMUp) {
		Layout();
		Area rect = GetClientArea();
		if(y<_headerHeight) {
			if(x>rect.GetWidth()-_headerHeight && y<_headerHeight) {
				if(_detachAttachAllowed) {
					// close button
					if(ev==MouseEventLUp) {
						ClosePane(_current);
						return;
					}
				}
			}
			else if((x>rect.GetWidth()-2*_headerHeight)) {
				if(_detachAttachAllowed) {
					if(ev==MouseEventLUp) {
						// add button
						DoAddMenu(x,y);
						return;
					}
				}
			}
			else {			
				// Select tab
				std::vector<TabPane>::iterator it = _panes.begin();
				Pixels totalWidth = GetTotalTabWidth();
				Area rc = GetClientArea();
				bool showScroller = totalWidth > rc.GetWidth();
				int left = showScroller ? -_offset : 0;
				Graphics g(GetWindow());

				strong<Theme> theme = ThemeManager::GetTheme();

				while(it!=_panes.end()) {
					TabPane& tp = *it;
					if(tp.IsDestroyed()) {
						++it;
						continue;
					}

					ref<Pane> pane = tp._pane;
					
					if(pane && pane->_detached) {
						++it;
						continue;
					}			
					left += tp.GetWidth() + 4;

					if(x<left) {
						if(ev==MouseEventLUp) {
							SelectPane(pane);
							_dragging = null;
						}
						else if(ev==MouseEventMUp) {
							ClosePane(pane);
						}
						else {
							if(_current==ref<Pane>(pane)) {
								SetCursor(theme->GetGrabbedCursor());
								SetDraggingPane(pane);
								_dragStartX = x;
								_dragStartY = y;
							}
						}
						break;
					}
					++it;
				}
			}

			if(_dragging) {
				if(ev==MouseEventLDown) {
					SetCapture(GetWindow());
				}
				else if(ev==MouseEventLUp) {
					_dragging = null;
					SetCursor(LoadCursor(0,IDC_ARROW));
					Repaint();
				}
			}
			else {
				ReleaseCapture();
			}
		}
	}
	else if(ev==MouseEventRDown) {
		_dragStartX = x;
		_dragStartY = y;
		SetCapture(GetWindow());
	}
	else if(ev==MouseEventRUp) {
		ReleaseCapture();
		_dragging = null;
	}
	else if(ev==MouseEventLeave) {
		_in = false;
		_dragging = null;
		_entryAnimation.SetReversed(true);
		_entryAnimation.Start();
		StartTimer(Time(50), 1);
	}
	else if(ev==MouseEventMove) {
		if(!_in) {
			_in = true;
			_entryAnimation.SetReversed(false);
			_entryAnimation.Start();
			SetWantMouseLeave(true);
			StartTimer(Time(50), 1);
		}

		if(IsKeyDown(KeyMouseRight)) {
			int dx = x-_dragStartX;

			_offset += dx;
			FixScrollerOffset();

			_dragStartX = x;
			_dragStartY = y;
			Repaint();
		}
		else if(_dragging && IsKeyDown(KeyMouseLeft)) {
			// we're dragging a tab
			int dx = abs(_dragStartX - x);
			int dy = abs(_dragStartY - y);
			
			if(dy < TearOffLimit && dx > TearOffLimit ) {
				DropPaneAt(x, _dragging);
			}
			else if(dy > TearOffLimit) {
				Detach(_dragging);
				_dragging = null;
			}
			Update();
		}
	}
}

void TabWnd::DropPaneAt(Pixels x, ref<Pane> dragging) {
	strong<Theme> theme = ThemeManager::GetTheme();

	// Find the dragging pane in the pane list
	for(std::vector<TabPane>::iterator dit = _panes.begin(); dit != _panes.end(); ++dit) {
		if(ref<Pane>((*dit)._pane) == dragging) {
			Pixels xo = 0;
			std::vector<TabPane>::iterator it = GetPaneIteratorAt(x,xo);
			if(it!=_panes.end()) {
				if(it<dit || (x-xo) > 10) {
					TabPane& tp = *it;
					(*dit)._pane = tp._pane;
					tp._pane = dragging;
					tp.Layout(theme);
					(*dit).Layout(theme);
					Update();
				}
			}
			return;		
		}
	}
}

void TabWnd::OnKey(Key k, wchar_t t, bool down, bool isAccelerator) {
	if(!isAccelerator) {
		if(k==KeyBrowseBack) {
			int cid = 0;
			std::vector<TabPane>::iterator it = _panes.begin();
			while(it!=_panes.end()) {
				if(ref<Pane>((*it)._pane)==_current) {
					break;
				}
				cid++;
				++it;
			}

			cid--;
			if(cid<0) {
				cid = 0;
			}

			SelectPane(cid);
			Update();
		}
		else if(k==KeyBrowseForward) {
			int cid = 0;
			std::vector<TabPane>::iterator it = _panes.begin();
			while(it!=_panes.end()) {
				if(ref<Pane>((*it)._pane)==_current) {
					break;
				}
				cid++;
				++it;
			}

			cid++;
			if(cid>int(_panes.size()-1)) {
				cid = int(_panes.size())-1;
			}

			SelectPane(cid);
			Update();
		}
	}
}

void TabWnd::Detach(ref<Pane> p) {
	if(!_detachAttachAllowed) return;

	std::vector<TabPane>::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		if(ref<Pane>((*it)._pane) == p) {
			_panes.erase(it);
			break;
		}
		++it;
	}

	if(p==_current) {
		_current = null;
	}
	ref<WindowManager> root = _root;
	if(root) {
		ref<FloatingPane> fp = root->AddFloatingPane(p);
		ReleaseCapture();
		SendMessage(fp->GetWindow(), WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}
	Update();
}

void TabWnd::Attach(ref<Pane> p) {
	if(!_detachAttachAllowed) return;

	AddPane(p);
	SetParent(p->GetWindow()->GetWindow(), GetWindow());
	Layout();
	Update();
}

Placement TabWnd::GetPlacement() const {
	Placement p;
	p._type = Placement::Tab;
	p._container = _id;

	return p;
}

ref<Pane> TabWnd::GetPaneAt(Pixels x) {
	Pixels xo = 0;
	std::vector<TabPane>::iterator it = GetPaneIteratorAt(x,xo);
	if(it!=_panes.end()) {
		TabPane& tp = *it;
		return tp._pane;
	}
	return null;
}

std::vector<TabWnd::TabPane>::iterator TabWnd::GetPaneIteratorAt(Pixels x, Pixels& xOnThisPane) {
	std::vector<TabPane>::iterator it = _panes.begin();
	unsigned int idx = 0; 
	Pixels left = 0;
	Graphics g(GetWindow());
	strong<Theme> theme = ThemeManager::GetTheme();

	while(it!=_panes.end()) {
		TabPane& tp = *it;
		
		ref<Pane> pane = tp._pane;
		if(pane && pane->_detached) {
			++it;
			idx++;
			continue;
		}
		Pixels paneWidth = tp.GetWidth() + 4;
		
		if(x<(left+paneWidth)) {
			xOnThisPane = x-(left+paneWidth);
			return it;
		}
		left += paneWidth;
		idx++;
		++it;
	}

	return _panes.end();
}

bool TabWnd::RevealWindow(ref<Wnd> w) {
	std::vector<TabPane>::iterator it = _panes.begin();
	int idx = 0;
	while(it!=_panes.end()) {
		TabPane& tp = *it;
		if(!tp.IsDestroyed()) {
			ref<Pane> p = tp._pane;
			if(p && p->GetWindow() == w) {
				SelectPane(idx);
				return true;
			}
		}
		idx++;
		++it;
	}

	return false;
}

void TabWnd::DoAddMenu(Pixels x, Pixels y) {
	ref<WindowManager> root = _root;
	if(!root) {
		return;
	}

	ContextMenu m;
	Area rc = GetClientArea();

	std::vector< ref<Pane> >* pv = root->GetOrphanPanes();
	if(pv->size()>0) {
		std::vector< ref<Pane> >::iterator it = pv->begin();
		int n = 1;
		while(it!=pv->end()) {
			ref<Pane> pane = *it;
			if(pane) {
				strong<MenuItem> ci = GC::Hold(new MenuItem(pane->GetTitle(), n, false, MenuItem::NotChecked, pane->GetIcon()));
				m.AddItem(ci);
				n++;
			}
			++it;
		}

		int c = m.DoContextMenu(ref<Wnd>(this), rc.GetRight()-2*_headerHeight, rc.GetTop()+_headerHeight);
		if(c>0) {
			try {
				if(_current) {
					_current->GetWindow()->Show(false);
				}

				ref<Pane> selected = pv->at(c-1);

				if(selected) {
					root->RemoveOrphanPane(selected);
					Attach(selected);
					_current = selected;
					Layout();
					_current->GetWindow()->Show(true);
				}
			}
			catch(...) {
			}
		}
	}
	else {
		m.AddItem(TL(tab_no_orphans), -1, false, false);
		m.DoContextMenu(ref<Wnd>(this), rc.GetRight()-2*_headerHeight, rc.GetTop()+_headerHeight);
	}
}
