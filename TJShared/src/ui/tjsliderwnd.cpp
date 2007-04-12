#include "../../include/tjshared.h"
#include <windowsx.h>
using namespace Gdiplus;
using namespace tj::shared;

#define ISVKKEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000))

const int SliderWnd::KDraggerWidth = 18;

SliderWnd::SliderWnd(HWND parent, const wchar_t* title): ChildWnd(title, parent) {
	_value = 0.0f;
	_hasFocus = false;
	_listener = 0;
	_displayValue = 0.0f;
	_flash = false;
	_oldValue = 0.0f;
	_mark = -1.0f;
	_showValue = true;
	_snapHalf = false;
}

void SliderWnd::SetShowValue(bool t) {
	_showValue = t;
}

void SliderWnd::SetSnapToHalf(bool s) {
	_snapHalf = s;
}

float SliderWnd::GetValue() const {
	return _value;
}

void SliderWnd::SetColor(int idx) {
	if(_color!=idx) {
		_color = idx;
		Repaint();
	}
}

void SliderWnd::Update() {
	Repaint();
}

void SliderWnd::Fill(LayoutFlags flags, Area& r) {
	if(flags==LayoutLeft) {
		Move(r.GetLeft(), r.GetTop(), KMinimumWidth, r.GetHeight());
		r.Narrow(KMinimumWidth,0,0,0);
	}
	else if(flags==LayoutRight) {
		Move(r.GetLeft()+r.GetWidth()-KMinimumWidth, r.GetTop(), KMinimumWidth, r.GetHeight());
		r.Narrow(0,0,KMinimumWidth,0);
	}
	else {
		ChildWnd::Fill(flags,r);
	}
}

void SliderWnd::SetDisplayValue(float v, bool notify) {
	if(_displayValue!=v) {
		_displayValue = v;

		if(notify) {
			Repaint();
			if(_listener!=0) {
				_listener->Notify(this, NotificationChanged);
			}
		}
	}
}

void SliderWnd::SetMarkValue(float v) {
	_mark = v;
	//Repaint();
}

void SliderWnd::SetValue(float f, bool notify) {
	if(_value!=f) {
		if(f<0.0f) f = 0.0f;
		if(f>1.0f) f = 1.0f;
		_value = f;
		
		if(notify) {
			Repaint();
			if(_listener!=0) {
				_listener->Notify(this, NotificationChanged);
			}
		}
	}
}

SliderWnd::~SliderWnd() {
}

void SliderWnd::Paint(Graphics& g) {
	if(_listener!=0) {
		_listener->Notify(this, NotificationUpdate);
	}
	
	ref<Theme> theme = ThemeManager::GetTheme();
	Area rect = GetClientArea();

	Gdiplus::Color colorStart = theme->GetSliderColorStart(_color);
	Gdiplus::Color colorEnd = theme->GetSliderColorEnd(_color);

	// background
	SolidBrush backBrush(theme->GetBackgroundColor());
	g.FillRectangle(&backBrush,rect);

	// middle rectangle, 6 pixels wide
	rect.Narrow(0, 5, 0, 25);

	LinearGradientBrush br(PointF(0.0f, float(rect.GetTop()-10)), PointF(0.0f, float(rect.GetHeight()+15)), colorStart, colorEnd);
	Pen pn(&br, 1.0f);
	const static int squareWidth = 6;
	int x = rect.GetWidth()/2 - (squareWidth/2);
	g.DrawRectangle(&pn, RectF(float(x), float(rect.GetTop()), float(squareWidth), float(rect.GetHeight())));

	if(_displayValue>0.0f) {
		float dvh = (1.0f-_displayValue) * rect.GetHeight();
		g.FillRectangle(&br,RectF(float(x+2), float(float(rect.GetTop())+dvh), float(squareWidth-4), float(rect.GetHeight())-dvh));
	}

	// markers
	int mx = (rect.GetWidth())/2 + (squareWidth/2);
	for(float my=0.0f;my<=1.0f;my+=0.1f) {
		float mty = float(int(rect.GetBottom()) - int(my*rect.GetHeight()));
		g.DrawLine(&pn, (REAL)mx, mty, (REAL)mx+2,mty);
	}

	if(_mark != _value && _mark <= 1.0f && _mark >= 0.0f) {
		mx = (rect.GetWidth())/2 - (squareWidth/2);
		Pen mpn(theme->GetCommandMarkerColor(), 3.0f);
		float mty = float(int(rect.GetBottom()) - int(_mark*rect.GetHeight()));
		g.DrawLine(&mpn, (REAL)mx, mty, (REAL)mx+squareWidth+1,mty);
	}

	// larger markers at 0.0, 0.5, 1.0
	float mty = float(int(rect.GetBottom()) - int(0.5f*int(rect.GetHeight())));
	g.DrawLine(&pn, (REAL)mx, mty, (REAL)mx+4,mty);
	mty = float( int(rect.GetBottom()) - int(1.0f*int(rect.GetHeight())));
	g.DrawLine(&pn, (REAL)mx, mty, (REAL)mx+4,mty);
	mty = float(int(rect.GetBottom()) - int(0.0f*int(rect.GetHeight())));
	g.DrawLine(&pn, (REAL)mx, mty, (REAL)mx+4,mty);

	// dragger
	const static int draggerWidth = KDraggerWidth; // TODO make class constant
	x = (rect.GetWidth())/2 - KDraggerWidth/2;
	int y = rect.GetBottom() - int(_value*int(rect.GetHeight()));
	SolidBrush border(colorEnd);
	g.FillRectangle(&border, RectF(float(x), float(y), float(draggerWidth), 6.0f));
	g.FillRectangle(&backBrush, RectF(float(x+1), float(y+1), float(draggerWidth-2), 4.0f));
	
	if(_hasFocus||_flash) {
		LinearGradientBrush lbr(PointF(float(x+1), float(y)), PointF(float(x+1), float(y+6)), colorStart, colorEnd );
		g.FillRectangle(&lbr, RectF(float(x+1), float(y+1), float(draggerWidth-2), 4.0f));
	}

	if(_showValue) {
		std::wostringstream os;
		os << int(_value*100);
		std::wstring msg = os.str();
		StringFormat sf;
		sf.SetAlignment(StringAlignmentCenter);
		SolidBrush tbr(theme->GetTextColor());
		g.DrawString(msg.c_str(), (INT)msg.length(), theme->GetGUIFontSmall(), RectF(0.0f, float(rect.GetBottom())+5.0f, float(rect.GetRight()), 11.0f), &sf, &tbr);
	}
}

