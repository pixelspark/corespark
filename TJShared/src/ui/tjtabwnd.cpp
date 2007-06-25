#include "../../include/tjshared.h"
#include "../../include/tjplatform.h"
#include <windowsx.h> 
using namespace Gdiplus;
using namespace tj::shared;

TabWnd::TabWnd(RootWnd* root): ChildWnd(L"TabWnd", NULL) {
	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	_headerHeight = defaultHeaderHeight;
	_root = root;
	_detachAttachAllowed = true;

	std::wstring fn = ResourceManager::Instance()->Get(L"icons/shared/tab_close.png");
	_closeIcon = Bitmap::FromFile(fn.c_str(), TRUE);
	fn = ResourceManager::Instance()->Get(L"icons/shared/tab_add.png");
	_addIcon = Bitmap::FromFile(fn.c_str(), TRUE);
	_childStyle = false;
	Layout();
}

TabWnd::~TabWnd() {
	delete _closeIcon;
	delete _addIcon;
}

void TabWnd::SetDetachAttachAllowed(bool allow) {
	_detachAttachAllowed = allow;
	Repaint();
}

void TabWnd::Add(ref<Wnd> child) {
	Throw(L"You cannot call Add on a TabWnd, use AddPane instead", ExceptionTypeSevere);
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

void TabWnd::Paint(Graphics& g) {
	g.SetSmoothingMode(SmoothingModeDefault);
	g.SetCompositingQuality(CompositingQualityDefault);

	if(_headerHeight>0) {
		Area rect = GetClientArea();
		ref<Theme> theme = ThemeManager::GetTheme();
		
		// draw background
		if(_childStyle) {
			Rect backrc(rect.GetLeft(), rect.GetTop(), rect.GetWidth(), _current?_headerHeight:(rect.GetHeight()));
			SolidBrush white(theme->GetBackgroundColor());
			LinearGradientBrush bbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), theme->ChangeAlpha(theme->GetActiveEndColor(), 50), theme->ChangeAlpha(theme->GetActiveEndColor(), 0));
			g.FillRectangle(&white, backrc);
			g.FillRectangle(&bbr, backrc);
		}
		else {
			HWND root = GetAncestor(GetWindow(), GA_ROOT);
			Gdiplus::Brush* abr = theme->GetApplicationBackgroundBrush(root, GetWindow());
			if(abr!=0) {
				g.FillRectangle(abr, Rect(rect.GetLeft(), rect.GetTop(), rect.GetRight(), _current?_headerHeight:(rect.GetHeight())));
				Pen back(abr, 2.0f);
				g.DrawRectangle(&back, RectF(0.0f, float(_headerHeight), float(rect.GetWidth()-1), float(rect.GetHeight()-_headerHeight+1)));
			
				delete abr;
			}
		}

		if(_root) {
			ref<TabWnd> dt =  _root->GetDragTarget();
			if(dt && dt.GetPointer()==this) {
				LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f,float(_headerHeight)), theme->GetHighlightColorStart(), theme->GetHighlightColorEnd());
				g.FillRectangle(&br, Rect(rect.GetLeft()+1, rect.GetTop(), rect.GetRight(), _headerHeight));
			}
		}
		
		Pen border(theme->GetActiveEndColor(), 1.0f);
		g.DrawRectangle(&border, RectF(0, float(_headerHeight)-1, float(rect.GetWidth())-1, float(rect.GetHeight()-_headerHeight)));

		std::vector< ref<Pane> >::iterator it = _panes.begin();
		SolidBrush textBrush = theme->GetTextColor();
		int left = 0;
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
					LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), theme->GetActiveEndColor(), theme->GetActiveEndColor());
					g.FillRectangle(&lbr, RectF(float(left), 2.0f, float(bound.Width+2+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight)));
					SolidBrush backBrush(theme->GetTabButtonBackgroundColor());
					g.FillRectangle(&backBrush, RectF(float(left+1.0f), 3.0f, float(bound.Width+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight)));
				}
				else {
					LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), theme->ChangeAlpha(theme->GetActiveEndColor(), 0), theme->GetActiveEndColor());
					LinearGradientBrush bbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), theme->ChangeAlpha(theme->GetActiveEndColor(), 0), theme->ChangeAlpha(theme->GetActiveEndColor(), 127));
					Pen pn(&lbr, 1.0f);
					
					g.DrawRectangle(&pn, RectF(float(left), -1.0f, float(bound.Width+2+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight-2)));
					g.FillRectangle(&bbr, RectF(float(left+1.0f), 0.0f, float(bound.Width+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight-2)));
				}
			}
			
			// gradient achter actieve tab of dragging tab (niet border)
			if((pane==_current && pane!=_dragging) || pane==_dragging) {
				Color start = theme->GetTabButtonColorStart();
				Color end = theme->GetTabButtonColorEnd();

				if(!_childStyle) {
					LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), start, end);
					g.FillRectangle(&lbr, RectF(float(left), 2.0f, float(bound.Width+2+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight-2)));
				
					LinearGradientBrush gbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)/2.0f), theme->GetGlassColorStart(), theme->GetGlassColorEnd());
					g.FillRectangle(&gbr, RectF(float(left), 2.0f, float(bound.Width+2+(pane->HasIcon()?KIconWidth:0)), float(_headerHeight/2.0f-2)));
				}
			}

			if(pane->HasIcon()) {
				g.DrawImage(pane->GetIcon(),RectF(float(left)+4.0f, 4.0f, (float)KRealIconWidth, (float)KRealIconWidth));
			}
			g.DrawString(title.c_str(), (INT)title.length(), theme->GetGUIFontBold(), PointF(float(left+1+(pane->HasIcon()?KIconWidth:0)), 4.0f), &textBrush);

			left += int(bound.Width) + 4 + (pane->HasIcon()?KIconWidth:0);
			++it;
			idx++;
		}
 
		if(!_childStyle && _detachAttachAllowed && left<(rect.GetWidth()-2*_headerHeight) && _addIcon!=0 && _closeIcon!=0) {
			g.DrawImage(_addIcon, RectF(float(rect.GetWidth()-2*_headerHeight), 0.0f, float(_headerHeight-2), float(_headerHeight-2)));
			g.DrawImage(_closeIcon, RectF(float(rect.GetWidth()-_headerHeight), 0.0f, float(_headerHeight-2), float(_headerHeight-2)));
		}
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

