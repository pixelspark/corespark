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

void TabWnd::OnTimer(unsigned int id) {
	if(!_entryAnimation.IsAnimating()) {
		StopTimer(id);
	}
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
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		ref<Pane> p = *it;
		if(p->GetWindow() ==wnd) {
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

void TabWnd::Paint(Graphics& g, ref<Theme> theme) {
	g.SetSmoothingMode(SmoothingModeDefault);
	g.SetCompositingQuality(CompositingQualityDefault);
	Area rect = GetClientArea();

	Pixels totalTabsWidth = GetTotalTabWidth();
	bool showScroller = (totalTabsWidth > (rect.GetWidth()-2*KIconWidth));
	Pixels scrollerHeight = showScroller?KScrollerHeight:0;
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
			if(dt && dt.GetPointer()==this) {
				LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f,float(_headerHeight)), theme->GetColor(Theme::ColorHighlightStart), theme->GetColor(Theme::ColorHighlightEnd));
				g.FillRectangle(&br, Rect(rect.GetLeft()+1, rect.GetTop(), rect.GetRight(), _headerHeight));
			}
		}
		
		Pen border(theme->GetColor(Theme::ColorActiveEnd), 1.0f);
		g.DrawRectangle(&border, RectF(0, float(_headerHeight)-1, float(rect.GetWidth())-1, float(rect.GetHeight()-_headerHeight)));

		std::vector< ref<Pane> >::iterator it = _panes.begin();
		SolidBrush textBrush = theme->GetColor(Theme::ColorText);
		SolidBrush inactiveTextBrush = Theme::ChangeAlpha(theme->GetColor(Theme::ColorText), int(127.0f + 127.0f * _entryAnimation.GetFraction()));
		int left = -offset;
		int idx = 0;

		while(it!=_panes.end()) {
			ref<Pane> pane = *it;
			if(pane->_detached) {
				++it;
				continue;
			}

			RectF bound;
			std::wstring title = pane->GetTitle();
			g.MeasureString(title.c_str(), (INT)title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
			
			// border
			if(pane==_current) {
				if(!_childStyle) {
					LinearGradientBrush lbr(PointF(0.0f, (float)scrollerHeight), PointF(0.0f, float(_headerHeight)), theme->GetColor(Theme::ColorActiveEnd), theme->GetColor(Theme::ColorActiveEnd));
					g.FillRectangle(&lbr, RectF(float(left), scrollerHeight+2.0f, float(bound.Width+2+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight-scrollerHeight)));
					SolidBrush backBrush(theme->GetColor(Theme::ColorTabButtonBackground));
					g.FillRectangle(&backBrush, RectF(float(left+1.0f), scrollerHeight+3.0f, float(bound.Width+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight-scrollerHeight)));
				}
				else {
					LinearGradientBrush lbr(PointF(0.0f, (float)scrollerHeight), PointF(0.0f, float(_headerHeight-2-scrollerHeight)), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 0), theme->GetColor(Theme::ColorActiveEnd));
					LinearGradientBrush bbr(PointF(0.0f, (float)scrollerHeight), PointF(0.0f, float(_headerHeight-2-scrollerHeight)), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 0), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 127));
					Pen pn(&lbr, 1.0f);
					
					g.DrawRectangle(&pn, RectF(float(left), (float)scrollerHeight, float(bound.Width+2+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight-3-scrollerHeight)));
					g.FillRectangle(&bbr, RectF(float(left+1.0f), (float)scrollerHeight, float(bound.Width+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight-3-scrollerHeight)));
				}
			}
			
			// gradient achter actieve tab of dragging tab (niet border)
			if((pane==_current && pane!=_dragging) || pane==_dragging) {
				Color start = theme->GetColor(Theme::ColorTabButtonStart);
				Color end = theme->GetColor(Theme::ColorTabButtonEnd);

				if(!_childStyle) {
					LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), start, end);
					g.FillRectangle(&lbr, RectF(float(left), scrollerHeight+2.0f, float(bound.Width+2+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight-2-scrollerHeight)));
				
					LinearGradientBrush gbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)/2.0f), theme->GetColor(Theme::ColorGlassStart), theme->GetColor(Theme::ColorGlassEnd));
					g.FillRectangle(&gbr, RectF(float(left), scrollerHeight+2.0f, float(bound.Width+2+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight/2.0f-2-scrollerHeight)));
				}
			}

			if(pane->HasIcon()) {
				ref<Icon> icon = pane->GetIcon();
				if(icon) {
					icon->Paint(g, Area(left+4, scrollerHeight+4, KRealIconWidth, KRealIconWidth), (pane==_current) ? 1.0f : (0.5f+(_entryAnimation.GetFraction()/2.0f)));
				}
			}
			g.DrawString(title.c_str(), (INT)title.length(), theme->GetGUIFontBold(), PointF(float(left+1+(pane->HasIcon()?KIconWidth:0)), scrollerHeight+4.0f), (pane==_current) ? &textBrush : &inactiveTextBrush);

			left += int(bound.Width) + 4 + (pane->HasIcon()?KIconWidth:0);
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
	assert(pane && pane->GetWindow());
	SetParent(pane->GetWindow()->GetWindow(), GetWindow());
	pane->GetWindow()->Show(false);
	_panes.push_back(pane);

	Placement np;
	np._type = Placement::Tab;
	np._container = GetID();
	pane->OnPlacementChange(np);

	if(_panes.size()==1) {
		SelectPane(0);
	}
	else if(select) {
		SelectPane(pane);
	}

	Repaint();
	return pane;
}

