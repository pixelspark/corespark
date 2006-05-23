#include "../include/tjshared.h"

// declared and used in tjui.cpp, but shouldn't be public
LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

EditWnd::EditWnd(HWND parent): Wnd(L"", parent, L"EDIT", false) {
	UnsetStyle(0xFFFFFFFF);
	SetStyle(ES_AUTOHSCROLL);
	SetStyle(WS_CHILD);
	_font = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
	SendMessage(_wnd, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);
	Show(true);
	_proc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(_wnd, GWL_WNDPROC);
	SetWindowLongPtr(_wnd, GWLP_WNDPROC, (LONG)(LONG_PTR)WndProc);
}

EditWnd::~EditWnd() {
	DeleteObject(_font);
}

void EditWnd::Paint(Gdiplus::Graphics& g) {
}

wchar_t EditWnd::GetPreferredHotkey() {
	return L'E';
}

LRESULT EditWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	return 0;
}

LRESULT EditWnd::PreMessage(UINT msg, WPARAM wp, LPARAM lp) {
	if(_proc!=0) return _proc(_wnd, msg, wp, lp);
	return DefWindowProc(_wnd, msg, wp, lp);
}

void EditWnd::SetCue(std::wstring cue) {
	if(_proc!=0) _proc(_wnd, EM_SETCUEBANNER, 0, (LPARAM)cue.c_str());
}

std::wstring EditWnd::GetText() {
	int n = GetWindowTextLength(_wnd);
	wchar_t* buffer = new wchar_t[n+2];
	GetWindowText(_wnd, buffer, n+1);
	std::wstring txt(buffer);
	delete[] buffer;
	return txt;
}