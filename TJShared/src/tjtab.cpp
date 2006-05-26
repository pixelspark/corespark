#include "../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

TabWnd::TabWnd(HWND parent, RootWnd* root): ChildWnd(L"TabWnd", parent) {
	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	_headerHeight = defaultHeaderHeight;
	_hotkey = L'O';
	_root = root;

	std::wstring fn = ResourceManager::Instance()->Get(L"icons/tab_close.png");
	_closeIcon = Bitmap::FromFile(fn.c_str(), TRUE);
	fn = ResourceManager::Instance()->Get(L"icons/tab_add.png");
	_addIcon = Bitmap::FromFile(fn.c_str(), TRUE);
	Layout();
	Show(true);
}

TabWnd::~TabWnd() {
	delete _closeIcon;
	delete _addIcon;
}

void TabWnd::Rename(ref<Wnd> wnd, std::wstring name) {
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		ref<Pane> p = *it;
		if(p->_wnd ==wnd) {
			p->_title = name;
			Update();
			return;
		}
		it++;
	}
}

void TabWnd::SetDraggingPane(ref<Pane> pane) {
	_dragging = pane;
	Update();
}

void TabWnd::Paint(Graphics& g) {
	if(_headerHeight>0) {
		RECT rect;
		GetClientRect(_wnd, &rect);
		ref<Theme> theme = ThemeManager::GetTheme();
		
		// draw application background
		HWND root = GetAncestor(_wnd, GA_ROOT);
		Gdiplus::Brush* abr = theme->GetApplicationBackgroundBrush(root, _wnd);
		if(abr!=0) {
			g.FillRectangle(abr, Rect(rect.left, rect.top, rect.right, _current?_headerHeight:(rect.bottom-rect.top)));
			Pen back(abr, 2.0f);
			g.DrawRectangle(&back, RectF(0.0f, float(_headerHeight), float(rect.right-rect.left-1), float(rect.bottom-rect.top-_headerHeight+1)));
		
			delete abr;
		}
		else {
			SolidBrush br(theme->GetTimeBackgroundColor());
			g.FillRectangle(&br, Rect(rect.left, rect.top, rect.right-rect.left, _current?_headerHeight:(rect.bottom-rect.top)));
		}

		if(_root) {
			ref<TabWnd> dt =  _root->GetDragTarget();
			if(dt && dt.GetPointer()==this) {
				LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f,float(_headerHeight)), theme->GetHighlightColorStart(), theme->GetHighlightColorEnd());
				g.FillRectangle(&br, Rect(rect.left+1, rect.top, rect.right, _headerHeight));
			}
		}
		
		g.SetSmoothingMode(SmoothingModeDefault);
		g.SetCompositingQuality(CompositingQualityDefault);
		
		Pen border(theme->GetActiveEndColor(), 1.0f);
		g.DrawRectangle(&border, RectF(1.0f, float(_headerHeight-1), float(rect.right-rect.left-2), float(rect.bottom-rect.top-_headerHeight)));
		//g.SetSmoothingMode(SmoothingModeHighQuality);
		//g.SetCompositingQuality(CompositingQualityHighQuality);

		std::vector< ref<Pane> >::iterator it = _panes.begin();
		SolidBrush textBrush = theme->GetTextColor();
		int left = 0;
		int idx = 0;
		while(it!=_panes.end()) {
			ref<Pane> pane = *it;
			if(pane->_detached) {
				it++;
				continue;
			}

			RectF bound;
			g.MeasureString(pane->_title.c_str(), (INT)pane->_title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
			
			if(pane==_current) {
				LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), theme->GetActiveStartColor(), theme->GetActiveEndColor());
				g.FillRectangle(&lbr, RectF(float(left+1), 2.0f, float(bound.Width+2), float(_headerHeight)));
				SolidBrush backBrush(theme->GetBackgroundColor());
				g.FillRectangle(&backBrush, RectF(float(left+2), 3.0f, float(bound.Width), float(_headerHeight)));
			}
			
			if((pane==_current && pane!=_dragging) || pane==_dragging) {
				Color start = theme->GetTabButtonColorStart();
				Color end = theme->GetTabButtonColorEnd();

				LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), start, end);
				g.FillRectangle(&lbr, RectF(float(left+1), 2.0f, float(bound.Width+2), float(_headerHeight-2)));
			}

			g.DrawString(pane->_title.c_str(), (INT)pane->_title.length(), theme->GetGUIFontBold(), PointF(float(left+2), 3.0f), &textBrush);

			left += int(bound.Width) + 4;
			it++;
			idx++;
		}

		if(left<(rect.right-rect.left-2*_headerHeight)) {
			g.DrawImage(_addIcon, RectF(float(rect.right-rect.left-2*_headerHeight), 0.0f, float(_headerHeight-2), float(_headerHeight-2)));
			g.DrawImage(_closeIcon, RectF(float(rect.right-rect.left-_headerHeight), 0.0f, float(_headerHeight-2), float(_headerHeight-2)));
		}
	}
}