LRESULT SliderWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CONTEXTMENU) {
		msg = WM_RBUTTONDOWN;
	}
	else if(msg==WM_KEYUP || msg==WM_RBUTTONUP) {
		if(_flash) {
			SetValue(_oldValue);
		}
		_flash = false;
	}
	else if(msg==WM_KEYDOWN) {
		if(ISVKKEYDOWN(VK_CONTROL)) {
			if(!_flash) {
				_flash = true;	
				_oldValue = _value;
			}
		}
		
		if(wp==VK_DOWN) {
			SetValue(_value - (1.0f/255.0f));
		}
		else if(wp==VK_UP) {
			SetValue(_value+(1.0f/255.0f));
		}
		else if(wp==VK_NEXT) {
			SetValue(0.0f);
		}
		else if(wp==VK_PRIOR) {
			SetValue(1.0f);
		}
		else if(wp>=L'0'&& wp <= L'9' || wp == VK_OEM_3) {
			float v = (wp - L'0')*0.1f;
			if(wp==L'0') {
				v = 1.0f;
			}
			else if(wp==VK_OEM_3) {
				v = 0.0f;
			}

			SetValue(v);
		}
		else {
			HWND first = ::GetWindow(GetWindow(), GW_HWNDFIRST);
			HWND last = ::GetWindow(GetWindow(), GW_HWNDLAST);

			if(wp==VK_LEFT) {
				if(GetWindow()==last) {
					SetFocus(first);
				}
				else {
					HWND next = ::GetWindow(GetWindow(), GW_HWNDNEXT);
					SetFocus(next);
				}
			}
			else if(wp==VK_RIGHT) {
				if(GetWindow()==first) {
					SetFocus(last);
				}
				else {
					HWND next = ::GetWindow(GetWindow(), GW_HWNDPREV);
					SetFocus(next);
				}
			}
		}
	}
	else if(msg==WM_MOUSEWHEEL) {
		int delta = GET_WHEEL_DELTA_WPARAM(wp);
		if(delta<0) {
			SetValue(max(0.0f, _value - 0.05f));
			Repaint();
		}
		else {
			SetValue(min(1.0f,_value+0.05f));
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

void SliderWnd::SetListener(Listener* listener) {
	_listener = listener;
}

void SliderWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventMove ||ev==MouseEventLDown || ev==MouseEventRDown) {
		if(ISVKKEYDOWN(VK_LBUTTON) || ISVKKEYDOWN(VK_RBUTTON)) {
			Area rc = GetClientArea();
			rc.Narrow(0,5, 0, _showValue?10:25);

			y -= rc.GetTop();
			float val = float(y)/rc.GetHeight();

			if(ev==MouseEventRDown) {
				if(!_flash) {
					_oldValue = _value;
					_flash = true;
				}
			}
			
			if(_snapHalf && val<0.51f && val>0.49f) {
				SetValue(0.5f);
			}
			else {
				SetValue(1.0f - val);
			}
		}
	}
}