ref<Pane> TabWnd::GetPane(int index) {
	try {
		return _panes.at(index);
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
	_current = null;
	_offset = null;
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		ref<Pane> pane = *it;
		if(pane->GetWindow() == wnd) {
			_panes.erase(it);
			break;
		}
		++it;
	}
	SelectPane(0);
}

void TabWnd::SelectPane(unsigned int index) {
	try {
		ref<Pane> pane = _panes.at(index);
		SelectPane(pane);
	}
	catch(...) {
	}
}

void TabWnd::SelectPane(ref<Wnd> wnd) {
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		ref<Pane> pane = *it;
		if(pane->GetWindow()==wnd) {
			SelectPane(pane);
			return;
		}
		++it;
	}
}

void TabWnd::SelectPane(ref<Pane> pane) {
	if(_current) {
		_current->GetWindow()->Show(false);
	}
	pane->GetWindow()->Show(true);
	_current = pane;
	SetFocus(_current->GetWindow()->GetWindow());

	Layout();
}

void TabWnd::Update() {
	Repaint();
}

void TabWnd::Layout() {
	Area rc = GetClientArea();

	if(_current) {	
		_current->GetWindow()->Move(_childStyle?0:1, rc.GetTop()+_headerHeight, rc.GetWidth()-(_childStyle?0:1), rc.GetHeight()-_headerHeight-1);
		
		std::vector< ref<Pane> >::iterator it = _panes.begin();
		while(it!=_panes.end()) {
			ref<Pane> pane = *it;
			pane->GetWindow()->Move(_childStyle?0:1, rc.GetTop()+_headerHeight, rc.GetWidth()-(_childStyle?0:1), rc.GetHeight()-_headerHeight-(_childStyle?0:1));
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
	if(msg==WM_SIZE) {
		Layout();
	}
	else if(msg==WM_MOUSEWHEEL) {
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
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	Graphics g(GetWindow());
	Pixels size = 0;
	ref<Theme> theme = ThemeManager::GetTheme();

	while(it!=_panes.end()) {
		ref<Pane> pane = *it;
		if(pane->_detached) {
			++it;
			continue;
		}

		RectF bound;
		std::wstring title = pane->GetTitle();
		g.MeasureString(title.c_str(), (INT)title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
		size += int(bound.Width) + 4 + (pane->HasIcon()?KIconWidth:0);
		++it;
	}

	return Pixels(size * theme->GetDPIScaleFactor());
}

void TabWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown || ev==MouseEventLUp) {
		Area rect = GetClientArea();
		if(y<_headerHeight) {
			if(x>rect.GetWidth()-_headerHeight && y<_headerHeight) {
				if(_detachAttachAllowed) {
					// close button
					if(ev==MouseEventLUp) {
						ref<WindowManager> root = _root;
						if(_current && root) {
							std::vector< ref<Pane> >::iterator it = _panes.begin();
							while(it!=_panes.end()) {
								ref<Pane> pn = *it;
								if(pn==_current) {
									_panes.erase(it);
									break;
								}
								++it;
							}

							ref<Wnd> wnd = _current->GetWindow();
							if(wnd) wnd->Show(false);

							if(!_current->IsClosable()) {
								root->AddOrphanPane(_current);
							}
							_current = 0;
							SelectPane(0);
							Update();
						}
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
				std::vector< ref<Pane> >::iterator it = _panes.begin();
				unsigned int idx = 0; 
				Pixels totalWidth = GetTotalTabWidth();
				Area rc = GetClientArea();
				bool showScroller = totalWidth > rc.GetWidth();
				int left = showScroller ? -_offset : 0;
				Graphics g(GetWindow());

				ref<Theme> theme = ThemeManager::GetTheme();

				while(it!=_panes.end()) {
					ref<Pane> pane = *it;
					if(pane->_detached) {
						++it;
						idx++;
						continue;
					}
					RectF bound;
					std::wstring title = pane->GetTitle();
					g.MeasureString(title.c_str(), (INT)title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
					left += int(bound.Width) + 4 + (pane->HasIcon()?KIconWidth:0);
					if(x<left) {
						if(ev==MouseEventLUp) {
							SelectPane(idx);
							_dragging = null;
						}
						else {
							SetDraggingPane(pane);
							_dragStartX = x;
							_dragStartY = y;
						}
						break;
					}
					idx++;
					++it;
				}
			}

			if(_dragging) {
				if(ev==MouseEventLDown) {
					SetCapture(GetWindow());
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
	}
	else if(ev==MouseEventLeave) {
		_in = false;
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
				// only move tabs
				try {
					std::vector< ref<Pane> >::iterator it = std::find(_panes.begin(), _panes.end(), _dragging);
					if(it!=_panes.end() && it!=_panes.begin()) {
						dx = x - _dragStartX;
						if(dx<0) {
							std::vector< ref<Pane> >::iterator leftNeighbour = it-1;
							if(leftNeighbour!=_panes.end()) {
								ref<Pane> left = *leftNeighbour;
								*leftNeighbour = *it;
								*it = left;
							}
						}
						else {
							std::vector< ref<Pane> >::iterator rightNeighbour = it+1;
							if(rightNeighbour!=_panes.end()) {
								ref<Pane> right = *rightNeighbour;
								*rightNeighbour = *it;
								*it = right;
							}
						}
					}
				}
				catch(...) {
					_dragging = null;
				}

				_dragStartX = x;
				_dragStartY = y;
			}
			else if(dy > TearOffLimit) {
				Detach(_dragging);
				_dragging = null;
			}
			Update();
		}
	}
}

void TabWnd::OnKey(Key k, wchar_t t, bool down, bool isAccelerator) {
	if(!isAccelerator) {
		if(k==KeyBrowseBack) {
			int cid = 0;
			std::vector< ref<Pane> >::iterator it = _panes.begin();
			while(it!=_panes.end()) {
				if(*it==_current) {
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
			std::vector< ref<Pane> >::iterator it = _panes.begin();
			while(it!=_panes.end()) {
				if(*it==_current) {
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

	std::vector< ref<Pane> >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		if(*it == p) {
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
	Update();
}

Placement TabWnd::GetPlacement() const {
	Placement p;
	p._type = Placement::Tab;
	p._container = _id;

	return p;
}

ref<Pane> TabWnd::GetPaneAt(int x) {
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	unsigned int idx = 0; 
	int left = 0;
	Graphics g(GetWindow());
	ref<Theme> theme = ThemeManager::GetTheme();

	RECT rc;
	GetWindowRect(GetWindow(), &rc);
	x -= rc.left;

	while(it!=_panes.end()) {
		ref<Pane> pane = *it;
		if(pane->_detached) {
			++it;
			idx++;
			continue;
		}
		RectF bound;
		std::wstring title = pane->GetTitle();
		g.MeasureString(title.c_str(), (INT)title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
		left += int(bound.Width) + 4 + (pane->HasIcon()?KIconWidth:0);
		if(x<left) {
			return pane;
		}
		idx++;
		++it;
	}

	return ref<Pane>(0);
}

bool TabWnd::RevealWindow(ref<Wnd> w) {
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	int idx = 0;
	while(it!=_panes.end()) {
		ref<Pane> p = *it;
		if(p->GetWindow() == w) {
			SelectPane(idx);
			return true;
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