void TabWnd::SetHotkey(wchar_t key) {
	_hotkey = key;
}

ref<Wnd> TabWnd::GetCurrentPane() {
	return _current->_wnd;
}

wchar_t TabWnd::GetPreferredHotkey() {
	if(_current) {
		return _current->_wnd->GetPreferredHotkey();
	}
	return L'\0';
}

void TabWnd::Clear() {
	_panes.clear();
}

void TabWnd::LeaveHotkeyMode(wchar_t key) {
	if(_current) {
		_current->_wnd->LeaveHotkeyMode(key);
	}
}

void TabWnd::EnterHotkeyMode() {
	if(_current) {
		_current->_wnd->EnterHotkeyMode();
	}
}

bool TabWnd::IsInHotkeyMode() {
	Wnd* parent = GetParent();
	if(parent==0) return false;
	return parent->IsInHotkeyMode();
}


ref<Pane> TabWnd::AddPane(std::wstring name, ref<Wnd> wnd) {
	assert(wnd);
	wnd->Show(false);
	ref<Pane> pane = GC::Hold(new Pane(name,wnd,false));
	_panes.push_back(pane);
	if(_panes.size()==1) {
		SelectPane(0);
	}

	return pane;
}

ref<Pane> TabWnd::AddPane(ref<Pane> pane) {
	assert(pane);
	pane->_wnd->Show(false);
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
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		ref<Pane> pane = *it;
		if(pane->_wnd == wnd) {
			_panes.erase(it);
			return;
		}
		it++;
	}
	SelectPane(0);
}

void TabWnd::SelectPane(unsigned int index) {
	try {
		ref<Pane> pane = _panes.at(index);

		if(_current) {
			_current->_wnd->Show(false);
		}
		pane->_wnd->Show(true);
		_current = pane;
	}
	catch(...) {
	}

	Layout();
}

void TabWnd::Update() {
	Repaint();
}

void TabWnd::Layout() {
	RECT rc;
	GetClientRect(_wnd, &rc);

	if(_current) {	
		SetWindowPos(_current->_wnd->GetWindow(), 0, 2,rc.top+_headerHeight,rc.right-rc.left-3,rc.bottom-rc.top-_headerHeight-1, SWP_NOZORDER);
		
		std::vector< ref<Pane> >::iterator it = _panes.begin();
		while(it!=_panes.end()) {
			ref<Pane> pane = *it;
			SetWindowPos(pane->_wnd->GetWindow(), 0, 2,rc.top+_headerHeight,rc.right-rc.left-3,rc.bottom-rc.top-_headerHeight-1, SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE);
		
			it++;
		}
	}

	Update();
}

