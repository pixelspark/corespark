/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #include "../include/tjsharedui.h"

#ifdef TJ_OS_WIN
	#include <windowsx.h>
#endif

using namespace tj::shared;

LoggerWnd::LoggerWnd(): Wnd(null, false) {
	SetText(L"Log");
	_list = CreateWindow(L"LISTBOX", L"LogListWnd", LBS_NOINTEGRALHEIGHT|WS_CHILD|WS_VISIBLE|LBS_DISABLENOSCROLL|(LBS_STANDARD&(~LBS_SORT)), 0, 0, 100, 100, GetWindow(), 0, GetModuleHandle(NULL), 0);

	UnsetStyle(WS_CHILD);
	SetStyle(WS_OVERLAPPEDWINDOW);
	Move(200,200,400,500);
	_font = CreateFont(-10, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
	SendMessage(_list, WM_SETFONT, (WPARAM)_font, TRUE);
	Layout();
}

LoggerWnd::~LoggerWnd() {
	DeleteObject(_font);
}

void LoggerWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
}

void LoggerWnd::Log(const std::wstring& ws) {
	// This could be called from any thread; is that safe?
	ListBox_AddString(_list, ws.c_str());
}

void LoggerWnd::Layout() {
	RECT rc; 
	GetClientRect(GetWindow(), &rc);
	MoveWindow(_list, rc.left, rc.top, rc.right, rc.bottom, TRUE);
}

std::wstring LoggerWnd::GetContents() {
	int n = ListBox_GetCount(_list);
	std::wostringstream msgs;
	for(int a=0;a<n;a++) {
		int length = ListBox_GetTextLen(_list, a);
		wchar_t* buf = new wchar_t[length+2];
		buf[0] = L'\0';
		ListBox_GetText(_list, a, buf);
		msgs << buf << std::endl;
		delete[] buf;
	}

	return msgs.str();
}

LRESULT LoggerWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CLOSE) {
		Show(false);
		return 0;
	}
	else if(msg==WM_SIZE) {
		Layout();
	}

	return Wnd::Message(msg,wp,lp);
}