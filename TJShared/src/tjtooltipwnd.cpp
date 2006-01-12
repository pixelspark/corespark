#include "../include/tjshared.h"

TooltipWnd::TooltipWnd(HWND parent): Wnd(L"Tooltips", parent, TOOLTIPS_CLASS, false) {
	UnsetStyle(WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	SetStyle(WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX);
	SetWindowPos(_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	Fill();
	Show(true);
}

TooltipWnd::~TooltipWnd() {
}

void TooltipWnd::AddTooltip(HWND wnd, std::wstring text, int x, int y, int w, int h, int uid) {
	TOOLINFO inf;
	memset(&inf, 0, sizeof(TOOLINFO));
	inf.cbSize = sizeof(TOOLINFO);
	inf.uFlags = TTF_SUBCLASS;
	inf.hinst = GetModuleHandle(NULL);
	inf.hwnd = wnd;
	inf.lpszText = (LPWSTR)text.c_str();
	inf.rect.left = x;
	inf.rect.top = y;
	inf.rect.right = x+w;
	inf.rect.bottom = y+h;
	//GetClientRect(wnd, &inf.rect);
	inf.uId = uid;

	//SendMessage(_wnd, TTM_ADDTOOL, 0, (LPARAM)&inf);
}

void TooltipWnd::Fill() {
}

void TooltipWnd::Layout() {
}

wchar_t TooltipWnd::GetPreferredHotkey() {
	return L'\0';
}

void TooltipWnd::Paint(Gdiplus::Graphics& g) {
}

void TooltipWnd::RemoveTooltip(HWND wnd, int uid) {
	TOOLINFO inf;
	memset(&inf, 0, sizeof(TOOLINFO));
	inf.cbSize = sizeof(TOOLINFO);
	inf.hinst = GetModuleHandle(NULL);
	inf.hwnd = wnd;
	inf.uId = uid;

	SendMessage(_wnd, TTM_DELTOOL, 0, (LPARAM)&inf);
}