ref<Pane> TabWnd::AddPane(std::wstring name, ref<Wnd> wnd, bool closable, bool select, std::wstring icon) {
	assert(wnd);
	wnd->Show(false);
	SetParent(wnd->GetWindow(), GetWindow());
	ref<Pane> pane = GC::Hold(new Pane(name,wnd,false, closable, icon));
	_panes.push_back(pane);

	if(select) {
		SelectPane(int(_panes.size())-1);
	}
	else if(!_current) {
		SelectPane(0);
	}
	return pane;
}

ref<Pane> TabWnd::AddPane(ref<Pane> pane) {
	assert(pane && pane->GetWindow());
	pane->GetWindow()->Show(false);
	SetParent(pane->GetWindow()->GetWindow(), GetWindow());

	_panes.push_back(pane);
	if(_panes.size()==1) {
		SelectPane(0);
	}

	return pane;
}

ref<Pane> TabWnd::GetPane(int index) {
	try {
		return _panes.at(index);
	}
	catch(...) {
		return 0;
	}
}

void TabWnd::RemovePane(ref<Wnd> wnd) {
	assert(wnd);

	wnd->Show(false);
	_current = 0;
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		ref<Pane> pane = *it;
		if(pane->GetWindow() == wnd) {
			_panes.erase(it);
			return;
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

	Update();
}

LRESULT TabWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE) {
		Layout();
	}
	else if(msg==WM_PARENTNOTIFY||msg==WM_KILLFOCUS) {
		Repaint();
	}
	else if(msg==WM_APPCOMMAND) {
		if(GET_APPCOMMAND_LPARAM(lp)==APPCOMMAND_BROWSER_BACKWARD) {
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
			return TRUE;
		}
		else if(GET_APPCOMMAND_LPARAM(lp)==APPCOMMAND_BROWSER_FORWARD) {
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
			return TRUE;
		}
	}
	else if(msg==WM_CONTEXTMENU) {
		// what tab did the user click?
		int x = GET_X_LPARAM(lp);
		int y = GET_Y_LPARAM(lp);

		DoContextMenu(x,y);
	}
	return ChildWnd::Message(msg,wp,lp);
}

void TabWnd::DoContextMenu(int x, int y) {
	RECT rc;
	GetClientRect(GetWindow(), &rc);
	ref<Pane> pane = GetPaneAt(x);

	if(pane) {
		enum {cmdDetach=1,cmdFullScreen};
		ContextMenu context;
		context.AddItem(TL(detach_tab), cmdDetach, true);
		switch(context.DoContextMenu(GetWindow(), x+rc.left, y)) {
			case cmdDetach:
				_current = 0;
				Detach(pane);
				break;
		}
		
		Update();
	}
}

void TabWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown || ev==MouseEventLUp) {
		std::vector< ref<Pane> >::iterator it = _panes.begin();
		unsigned int idx = 0; 
		int left = 0;
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
					_dragging = 0;
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

		Area rect = GetClientArea();
		if(_detachAttachAllowed && left<(rect.GetWidth()-2*_headerHeight) && ev==MouseEventLDown) {
			if(x>rect.GetWidth()-_headerHeight && y<_headerHeight) {
				// close button
				if(_current && _root) {
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
						_root->AddOrphanPane(_current);
					}
					_current = 0;
					SelectPane(0);
					Update();
				}
			}
			else if(x>rect.GetWidth()-2*_headerHeight) {
				// add button
				DoAddMenu(x,y);
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
	else if(ev==MouseEventMove) {
		if(_dragging && ISVKKEYDOWN(VK_LBUTTON)) {
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
					_dragging = 0;
				}

				_dragStartX = x;
				_dragStartY = y;
			}
			else if(dy > TearOffLimit) {
				Detach(_dragging);
				_dragging = 0;
			}
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
		_current = 0;
	}
	ref<FloatingPane> fp = _root->AddFloatingPane(p, this);
	ReleaseCapture();
	SendMessage(fp->GetWindow(), WM_NCLBUTTONDOWN, HTCAPTION, 0);
	Update();
}

void TabWnd::Attach(ref<Pane> p) {
	if(!_detachAttachAllowed) return;

	AddPane(p);
	SetParent(p->GetWindow()->GetWindow(), GetWindow());
	Update();
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

void TabWnd::DoAddMenu(int x, int y) {
	ContextMenu m;
	std::vector< ref<Pane> >* pv = _root->GetOrphanPanes();
	std::vector< ref<Pane> >::iterator it = pv->begin();
	int n = 1;
	while(it!=pv->end()) {
		ref<Pane> pane = *it;
		if(pane) {
			m.AddItem(pane->GetTitle(), n, false, false);
			n++;
		}
		++it;
	}

	int c = m.DoContextMenu(GetWindow(), x,y,true);
	if(c>0) {
		try {
			if(_current) {
				_current->GetWindow()->Show(false);
			}

			ref<Pane> selected = pv->at(c-1);

			if(selected) {
				_root->RemoveOrphanPane(selected);
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