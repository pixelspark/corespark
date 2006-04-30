#include "../include/tjshared.h"
using namespace Gdiplus;

TabWnd::TabWnd(HWND parent): ChildWnd(L"TabWnd", parent) {
	SetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	_headerHeight = defaultHeaderHeight;
	_hotkey = L'O';
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
		if(pane->_detached) {
			HWND paneWnd = pane->_wnd->GetWindow();
			
			int style = GetWindowLong(paneWnd, GWL_STYLE);

			if((style&WS_VISIBLE)!=0) {
				HWND panel = ::GetParent(paneWnd);
				SetForegroundWindow(panel);
				_current = 0;
				return;
			}
			else {
				pane->SetDetached(false,this);
			}
		}

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
				}
				break;
			}
			idx++;
			it++;
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
				pane->SetDetached(true,this);
				break;
		}
		
		Update();
	}
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

Pane::Pane(std::wstring title, ref<Wnd> window, bool detached) {
	_title = title;
	_wnd = window;
	_detached = detached;
}

void Pane::SetDetached(bool d, TabWnd* tab) {
	assert(_wnd);
	assert(tab!=0);
	if(!d) {
		SetParent(_wnd->GetWindow(),tab->GetWindow());
		//_wnd->Show(true);
		_wnd->Update();
		UpdateWindow(_wnd->GetWindow());
	}
	else {
		// Create holder window
		HWND parent = 0L;
		RECT tabrc;
		GetWindowRect(tab->GetWindow(), &tabrc);

		HWND panel = CreateWindowEx(WS_EX_PALETTEWINDOW, TJ_TAB_PANEL_CLASS_NAME, L"Panel", WS_OVERLAPPEDWINDOW,  tabrc.left+30, tabrc.top+30, tabrc.right-tabrc.left, tabrc.bottom-tabrc.top, parent, 0L, GetModuleHandle(NULL), 0L);
		SetWindowText(panel, _title.c_str());
		HWND window = _wnd->GetWindow();
		SetParent(window,panel);

		SendMessage(panel, WM_SIZE, 0,0);
		ShowWindow(panel, SW_SHOW);
	}
	_detached = d;
}

ref<Wnd> Pane::GetWindow() {
	return _wnd;
}

LRESULT CALLBACK TabPanelWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CREATE) {
		return 1;
	}
	else if(msg==WM_CLOSE) {
		HWND child = GetWindow(wnd, GW_CHILD);
		if(child!=0) {
			// hide it somewhere so it won't be destroyed and TabWnd can pick it back up
			ShowWindow(child, SW_HIDE);
			SetParent(child,0L);
		}
		DestroyWindow(wnd);
		return 0;
	}
	else if(msg==WM_SIZE) {
		// update size of child window
		HWND child = GetWindow(wnd, GW_CHILD);
		if(child!=0) {
			RECT r;
			GetClientRect(wnd, &r);
			SetWindowPos(child, 0, 0,0, r.right-r.left, r.bottom-r.top, SWP_NOZORDER);
		}
	}
	else if(msg==WM_KEYDOWN) {
		if(wp==VK_ESCAPE) {
			SetWindowLong(wnd, GWL_STYLE, WS_OVERLAPPEDWINDOW|WS_VISIBLE);
			UpdateWindow(wnd);
			SetWindowPos(wnd, 0L, 100, 100, 640, 480, SWP_NOZORDER);
		}
	}
	else if(msg==WM_CONTEXTMENU) {
		int style = GetWindowLong(wnd, GWL_STYLE);
		int command = 0;
		enum {cmdNothing = 0, cmdExit, cmdEnter};
		ContextMenu men;

		if((style & WS_POPUP) >0) {
			// we're fullscreen
			men.AddItem(TL(leave_full_screen), cmdExit, true);
		}
		else {
			men.AddItem(TL(enter_full_screen), cmdEnter, true);
		}

		command = men.DoContextMenu(wnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp), false);
		if(command==cmdEnter) {
			SetWindowLong(wnd, GWL_STYLE, WS_POPUP|WS_VISIBLE);
			RECT rect;
			GetWindowRect(wnd,&rect);
			HMONITOR mon = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);

			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(mon, &mi);

			SetWindowPos(wnd,0,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,SWP_NOZORDER);
		}
		else if(command==cmdExit) {
			SetWindowLong(wnd, GWL_STYLE, WS_OVERLAPPEDWINDOW|WS_VISIBLE);
			UpdateWindow(wnd);
			SetWindowPos(wnd, 0L, 100, 100, 640, 480, SWP_NOZORDER);
		}
	}

	return DefWindowProc(wnd, msg, wp, lp);
}