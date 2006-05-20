#include "../include/tjshared.h"

EditWnd::EditWnd(HWND parent): Wnd(L"", parent, L"EDIT", false) {
	UnsetStyle(0xFFFFFFFF);
	SetStyle(ES_AUTOHSCROLL);
	SetStyle(WS_CHILD);
	_font = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
	SendMessage(_wnd, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);
	Show(true);
}

EditWnd::~EditWnd() {
	DeleteObject(_font);
}

void EditWnd::Paint(Gdiplus::Graphics& g) {
}

wchar_t EditWnd::GetPreferredHotkey() {
	return L'E';
}

std::wstring EditWnd::GetText() {
	int n = GetWindowTextLength(_wnd);
	wchar_t* buffer = new wchar_t[n+2];
	GetWindowText(_wnd, buffer, n+1);
	std::wstring txt(buffer);
	delete[] buffer;
	return txt;
}