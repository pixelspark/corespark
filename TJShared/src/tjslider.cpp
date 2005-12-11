#include "../include/tjshared.h"
using namespace Gdiplus;
#include <iomanip>

#define ISVKKEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000))

SliderWnd::SliderWnd(HWND parent, const wchar_t* title): ChildWnd(title, parent) {
	_value = 0.0f;
	_hotkey = L'S';
	_bitmap = 0;
	_hasFocus = false;
	_listener = 0;
}

float SliderWnd::GetValue() const {
	return _value;
}

void SliderWnd::Update() {
	Repaint();
}

void SliderWnd::SetValue(float f, bool notify) {
	if(f<0.0f) f = 0.0f;
	if(f>1.0f) f = 1.0f;
	_value = f;
	Repaint();

	if(notify && _listener!=0) {
		_listener->Notify(this, NotificationChanged);
	}
}

SliderWnd::~SliderWnd() {
	delete _bitmap;
}

LRESULT SliderWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_PAINT) {
		if(_listener!=0) {
			_listener->Notify(this, NotificationUpdate);
		}

		PAINTSTRUCT ps;
		BeginPaint(_wnd, &ps);
		RECT rect;
		GetClientRect(_wnd, &rect);

		Graphics org(ps.hdc);
		if(_bitmap==0 || int(_bitmap->GetWidth())!=rect.right-rect.left || int(_bitmap->GetHeight())!=rect.bottom-rect.top) {
			delete _bitmap;
			_bitmap = new Bitmap(rect.right-rect.left, rect.bottom-rect.top, &org);
		}
		Graphics g(_bitmap);
		
		ref<Theme> theme = ThemeManager::GetTheme();

		// background
		SolidBrush backBrush(theme->GetBackgroundColor());
		g.FillRectangle(&backBrush,Rect(0,0,rect.right-rect.left, rect.bottom-rect.top));

		// middle rectangle, 6 pixels wide
		rect.top += 5;
		rect.bottom -= 60;
		LinearGradientBrush br(PointF(0.0f, float(rect.top-10)), PointF(0.0f, float(rect.bottom-rect.top+15)), theme->GetActiveStartColor(), theme->GetActiveEndColor());
		Pen pn(&br, 1.0f);
		const static int squareWidth = 6;
		int x = (rect.right-rect.left)/2 - (squareWidth/2);
		g.DrawRectangle(&pn, RectF(float(x), float(rect.top), float(squareWidth), float(rect.bottom-rect.top)));

		// dragger
		const static int draggerWidth = 22;
		x = (rect.right-rect.left)/2 - draggerWidth/2;
		int y = rect.bottom - int(_value*int(rect.bottom-rect.top));
		SolidBrush border(theme->GetActiveEndColor());
		g.FillRectangle(&border, RectF(float(x), float(y), float(draggerWidth), 6.0f));
		g.FillRectangle(&backBrush, RectF(float(x+1), float(y+1), float(draggerWidth-2), 4.0f));
		
		if(_hasFocus) {
			LinearGradientBrush lbr(PointF(float(x+1), float(y)), PointF(float(x+1), float(y+6)), theme->GetActiveStartColor(), theme->GetActiveEndColor() );
			g.FillRectangle(&lbr, RectF(float(x+1), float(y+1), float(draggerWidth-2), 4.0f));
		}

		std::wostringstream os;
		os << int(_value*100) << L'%';
		std::wstring msg = os.str();
		StringFormat sf;
		sf.SetAlignment(StringAlignmentCenter);
		SolidBrush tbr(theme->GetTextColor());
		g.DrawString(msg.c_str(), (INT)msg.length(), theme->GetGUIFont(), RectF(0.0f, float(rect.bottom+15), float(rect.right), 16.0f), &sf, &tbr);

		org.DrawImage(_bitmap, PointF(0.0f, 0.0f));
		EndPaint(_wnd, &ps);
	}
	else if(msg==WM_MOUSEMOVE || msg==WM_LBUTTONDOWN) {
		if(ISVKKEYDOWN(VK_LBUTTON)) {
			int y = GET_Y_LPARAM(lp);
			RECT rc;
			GetClientRect(_wnd, &rc);
			rc.top += 5;
			rc.bottom -= 60;

			y -= rc.top;
			float val = float(y)/float(rc.bottom-rc.top);
			SetValue(1.0f - val);
		}
	}
	else if(msg==WM_KEYDOWN) {
		if(wp==VK_DOWN) {
			SetValue(_value - 0.05f);
		}
		else if(wp==VK_UP) {
			SetValue(_value+0.05f);
		}
	}
	else if(msg==WM_MOUSEWHEEL) {
		int delta = GET_WHEEL_DELTA_WPARAM(wp);
		if(delta<0) {
			_value = max(0.0f, _value - 0.05f);
			Repaint();
		}
		else {
			_value = min(1.0f,_value+0.05f);
			Repaint();
		}
	}
	else if(msg==WM_SETFOCUS) {
		_hasFocus = true;
		Repaint();
	}
	else if(msg==WM_KILLFOCUS) {
		_hasFocus = false;
		Repaint();
	}

	return ChildWnd::Message(msg, wp, lp);
}

wchar_t SliderWnd::GetPreferredHotkey() {
	return _hotkey;
}

void SliderWnd::SetHotkey(wchar_t hotkey) {
	_hotkey = hotkey;
}

void SliderWnd::SetListener(Listener* listener) {
	_listener = listener;
}