#include "../../include/ui/tjui.h" 
#include <commctrl.h>
using namespace tj::shared;
using namespace tj::shared::graphics;

// declared and used in tjui.cpp, but shouldn't be public
LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

LRESULT CALLBACK EditWndSubclassProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,UINT_PTR uIdSubclass,DWORD_PTR dwRefData) {
	// This is the same behaviour as in Wnd::Message
	if(uMsg==WM_KEYDOWN && LOWORD(wParam)==VK_TAB) {
		HWND root = GetAncestor(hWnd, GA_ROOT);
		HWND next = GetNextDlgTabItem(root, hWnd, !Wnd::IsKeyDown(KeyShift));
		SetFocus(next);
		return 0;
	}
	else if((uMsg==WM_CHAR || uMsg==WM_KEYUP) && LOWORD(wParam)==VK_TAB) {
		// consume
		return 0;
	}
	else if(uMsg==WM_SETFOCUS) {
		// Send message to parent (this is useful for PropertyGridWnd, for example
		HWND parent = ::GetParent(hWnd);
		SendMessage(parent, WM_PARENTNOTIFY, WM_SETFOCUS, 0);
	}
	
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

EditWnd::EditWnd(bool multiline): ChildWnd(L"", false), _backBrush(0) {
	SetStyle(WS_CLIPCHILDREN);
	SetStyleEx(WS_EX_CONTROLPARENT);

	long flags = WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL;
	if(multiline) {
		flags |= ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL;
	}

	_ctrl = CreateWindowEx(0, L"EDIT", L"", flags, 0, 0, 10, 10, GetWindow(), 0, GetModuleHandle(NULL), 0);
	_font = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
	
	SetWindowSubclass(_ctrl, EditWndSubclassProc, 1, 0);
	SendMessage(_ctrl, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);
	Layout();
}

EditWnd::~EditWnd() {
	RemoveWindowSubclass(_ctrl, EditWndSubclassProc, 1);
	DestroyWindow(_ctrl);
	DeleteObject(_font);
	DeleteObject(_backBrush);
}

void EditWnd::UpdateColor() {
	ref<Theme> theme = ThemeManager::GetTheme();
	Color nb = theme->GetColor(Theme::ColorEditBackground);
	if(nb.GetValue()!=_back.GetValue()||_backBrush==0) {
		_back = nb;
		if(_backBrush!=0) DeleteObject(_backBrush);
		_backBrush = CreateSolidBrush(RGB(_back.GetRed(), _back.GetGreen(), _back.GetBlue()));
	}
	
}

void EditWnd::Paint(graphics::Graphics& g, ref<Theme> theme) {
}

void EditWnd::Show(bool s) {
	if(s) {
		ShowWindow(_ctrl, SW_SHOW);
	}
	ChildWnd::Show(s);
}

void EditWnd::Layout() {
	Area rc = GetClientArea();
	ref<Theme> theme = ThemeManager::GetTheme();
	rc.MultiplyCeil(theme->GetDPIScaleFactor(), theme->GetDPIScaleFactor());
	SetWindowPos(_ctrl, 0L, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight(), SWP_NOZORDER);
}

void EditWnd::SetReadOnly(bool r) {
	SendMessage(_ctrl, EM_SETREADONLY, (WPARAM)(BOOL)r, 0);
}

void EditWnd::SetBorder(bool t) {
	long s = GetWindowLong(_ctrl, GWL_EXSTYLE);
	if(t) {
		s |= WS_EX_CLIENTEDGE;
	}
	else {
		s &= (~WS_EX_CLIENTEDGE);
	}
	SetWindowLong(_ctrl, GWL_EXSTYLE, s);
	Repaint();
}

LRESULT EditWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE) {
		Layout();
	}
	else if(msg==WM_CTLCOLOREDIT) {
		UpdateColor();
		/// SetBkMode(TRANSPARENT) doesn't seem to work with multiline edit controls
		///SetBkMode((HDC)wp, TRANSPARENT);

		Color back = ThemeManager::GetTheme()->GetColor(Theme::ColorBackground);
		SetBkColor((HDC)wp, RGB(back.GetRed(), back.GetGreen(), back.GetBlue()));

		Color text = ThemeManager::GetTheme()->GetColor(Theme::ColorText);
		SetTextColor((HDC)wp, RGB(text.GetRed(),text.GetGreen(),text.GetBlue()));
		
		return (LRESULT)(HBRUSH)_backBrush;
		
	}
	else if(msg==WM_COMMAND) {
		if(HIWORD(wp)==EN_CHANGE) {
			EventTextChanged.Fire(ref<Object>(this), NotificationTextChanged());
			UpdateWindow(_ctrl);
		}
		else {
			HWND parent = ::GetParent(GetWindow());
			if(parent!=0) {
				return SendMessage(parent, msg, wp, lp);
			}
		}
	}
	else if(msg==WM_PARENTNOTIFY) {
		if(wp==WM_SETFOCUS) {
			HWND parent = ::GetParent(GetWindow());
			SendMessage(parent, WM_PARENTNOTIFY, WM_SETFOCUS, 0);
		}
	}
	return ChildWnd::Message(msg,wp,lp);
}

void EditWnd::Focus() {
	OnFocus(true);
}

void EditWnd::OnFocus(bool f) {
	if(f) {
		SetFocus(_ctrl);
	}
}

bool EditWnd::HasFocus(bool childrenToo) const {
	return GetFocus() == _ctrl;
}

void EditWnd::SetCue(const std::wstring& cue) {
	SendMessage(_ctrl, EM_SETCUEBANNER, 0, (LPARAM)cue.c_str());
}

void EditWnd::SetText(const std::wstring& x) {
	SetWindowText(_ctrl, x.c_str());
}

std::wstring EditWnd::GetText() {
	int n = GetWindowTextLength(_ctrl);
	wchar_t* buffer = new wchar_t[n+2];
	GetWindowText(_ctrl, buffer, n+1);
	std::wstring txt(buffer);
	delete[] buffer;
	return txt;
}