#include "../../include/ui/tjui.h" 
#include <commctrl.h>
using namespace tj::shared;
using namespace tj::shared::graphics;

// declared and used in tjui.cpp, but shouldn't be public
LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

EditWnd::EditWnd(): ChildWnd(L"", false, false), _backBrush(0) {
	SetStyle(WS_CLIPCHILDREN);
	SetStyleEx(WS_EX_CONTROLPARENT);

	_ctrl = CreateWindowEx(0, L"EDIT", L"", WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL, 0, 0, 10, 10, GetWindow(), 0, GetModuleHandle(NULL), 0);
	_font = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
	SendMessage(_ctrl, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);
	ShowWindow(_ctrl, SW_SHOW);
	Layout();
}

EditWnd::~EditWnd() {
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

void EditWnd::Layout() {
	Area rc = GetClientArea();
	ref<Theme> theme = ThemeManager::GetTheme();
	rc.MultiplyCeil(theme->GetDPIScaleFactor(), theme->GetDPIScaleFactor());
	SetWindowPos(_ctrl, 0L, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight(), SWP_NOZORDER);
}

void EditWnd::SetReadOnly(bool r) {
	SendMessage(_ctrl, EM_SETREADONLY, (WPARAM)(BOOL)r, 0);
}

void EditWnd::SetMultiline(bool t) {
	long s = GetWindowLong(_ctrl, GWL_STYLE);
	if(t) {
		s |= ES_MULTILINE;
	}
	else {
		s &= (~ES_MULTILINE);
	}
	SetWindowLong(_ctrl, GWL_STYLE, s);
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
		SetBkMode((HDC)wp, TRANSPARENT);
		Color text = ThemeManager::GetTheme()->GetColor(Theme::ColorText);
		SetTextColor((HDC)wp, RGB(text.GetRed(),text.GetGreen(),text.GetBlue()));
		return (LRESULT)(HBRUSH)_backBrush;
		
	}
	else if(msg==WM_COMMAND) {
		if(HIWORD(wp)==EN_CHANGE) {
			EventTextChanged.Fire(this, NotificationTextChanged());
		}
		else {
			HWND parent = ::GetParent(GetWindow());
			if(parent!=0) {
				return SendMessage(parent, msg, wp, lp);
			}
		}
	}
	return ChildWnd::Message(msg,wp,lp);
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

void EditWnd::SetText(std::wstring x) {
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