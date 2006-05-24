#include "../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

ButtonWnd::ButtonWnd(HWND parent, wchar_t hotkey, ref<Listener> listener, const wchar_t* image, const wchar_t* text): ChildWnd(L"", parent, false, true) {
	_hotkey = hotkey;
	_listener = listener;
	std::wstring fn = ResourceManager::Instance()->Get(image);
	_image =  Bitmap::FromFile(fn.c_str(),TRUE);
	Show(true);

	if(text!=0) {
		_text = text;
	}
}

ButtonWnd::~ButtonWnd() {
	delete _image;
}

void ButtonWnd::EnterHotkeyMode() {
	if(_listener) {
		_listener->Notify(this, NotificationClick);
	}
	Repaint();
}

void ButtonWnd::Paint(Graphics& g) {
	RECT rc;
	GetClientRect(_wnd,&rc);
	ref<Theme> theme = ThemeManager::GetTheme();
	
	Color col = theme->GetTimeBackgroundColor();
	if(IsMouseOver()) {
		if(ISVKKEYDOWN(VK_LBUTTON)) {
		col = theme->GetActiveStartColor();
		}
		else {
			col = theme->GetActiveEndColor();
		}
	}

	SolidBrush backBr(col);
	g.FillRectangle(&backBr, RectF(0.0f, 0.0f, REAL(rc.right-rc.left), REAL(rc.bottom-rc.top)));
	g.DrawImage(_image, RectF(0.0f, 0.0f, REAL(rc.bottom-rc.top), REAL(rc.bottom-rc.top)));

	Font* fnt = theme->GetGUIFontBold();
	SolidBrush textBrush(theme->GetTextColor());
	RectF lr(REAL(rc.bottom-rc.top), 0.0f, REAL(rc.right-rc.left-(rc.bottom-rc.top)), REAL(rc.bottom-rc.top));
	StringFormat sf;
	sf.SetLineAlignment(StringAlignmentCenter);
	
	g.DrawString(_text.c_str(), (unsigned int)_text.length(), fnt,lr, &sf, &textBrush);
}

LRESULT ButtonWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CREATE) {
		TRACKMOUSEEVENT evt;
		evt.cbSize = sizeof(evt);
		evt.dwFlags = TME_LEAVE;
		evt.dwHoverTime = 0;
		evt.hwndTrack = _wnd;
		TrackMouseEvent(&evt);
	}
	else if(msg==WM_LBUTTONDOWN) {
		Repaint();

		if(_listener) {
			_listener->Notify(this, NotificationClick);
			return 0;
		}
	}
	else if(msg==WM_LBUTTONUP) {
		Repaint();
	}
	else if(msg==WM_MOUSEMOVE) {
		TRACKMOUSEEVENT evt;
		evt.cbSize = sizeof(evt);
		evt.dwFlags = TME_LEAVE;
		evt.dwHoverTime = 0;
		evt.hwndTrack = _wnd;
		TrackMouseEvent(&evt);
		Repaint();
	}
	else if(msg==WM_MOUSELEAVE) {
		Repaint();
	}

	return ChildWnd::Message(msg,wp,lp);
}

wchar_t ButtonWnd::GetPreferredHotkey() {
	return _hotkey;
}

/* StateButtonWnd */
StateButtonWnd::StateButtonWnd(HWND parent, wchar_t hotkey, ref<Listener> listener, const wchar_t* imageOn, const wchar_t* imageOff, const wchar_t* imageOther):
ButtonWnd(parent,hotkey, listener, imageOn) {
	std::wstring fn = ResourceManager::Instance()->Get(imageOff);
	_offImage =  Bitmap::FromFile(fn.c_str(),TRUE);
	fn = ResourceManager::Instance()->Get(imageOn);
	_otherImage =  Bitmap::FromFile(fn.c_str(),TRUE);
	_on = Off;
}

StateButtonWnd::~StateButtonWnd() {
	delete _offImage;
	delete _otherImage;
}

void StateButtonWnd::Paint(Graphics& g) {
	RECT rc;
	GetClientRect(_wnd,&rc);

	ref<Theme> theme = ThemeManager::GetTheme();
	SolidBrush backBr(IsMouseOver()?theme->GetActiveEndColor():theme->GetTimeBackgroundColor());
	
	g.FillRectangle(&backBr, RectF(0.0f, 0.0f, REAL(rc.right-rc.left), REAL(rc.bottom-rc.top)));
	
	Bitmap* img = 0;
	switch(_on) {
		case On:
			img = _image;
			break;
		case Off:
		default:			
			img = _offImage;
			break;
		case Other:
			img = _otherImage;
	}
	
	if(img!=0) {
		g.DrawImage(img, RectF(0.0f, 0.0f, REAL(rc.right-rc.left), REAL(rc.bottom-rc.top)));
	}
}

LRESULT StateButtonWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_MOUSEMOVE) {
		TRACKMOUSEEVENT evt;
		evt.cbSize = sizeof(evt);
		evt.dwFlags = TME_LEAVE;
		evt.dwHoverTime = 0;
		evt.hwndTrack = _wnd;
		TrackMouseEvent(&evt);
		Repaint();
	}
	else if(msg==WM_MOUSELEAVE) {
		Repaint();
	}

	return ButtonWnd::Message(msg,wp,lp);
}

void StateButtonWnd::SetOn(ButtonState o) {
	_on = o;
	Repaint();
}
