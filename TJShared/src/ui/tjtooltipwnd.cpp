#include "../../include/tjshared.h"
#include <commctrl.h>
using namespace tj::shared;

TooltipWnd::TooltipWnd(HWND owner) {
	_owner = owner;
	_wnd = CreateWindow(TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX,0, 0, 0, 0,NULL, NULL,GetModuleHandle(NULL),NULL);

	TOOLINFO ti;
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_TRANSPARENT | TTF_PARSELINKS |  TTF_TRACK | TTF_SUBCLASS | TTF_ABSOLUTE;
	ti.hwnd = _owner;
	ti.uId = 0;
	ti.hinst = NULL;
	ti.lpszText = L"Empty";

	GetClientRect(_owner, &ti.rect);
	SendMessage(_wnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

TooltipWnd::~TooltipWnd() {
	DestroyWindow(_wnd);
}

void TooltipWnd::SetTrackEnabled(bool t) {
	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = _owner;
	ti.uId = 0;
	SendMessage(_wnd, TTM_TRACKACTIVATE, (WPARAM)t?TRUE:FALSE, (LPARAM)&ti);
}

void TooltipWnd::SetTrackPosition(int x, int y) {
	SendMessage(_wnd, TTM_TRACKPOSITION, (WPARAM)0, MAKELONG(x, y));
}

void TooltipWnd::SetTooltip(std::wstring text) {
	TOOLINFO ti;
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_TRANSPARENT | TTF_PARSELINKS |  TTF_TRACK | TTF_SUBCLASS | TTF_ABSOLUTE;
	ti.hwnd = _owner;
	ti.uId = 0;
	ti.hinst = NULL;
	ti.lpszText = (wchar_t*)text.c_str();

	GetClientRect(_owner, &ti.rect);
	SendMessage(_wnd, TTM_SETTOOLINFO, 0, (LPARAM)&ti);
}