#include "../../include/ui/tjui.h" 
#include <windowsx.h>
using namespace tj::shared::graphics;
using namespace tj::shared;

SliderWnd::SliderWnd(const wchar_t* title): ChildWnd(title), _value(0.0f), _displayValue(0.0f), _mark(-1.0f), _showValue(true), _snapHalf(false), _preciseDrag(false), _color(0) {
	SetStyle(WS_TABSTOP);
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

float SliderWnd::GetMarkValue() const {
	return _mark;
}

float SliderWnd::GetDisplayValue() const {
	return _displayValue;
}

void SliderWnd::OnFocus(bool focus) {
	Repaint();
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

void SliderWnd::Fill(LayoutFlags flags, Area& r, bool direct) {
	if(flags==LayoutLeft) {
		if(direct) Move(r.GetLeft(), r.GetTop(), KMinimumWidth, r.GetHeight());
		r.Narrow(KMinimumWidth,0,0,0);
	}
	else if(flags==LayoutRight) {
		if(direct) Move(r.GetLeft()+r.GetWidth()-KMinimumWidth, r.GetTop(), KMinimumWidth, r.GetHeight());
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
			EventChanged.Fire(ref<Object>(this), NotificationChanged());
		}
	}
}

void SliderWnd::SetMarkValue(float v) {
	_mark = v;
}

void SliderWnd::SetValue(float f, bool notify) {
	if(_value!=f) {
		if(f<0.0f) f = 0.0f;
		if(f>1.0f) f = 1.0f;
		_value = f;
		
		if(notify) {
			Repaint();
			EventChanged.Fire(ref<Object>(this), NotificationChanged());
		}
	}
}

SliderWnd::~SliderWnd() {
}

void SliderWnd::Paint(Graphics& g, ref<Theme> theme) {
	EventUpdate.Fire(ref<Wnd>(this), NotificationUpdate());

	Pixels draggerWidth = theme->GetMeasureInPixels(Theme::MeasureSliderDraggerWidth);
	Area rect = GetClientArea();

	graphics::Color colorStart = theme->GetSliderColorStart((Theme::SliderType)_color);
	graphics::Color colorEnd = theme->GetSliderColorEnd((Theme::SliderType)_color);

	// background
	SolidBrush backBrush(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&backBrush,rect);

	// middle rectangle, 6 pixels wide
	rect.Narrow(0, 5, 0, _showValue ? KValueHeight : 10);

	LinearGradientBrush br(PointF((float)rect.GetLeft(), (float)rect.GetTop()), PointF((float)rect.GetRight(), (float)rect.GetTop()), colorStart, colorEnd);
	Pen pn(&br, 1.0f);
	const static int squareWidth = 6;
	int x = rect.GetWidth()/2 - (squareWidth/2);
	g.DrawRectangle(&pn, RectF(float(x), float(rect.GetTop()), float(squareWidth), float(rect.GetHeight())));

	if(_displayValue>0.0f) {
		float dvh = (1.0f-_displayValue) * rect.GetHeight();
		g.FillRectangle(&br, RectF(float(x+2), float(float(rect.GetTop())+dvh), float(squareWidth-3), float(rect.GetHeight())-dvh));
	}

	// markers
	int mx = (rect.GetWidth())/2 + (squareWidth/2);
	for(float my=0.0f;my<=1.0f;my+=0.1f) {
		float mty = float(int(rect.GetBottom()) - int(my*rect.GetHeight()));
		g.DrawLine(&pn, (REAL)mx, mty, (REAL)mx+2,mty);
	}

	if(_mark != _value && _mark <= 1.0f && _mark >= 0.0f) {
		mx = (rect.GetWidth())/2 - (squareWidth/2);
		Pen mpn(theme->GetColor(Theme::ColorCommandMarker), 3.0f);
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
	x = (rect.GetWidth())/2 - draggerWidth/2;
	int y = rect.GetBottom() - int(_value*int(rect.GetHeight()));
	SolidBrush border(colorEnd);

	Area dragger(x,y,draggerWidth, 6);
	Area draggerInside = dragger;
	draggerInside.Narrow(1,1,1,1);

	g.FillRectangle(&border, dragger);
	g.FillRectangle(&backBrush, draggerInside);
	
	if(HasFocus()) {
		LinearGradientBrush lbr(PointF(float(x+1), float(y)), PointF(float(x+1), float(y+6)), colorStart, colorEnd );
		g.FillRectangle(&lbr, draggerInside);
	}

	if(_showValue) {
		std::wostringstream os;
		os << int(_value*100);
		std::wstring msg = os.str();
		StringFormat sf;
		sf.SetAlignment(StringAlignmentCenter);
		SolidBrush tbr(theme->GetColor(Theme::ColorText));
		g.DrawString(msg.c_str(), (INT)msg.length(), theme->GetGUIFontSmall(), RectF(0.0f, float(rect.GetBottom())+5.0f, float(rect.GetRight()), 11.0f), &sf, &tbr);
	}
}

LRESULT SliderWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_MOUSEWHEEL) {
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
	return ChildWnd::Message(msg, wp, lp);
}

void SliderWnd::OnSize(const Area& ns) {
	Repaint();
}

void SliderWnd::OnKey(Key k, wchar_t ch, bool down, bool isaccel) {	
	if(k==KeyDown) {
		SetValue(_value - (1.0f/255.0f));
	}
	else if(k==KeyUp) {
		SetValue(_value+(1.0f/255.0f));
	}
	else if(k==KeyPageDown) {
		SetValue(0.0f);
	}
	else if(k==KeyPageUp) {
		SetValue(1.0f);
	}
	else if(k==KeyCharacter && (ch>=L'0'&& ch <= L'9') || ch == VK_OEM_3) {
		float v = (ch - L'0')*0.1f;
		if(ch==L'0') {
			v = 1.0f;
		}
		else if(ch==VK_OEM_3) {
			v = 0.0f;
		}

		SetValue(v);
	}
}

void SliderWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		Focus();
		_preciseDrag = IsKeyDown(KeyControl);
	}

	if(_preciseDrag) {
		if(ev==MouseEventLDown) {
			SetCapture(GetWindow());
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
		}
		else if(ev==MouseEventMove && IsKeyDown(KeyMouseLeft)) {
			Pixels dx = _startX - x;
			Pixels dy = _startY - y;

			float ratio = 1.0f;
			Area rc = GetClientArea();
			if(rc.GetHeight()!=0) {
				ratio = 1.0f / (rc.GetHeight()*15);
			}

			float val = _value + (dy * ratio);
			val = max(0.0f, val);
			val = min(1.0f, val);
			SetValue(val);
		}
		else if(ev==MouseEventLUp) {
			ReleaseCapture();
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}

		_startX = x;
		_startY = y;
	}
	else {
		if(ev==MouseEventMove ||ev==MouseEventLDown || ev==MouseEventRDown) {
			if(IsKeyDown(KeyMouseLeft) || IsKeyDown(KeyMouseRight)) {
				Area rc = GetClientArea();
				rc.Narrow(0,5, 0, _showValue ? KValueHeight : 10);

				y -= rc.GetTop();
				float val = float(y)/rc.GetHeight();
				
				if(_snapHalf && val<0.51f && val>0.49f) {
					SetValue(0.5f);
				}
				else {
					SetValue(1.0f - val);
				}
			}
		}
	}
}
