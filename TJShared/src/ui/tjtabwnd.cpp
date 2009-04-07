#include "../../include/ui/tjui.h" 

#ifdef TJ_OS_WIN
	// TODO: this used?
	#include <windowsx.h>
#endif

using namespace tj::shared::graphics;
using namespace tj::shared;

TabWnd::TabWnd(ref<WindowManager> root, const std::wstring& id): ChildWnd(L"TabWnd"), 
	_closeIcon(Icons::GetIconPath(Icons::IconTabClose)),
	_addIcon(Icons::GetIconPath(Icons::IconTabAdd)),
	_closeActiveIcon(Icons::GetIconPath(Icons::IconTabCloseActive)),
	_addActiveIcon(Icons::GetIconPath(Icons::IconTabAddActive)),
	_offset(0) {

	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
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
		return Pixels(_width * _appearAnimation.GetFraction()) + ((_pane && _pane->HasIcon()) ? TabWnd::KIconWidth : 0) + 1;
	}

	if(_pane) {
		if(_width==0) {
			// Icon-only pane (even if the pane has no icon, showing a little tab is better than nothing)
			return TabWnd::KIconWidth + 4;
		}
		else if(_width>0) {
			return _width + (_pane->HasIcon() ? TabWnd::KIconWidth : 0) + 1;
		}
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
	else {
		Layout();
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
	if(pane) {
		_entryAnimation.Start(Time(300), true);
		StartTimer(1, Time(10));
	}
	Update();
}

void TabWnd::TabPane::Paint(Graphics& g, strong<Theme> theme, const Area& tab, bool isCurrent, bool isDragging, bool childStyle, float entryFraction) {
	SolidBrush textBrush = theme->GetColor(Theme::ColorText);
	SolidBrush inactiveTextBrush = Theme::ChangeAlpha(theme->GetColor(Theme::ColorText), int(127.0f + 127.0f * entryFraction));

	// border and background of pane
	if(isCurrent) {
		Area back = tab;
		
		if(childStyle) {
			back.Narrow(0,0,0,1);
			LinearGradientBrush lbr(PointF(float(back.GetLeft()), float(back.GetTop())), PointF(float(back.GetLeft()), float(back.GetBottom())), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 0), theme->GetColor(Theme::ColorActiveEnd));
			LinearGradientBrush bbr(PointF(float(back.GetLeft()), float(back.GetTop())), PointF(float(back.GetLeft()), float(back.GetBottom())), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 0), theme->ChangeAlpha(theme->GetColor(Theme::ColorActiveEnd), 127));
			Pen pn(&lbr, 1.0f);
			back.Narrow(0,0,0,2);
			g.FillRectangle(&bbr, back);
			g.DrawRectangle(&pn, back);
		}
		else {
			back.Widen(2,0,0,2);
			LinearGradientBrush lbr(PointF(float(back.GetLeft()), float(back.GetTop())), PointF(float(back.GetLeft()), float(back.GetBottom())), theme->GetColor(Theme::ColorActiveEnd), theme->GetColor(Theme::ColorActiveEnd));
			SolidBrush backBrush(theme->GetColor(Theme::ColorTabButtonBackground));
			back.Narrow(2,0,0,2);
			g.FillRectangle(&lbr, back);
			back.Narrow(1,1,1,0);
			g.FillRectangle(&backBrush, back);
		}
	}
			
	// gradient achter actieve tab of dragging tab (niet border)
	if(isDragging || isCurrent) {
		if(!childStyle) {
			Color start = theme->GetColor(Theme::ColorTabButtonStart);
			Color end = theme->GetColor(Theme::ColorTabButtonEnd);
			Area gradient = tab;
			gradient.Narrow(1,1,1,0);

			LinearGradientBrush lbr(PointF(float(tab.GetLeft()), float(tab.GetTop()-1)), PointF(float(tab.GetLeft()), float(tab.GetBottom()+1)), start, end);
			g.FillRectangle(&lbr, gradient);
		
			gradient.Narrow(1,1,1,gradient.GetHeight()/2);
			LinearGradientBrush gbr(PointF(float(tab.GetLeft()), float(tab.GetTop()-1)), PointF(float(tab.GetLeft()), float(tab.GetBottom()-(tab.GetHeight()/2))), theme->GetColor(Theme::ColorGlassStart), theme->GetColor(Theme::ColorGlassEnd));
			g.FillRectangle(&gbr, gradient);
		}
	}

	if(_pane) {
		// Draw icon (if any)
		if(_pane->HasIcon()) {
			ref<Icon> icon = _pane->GetIcon();
			if(icon) {
				Area iconArea = tab;
				iconArea.Narrow(4,4,0,0);
				iconArea.SetHeight(TabWnd::KRealIconWidth);
				iconArea.SetWidth(TabWnd::KRealIconWidth);
				icon->Paint(g, iconArea, isCurrent ? 1.0f : (0.5f+(entryFraction/2.0f)));
			}
		}

		// Draw text
		std::wstring title = _pane->GetTitle();
		StringFormat sf;
		sf.SetFormatFlags(StringFormatFlagsLineLimit);
		sf.SetAlignment(StringAlignmentNear);
		Area labelArea = tab;
		labelArea.Narrow(1 + (_pane->HasIcon() ? TabWnd::KIconWidth : 0), 4, 0, 0);
		AreaF labelAreaShadow(float(labelArea.GetLeft()), float(labelArea.GetTop()), float(labelArea.GetWidth()), float(labelArea.GetHeight()));
		labelAreaShadow.Translate(0.8f, 0.8f);

		SolidBrush shadowBrush(Theme::ChangeAlpha(theme->GetColor(Theme::ColorBackground),172));
		g.DrawString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), labelAreaShadow, &sf, &shadowBrush);
		g.DrawString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), labelArea, &sf, isCurrent ? &textBrush : &inactiveTextBrush);
	}
}

