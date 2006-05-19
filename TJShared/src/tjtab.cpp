#include "../include/tjshared.h"
using namespace Gdiplus;

TabWnd::TabWnd(HWND parent, RootWnd* root): ChildWnd(L"TabWnd", parent) {
	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	_headerHeight = defaultHeaderHeight;
	_hotkey = L'O';
	_root = root;
	Show(true);
}

TabWnd::~TabWnd() {
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
		SolidBrush br(theme->GetTimeBackgroundColor());

		g.FillRectangle(&br, Rect(rect.left, rect.top, rect.right, _current?_headerHeight:(rect.bottom-rect.top)));
		
		g.SetSmoothingMode(SmoothingModeDefault);
		g.SetCompositingQuality(CompositingQualityDefault);
		Pen back(theme->GetTimeBackgroundColor(), 2.0f);
		g.DrawRectangle(&back, RectF(0.0f, float(_headerHeight), float(rect.right-rect.left-1), float(rect.bottom-rect.top-_headerHeight+1)));

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
			
			if(pane==_dragging) {
				Color tstart = theme->GetActiveStartColor();
				Color tend = theme->GetActiveEndColor();
				Color start(80, tstart.GetR(), tstart.GetG(), tstart.GetB());
				Color end(80, tend.GetR(), tend.GetG(), tend.GetB());

				LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), start, end);
				g.FillRectangle(&lbr, RectF(float(left+1), 2.0f, float(bound.Width+2), float(_headerHeight)));
			}

			g.DrawString(pane->_title.c_str(), (INT)pane->_title.length(), theme->GetGUIFontBold(), PointF(float(left+2), 3.0f), &textBrush);

			left += int(bound.Width) + 4;
			it++;
			idx++;
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
	if(_current) {
		RECT rct;
		GetClientRect(_wnd, &rct);
		SetWindowPos(_current->_wnd->GetWindow(), 0, 2,rct.top+_headerHeight,rct.right-rct.left-3,rct.bottom-rct.top-_headerHeight-1, SWP_NOZORDER);
		//_current->Move(rct.left, rct.top+_headerHeight, rct.right-rct.left, rct.bottom-rct.top-_headerHeight);
	
		std::vector< ref<Pane> >::iterator it = _panes.begin();
		while(it!=_panes.end()) {
			ref<Pane> pane = *it;
			SetWindowPos(pane->_wnd->GetWindow(), 0, 2,rct.top+_headerHeight,rct.right-rct.left-3,rct.bottom-rct.top-_headerHeight-1, SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE);
		
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