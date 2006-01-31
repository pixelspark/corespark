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

void TabWnd::Paint(Graphics& g) {
	if(_headerHeight>0) {
		RECT rect;
		GetClientRect(_wnd, &rect);
		ref<Theme> theme = ThemeManager::GetTheme();
		SolidBrush br(theme->GetTimeBackgroundColor());

		g.FillRectangle(&br, Rect(rect.left, rect.top, rect.right, _current?_headerHeight:(rect.bottom-rect.top)));
		
		Pen pn(theme->GetActiveEndColor(), 1.0f);
		g.DrawLine(&pn, 0, _headerHeight-1, rect.right, _headerHeight-1);

		std::vector< Pane >::iterator it = _panes.begin();
		SolidBrush textBrush = theme->GetTextColor();
		int left = 0;
		int idx = 0;
		while(it!=_panes.end()) {
			Pane& pane = *it;
			RectF bound;
			g.MeasureString(pane._title.c_str(), (INT)pane._title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
			
			if(pane._wnd==_current) {
				LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(_headerHeight)), theme->GetActiveStartColor(), theme->GetActiveEndColor());
				g.FillRectangle(&lbr, RectF(float(left+2), 2.0f, float(bound.Width+2), float(_headerHeight)));
				SolidBrush backBrush(theme->GetBackgroundColor());
				g.FillRectangle(&backBrush, RectF(float(left+3), 3.0f, float(bound.Width), float(_headerHeight)));
			}

			g.DrawString(pane._title.c_str(), (INT)pane._title.length(), theme->GetGUIFontBold(), PointF(float(left+3), 3.0f), &textBrush);

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
	return _current;
}

wchar_t TabWnd::GetPreferredHotkey() {
	if(_current) {
		return _current->GetPreferredHotkey();
	}
	return L'\0';
}

void TabWnd::Clear() {
	_panes.clear();
}

void TabWnd::LeaveHotkeyMode(wchar_t key) {
	if(_current) {
		_current->LeaveHotkeyMode(key);
	}
}

void TabWnd::EnterHotkeyMode() {
	if(_current) {
		_current->EnterHotkeyMode();
	}
}

bool TabWnd::IsInHotkeyMode() {
	Wnd* parent = GetParent();
	if(parent==0) return false;
	return parent->IsInHotkeyMode();
}


void TabWnd::AddPane(std::wstring name, ref<Wnd> wnd) {
	assert(wnd);
	wnd->Show(false);
	_panes.push_back(Pane(name,wnd,false));
	if(_panes.size()==1) {
		SelectPane(0);
	}
}

void TabWnd::SelectPane(unsigned int index) {
	try {
		Pane& pane = _panes.at(index);
		if(pane._detached) {
			HWND paneWnd = pane._wnd->GetWindow();
			
			int style = GetWindowLong(paneWnd, GWL_STYLE);

			if((style&WS_VISIBLE)!=0) {
				HWND panel = ::GetParent(paneWnd);
				SetForegroundWindow(panel);
				_current = 0;
				return;
			}
			else {
				pane.SetDetached(false,this);
			}
		}

		if(_current) {
			_current->Show(false);
		}
		pane._wnd->Show(true);
		_current = pane._wnd;
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
		SetWindowPos(_current->GetWindow(), 0, 0,rct.top+_headerHeight,rct.right-rct.left,rct.bottom-rct.top-_headerHeight, SWP_NOZORDER);
		//_current->Move(rct.left, rct.top+_headerHeight, rct.right-rct.left, rct.bottom-rct.top-_headerHeight);
	}
	Update();
}

LRESULT TabWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE) {
		Layout();
	}
	else if(msg==WM_LBUTTONDOWN) {
		int x = GET_X_LPARAM(lp);

		std::vector< Pane >::iterator it = _panes.begin();
		unsigned int idx = 0; 
		int left = 0;
		Graphics g(_wnd);

		ref<Theme> theme = ThemeManager::GetTheme();

		while(it!=_panes.end()) {
			Pane& pane = *it;
			RectF bound;
			g.MeasureString(pane._title.c_str(), (INT)pane._title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
			left += int(bound.Width) + 4;
			if(x<left) {
				SelectPane(idx);
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
	std::vector< Pane >::iterator it = _panes.begin();
	unsigned int idx = 0; 
	int left = 0;
	Graphics g(_wnd);
	ref<Theme> theme = ThemeManager::GetTheme();
	
	RECT rc;
	GetWindowRect(_wnd, &rc);
	x -= rc.left;


	while(it!=_panes.end()) {
		Pane& pane = *it;
		RectF bound;
		g.MeasureString(pane._title.c_str(), (INT)pane._title.length(), theme->GetGUIFontBold(), PointF(0.0f, 0.0f), &bound);				
		left += int(bound.Width) + 4;
		if(x<left) {
			enum {cmdDetach=1,};
			ContextMenu context;
			context.AddItem(TL(detach_tab), cmdDetach, true);
			switch(context.DoContextMenu(_wnd, x+rc.left, y)) {
				case cmdDetach:
					_current = 0;
					pane.SetDetached(true,this);
					break;
			}
			
			Update();
			return;
		}
		idx++;
		it++;
	}
}

TabWnd::Pane::Pane(std::wstring title, ref<Wnd> window, bool detached) {
	_title = title;
	_wnd = window;
	_detached = detached;
}

void TabWnd::Pane::SetDetached(bool d, TabWnd* tab) {
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
	return DefWindowProc(wnd, msg, wp, lp);
}