void TabWnd::Paint(Graphics& g, strong<Theme> theme) {
	if(_headerHeight>0) {	
		Area rect = GetClientArea();

		// Calculate total width and offset for scroller
		Pixels totalTabsWidth = GetTotalTabWidth();
		bool showScroller = (totalTabsWidth > (rect.GetWidth()-2*KIconWidth));
		Pixels scrollerHeight = (showScroller && !_childStyle) ?KScrollerHeight : 0;
		Pixels offset = showScroller ? _offset : 0;
	
		// draw background (child style tab windows have their own; others use the application background)
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

		// If this tab window is the target for dropping a tab pane, then highlight it
		ref<WindowManager> root = _root;
		if(root) {
			ref<TabWnd> dt =  root->GetDragTarget();
			if(dt.GetPointer() == this) {
				LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f,float(_headerHeight)), theme->GetColor(Theme::ColorHighlightStart), theme->GetColor(Theme::ColorHighlightEnd));
				g.FillRectangle(&br, Rect(rect.GetLeft()+1, rect.GetTop(), rect.GetRight(), _headerHeight));
			}
		}
		
		// Draw line around pane contents
		Pen border(theme->GetColor(Theme::ColorActiveEnd), 1.0f);
		g.DrawRectangle(&border, Area(rect.GetLeft(), _headerHeight-1, rect.GetWidth()-1, rect.GetHeight()-_headerHeight));
		
		// Draw pane buttons
		Pixels currentLeft = rect.GetLeft() - offset;
		std::vector<TabPane>::iterator it = _panes.begin();
		std::vector<TabPane>::iterator end = _panes.end();
		TabPane* draggingPane = 0;
		float entryFraction = _entryAnimation.GetFraction();

		while(it!=end) {
			TabPane& tp = *it;
			Pixels paneWidth = tp.GetWidth() + 2;
				
			ref<Pane> pane = tp.GetPane();
			if(pane) {
				if(pane->_detached) {
					++it;
					continue;
				}
				
				// We draw the dragged pane later
				if(pane==_dragging) {
					draggingPane = &tp;
				}
				else {
					Area tab(currentLeft, rect.GetTop()+scrollerHeight+1, paneWidth, _headerHeight - scrollerHeight - 1);
					tp.Paint(g, theme, tab, _current && pane==_current, false, _childStyle, entryFraction);
				}
				currentLeft += paneWidth;
			}
			++it;
		}

		// Get current mouse position
		POINT lp;
		GetCursorPos(&lp);
		if(ScreenToClient(GetWindow(), &lp)) {
			lp.x = long(lp.x / theme->GetDPIScaleFactor());
			lp.y = long(lp.y / theme->GetDPIScaleFactor());
		}

		// Draw the pane that is being dragged
		if(draggingPane != 0) {
			Area tab(lp.x - draggingPane->GetWidth()/2, rect.GetTop()+scrollerHeight+1, draggingPane->GetWidth() + 2, _headerHeight - scrollerHeight - 1);
			ref<Pane> pane = draggingPane->GetPane();
			if(pane) {
				draggingPane->Paint(g, theme, tab, _current && pane==_current, true, _childStyle, entryFraction);
			}
		}
		
		// Draw buttons on the right
		if(!_childStyle && !_dragging && _detachAttachAllowed && _addIcon!=0 && _closeIcon!=0) {
			Pixels buttonsLeft = rect.GetRight()-2*19;

			if(!((totalTabsWidth+offset)<(rect.GetWidth()-2*_headerHeight))) {
				HWND root = GetAncestor(GetWindow(), GA_ROOT);
				graphics::Brush* abr = theme->GetApplicationBackgroundBrush(root, GetWindow());
				g.FillRectangle(abr, RectF((float)buttonsLeft, 0.0f, float(rect.GetWidth()-buttonsLeft), float(_headerHeight)-1.0f));
				delete abr;
			}

			Area addArea(buttonsLeft, rect.GetTop(), 19, 19);
			if(addArea.IsInside(lp.x, lp.y)) {
				_addActiveIcon.Paint(g, addArea, 0.5f + (_entryAnimation.GetFraction()/2.0f));
			}
			else {
				_addIcon.Paint(g, addArea, 0.5f + (_entryAnimation.GetFraction()/2.0f));
			}

			if(_current) {
				Area closeArea(buttonsLeft+19, rect.GetTop(), 19, 19);
				if(closeArea.IsInside(lp.x, lp.y)) {
					_closeActiveIcon.Paint(g, Area(rect.GetWidth()-_headerHeight, 0, 19, 19), 0.5f + (_entryAnimation.GetFraction()/2.0f));
				}
				else {
					_closeIcon.Paint(g, Area(rect.GetWidth()-_headerHeight, 0, 19, 19), 0.5f + (_entryAnimation.GetFraction()/2.0f));
				}
			}
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

	bool removedWasCurrent = false;
	std::vector<TabPane>::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		TabPane& tp = *it;
		strong<Pane> pane = tp._pane;
		if(pane->GetWindow() == wnd) {
			wnd->Show(false);

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
	Layout();
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

#ifdef TJ_OS_WIN
	LRESULT TabWnd::PreMessage(UINT msg, WPARAM wp, LPARAM lp) {
		/** The browser back/forward keys are used to navigate tabs in TabWnd. We don't want
		those messages to propagate to parent windows, because we can handle them. Since the
		WM_APPCOMMAND message is automatically propagated to the parent when it is not handled,
		we should return 0 here to prevent DefWindowProc from doing that. **/
		if(msg==WM_APPCOMMAND) {
			int c = GET_APPCOMMAND_LPARAM(lp);
			if(c==APPCOMMAND_BROWSER_BACKWARD || c==APPCOMMAND_BROWSER_FORWARD) {
				ChildWnd::PreMessage(msg,wp,lp);
				return 0;
			}
		}
		return ChildWnd::PreMessage(msg,wp,lp);
	}
#endif

void TabWnd::OnMouseWheelMove(WheelDirection wd) {
	if(wd==WheelDirectionDown) {
		_offset += 5;
	}
	else {
		_offset -= 5;
	}
	FixScrollerOffset();
	Repaint();
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
			SetDraggingPane(null);
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
							SetDraggingPane(null);
						}
						else if(ev==MouseEventMUp) {
							ClosePane(pane);
						}
						else {
							if(_current==ref<Pane>(pane)) {
								Mouse::Instance()->SetCursorType(CursorHandGrab);
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
					_capture.StartCapturing(Mouse::Instance(), ref<Wnd>(this));
				}
				else if(ev==MouseEventLUp) {
					SetDraggingPane(null);
					Mouse::Instance()->SetCursorType(CursorDefault);
					Repaint();
				}
			}
			else {
				_capture.StopCapturing();
			}
		}
	}
	else if(ev==MouseEventRDown) {
		_dragStartX = x;
		_dragStartY = y;
		_capture.StartCapturing(Mouse::Instance(), ref<Wnd>(this));
	}
	else if(ev==MouseEventRUp) {
		_capture.StopCapturing();
		SetDraggingPane(null);
	}
	else if(ev==MouseEventLeave) {
		_in = false;
		SetDraggingPane(null);
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

		if(_in) {
			Area rc = GetClientArea();
			if(x>rc.GetRight()-2*19 && y<_headerHeight) {
				Repaint();
			}
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
				SetDraggingPane(null);
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
			Pixels xl = x;  /* - ((*dit).GetWidth()/2); */
			Pixels xr = x + ((*dit).GetWidth());

			// Dragging to the left
			std::vector<TabPane>::iterator leftit = GetPaneIteratorAt(x,xo);
			std::vector<TabPane>::iterator rightit = GetPaneIteratorAt(x,xo);
			
			std::vector<TabPane>::iterator swappingWith = _panes.end();
			if(leftit!=_panes.end() && leftit < dit) {
				swappingWith = leftit;
			}

			if(rightit!=_panes.end() && rightit > dit) {
				swappingWith = rightit;
			}

			if(swappingWith!=_panes.end()) {
				TabPane& tp = *swappingWith;
				if(tp._pane!=(*dit)._pane) {
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
		_capture.StopCapturing();
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
