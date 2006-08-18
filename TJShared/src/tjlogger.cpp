#include "../include/tjshared.h"
#include <windowsx.h>
using namespace tj::shared;

LoggerWnd::LoggerWnd(HWND parent): Wnd(L"Log", parent,TJ_DEFAULT_CLASS_NAME, false) {
	_list = CreateWindow(L"LISTBOX", L"LogListWnd", LBS_NOINTEGRALHEIGHT|WS_CHILD|WS_VISIBLE|LBS_DISABLENOSCROLL|(LBS_STANDARD&(~LBS_SORT)), 0, 0, 100, 100, _wnd, 0, GetModuleHandle(NULL), 0);

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

void LoggerWnd::Paint(Gdiplus::Graphics& g) {
}

void LoggerWnd::Log(std::wstring ws) {
	ListBox_AddString(_list, ws.c_str());
	//PostMessage(_list, LB_ADDSTRING, 0, ws.c_str());
}

wchar_t LoggerWnd::GetPreferredHotkey() {
	return L'L';
}

void LoggerWnd::Layout() {
	RECT rc; 
	GetClientRect(_wnd, &rc);
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