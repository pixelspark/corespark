#include "../../include/tjshared.h"
#include <commctrl.h>
using namespace tj::shared;
using namespace Gdiplus;

// declared and used in tjui.cpp, but shouldn't be public
LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

EditWnd::EditWnd(): ChildWnd(L"", false, false) {
	SetStyle(WS_CLIPCHILDREN);
	SetStyleEx(WS_EX_CONTROLPARENT);
	_ctrl = CreateWindowEx(0, L"EDIT", L"", WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL, 0, 0, 10, 10, GetWindow(), 0, GetModuleHandle(NULL), 0);
	_font = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
	SendMessage(_ctrl, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);
	ShowWindow(_ctrl, SW_SHOW);
	Layout();
	_backBrush = 0;
}

EditWnd::~EditWnd() {
	DestroyWindow(_ctrl);
	DeleteObject(_font);
	DeleteObject(_backBrush);
}

void EditWnd::UpdateColor() {
	ref<Theme> theme = ThemeManager::GetTheme();
	Color nb = theme->GetEditBackgroundColor();
	if(nb.GetValue()!=_back.GetValue()||_backBrush==0) {
		_back = nb;
		if(_backBrush!=0) DeleteObject(_backBrush);
		_backBrush = CreateSolidBrush(RGB(_back.GetRed(), _back.GetGreen(), _back.GetBlue()));
	}
	
}

void EditWnd::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
}

void EditWnd::Layout() {
	Area rc = GetClientArea();
	ref<Theme> theme = ThemeManager::GetTheme();
	rc.MultiplyCeil(theme->GetDPIScaleFactor(), theme->GetDPIScaleFactor());
	SetWindowPos(_ctrl, 0L, rc.GetLeft(), rc.GetTop(), rc.GetWidth(), rc.GetHeight(), SWP_NOZORDER);
}

LRESULT EditWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_SIZE) {
		Layout();
	}
	else if(msg==WM_CTLCOLOREDIT) {
		UpdateColor();
		SetBkMode((HDC)wp, TRANSPARENT);
		Color text = ThemeManager::GetTheme()->GetTextColor();
		SetTextColor((HDC)wp, RGB(text.GetRed(),text.GetGreen(),text.GetBlue()));
		return (LRESULT)(HBRUSH)_backBrush;
		
	}
	else if(msg==WM_COMMAND) {
		if(_listener && HIWORD(wp)==EN_CHANGE) {
			_listener->Notify(this, NotificationChanged);
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

void EditWnd::SetListener(ref<Listener> ls) {
	_listener = ls;
}

void EditWnd::SetCue(std::wstring cue) {
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