LRESULT TabWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE) {
		Layout();
	}
	else if(msg==WM_LBUTTONUP || msg==WM_LBUTTONDOWN) {
		int x = GET_X_LPARAM(lp);

		std::vector< ref<Pane> >::iterator it = _panes.begin();
		unsigned int idx = 0; 
		int left = 0;
		Graphics g(_wnd);

		ref<Theme> theme = ThemeManager::GetTheme();

		while(it!=_panes.end()) {
			ref<Pane> pane = *it;
			if(pane->_detached) {
				it++;
				idx++;
				continue;
			}
			RectF bound;
			g.MeasureString(pane->_title.c_str(), (INT)pane->_title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
			left += int(bound.Width) + 4;
			if(x<left) {
				if(msg==WM_LBUTTONUP) {
					SelectPane(idx);
					_dragging = 0;
				}
				else {
					SetDraggingPane(pane);
					_dragStartX = GET_X_LPARAM(lp);
					_dragStartY = GET_Y_LPARAM(lp);
				}
				break;
			}
			idx++;
			it++;
		}

		RECT rect;
		GetClientRect(_wnd, &rect);
		if(left<(rect.right-rect.left-2*_headerHeight) && msg==WM_LBUTTONDOWN) {
			if(x>rect.right-rect.left-_headerHeight) {
				// close button
				if(_current && _root) {
					std::vector< ref<Pane> >::iterator it = _panes.begin();
					while(it!=_panes.end()) {
						ref<Pane> pn = *it;
						if(pn==_current) {
							_panes.erase(it);
							break;
						}
						it++;
					}
					_root->AddOrphanPane(_current);
					ref<Wnd> wnd = _current->GetWindow();
					if(wnd) wnd->Show(false);
					_current = 0;
					SelectPane(0);
					Update();
				}
			}
			else if(x>rect.right-rect.left-2*_headerHeight) {
				// add button
				DoAddMenu(x,GET_Y_LPARAM(lp));
			}
		}

		if(_dragging) {
			if(msg==WM_LBUTTONDOWN) {
				SetCapture(_wnd);
			}
		}
		else {
			ReleaseCapture();
		}
	}
	else if(msg==WM_MOUSEMOVE) {
		if(_dragging && ISVKKEYDOWN(VK_LBUTTON)) {
			// we're dragging a tab
			int dx = abs(_dragStartX - GET_X_LPARAM(lp));
			int dy = abs(_dragStartY - GET_Y_LPARAM(lp));
			
			if(dy < TearOffLimit && dx > TearOffLimit ) {
				// only move tabs
				try {
					std::vector< ref<Pane> >::iterator it = std::find(_panes.begin(), _panes.end(), _dragging);
					if(it!=_panes.end() && it!=_panes.begin()) {
						dx = GET_X_LPARAM(lp) - _dragStartX;
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
					else {
						Log::Write(L"TJShared/Tab", L"Could not find the tab pane you're dragging");
					}
				}
				catch(...) {
					_dragging = 0;
				}

				_dragStartX = GET_X_LPARAM(lp);
				_dragStartY = GET_Y_LPARAM(lp);
			}
			else if(dy > TearOffLimit) {
				Detach(_dragging);
				_dragging = 0;
			}
			Update();
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
	GetClientRect(_wnd, &rc);
	ref<Pane> pane = GetPaneAt(x);

	if(pane) {
		enum {cmdDetach=1,cmdFullScreen};
		ContextMenu context;
		context.AddItem(TL(detach_tab), cmdDetach, true);
		switch(context.DoContextMenu(_wnd, x+rc.left, y)) {
			case cmdDetach:
				_current = 0;
				Detach(pane);
				break;
		}
		
		Update();
	}
}

void TabWnd::Detach(ref<Pane> p) {
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	while(it!=_panes.end()) {
		if(*it == p) {
			_panes.erase(it);
			break;
		}
		it++;
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
	AddPane(p);
	SetParent(p->GetWindow()->GetWindow(), _wnd);
	Update();
}

ref<Pane> TabWnd::GetPaneAt(int x) {
	std::vector< ref<Pane> >::iterator it = _panes.begin();
	unsigned int idx = 0; 
	int left = 0;
	Graphics g(_wnd);
	ref<Theme> theme = ThemeManager::GetTheme();

	RECT rc;
	GetWindowRect(_wnd, &rc);
	x -= rc.left;

	while(it!=_panes.end()) {
		ref<Pane> pane = *it;
		if(pane->_detached) {
			it++;
			idx++;
			continue;
		}
		RectF bound;
		g.MeasureString(pane->_title.c_str(), (INT)pane->_title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
		left += int(bound.Width) + 4;
		if(x<left) {
			return pane;
		}
		idx++;
		it++;
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
		it++;
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
		it++;
	}

	int c = m.DoContextMenu(_wnd, x,y,true);
	if(c>0) {
		try {
			ref<Pane> selected = pv->at(c-1);

			if(selected) {
				_root->RemoveOrphanPane(selected);
				Attach(selected);
				_current = selected;
			}
		}
		catch(...) {
		}
	}
}