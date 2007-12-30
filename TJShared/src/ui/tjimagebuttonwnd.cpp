#include "../../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

ButtonWnd::ButtonWnd(const wchar_t* image, const wchar_t* text): ChildWnd(L"", false, true) {
	SetWantMouseLeave(true);
	std::wstring fn = ResourceManager::Instance()->Get(image);
	_image =  Bitmap::FromFile(fn.c_str(),TRUE);
	_down = false;

	if(text!=0) {
		_text = text;
	}
}

ButtonWnd::~ButtonWnd() {
	delete _image;
}

void ButtonWnd::SetListener(ref<Listener> lr) {
	_listener = lr;
}

void ButtonWnd::Paint(Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();

	// TODO: make changeable at runtime
	static const Pixels KImgHeight = 16;
	static const Pixels KImgWidth = 16;

	// Fill background
	LinearGradientBrush backGradient(PointF(0.0f, 0.0f), PointF(0.0f, (float)rc.GetHeight()), theme->GetActiveStartColor(), theme->GetActiveEndColor());
	LinearGradientBrush backGradientReverse(PointF(0.0f, (float)rc.GetHeight()), PointF(0.0f, 0.0f), theme->GetActiveStartColor(), theme->GetActiveEndColor());
	SolidBrush disabledBrush(theme->GetDisabledOverlayColor());

	if(_down) {
		g.FillRectangle(&backGradient, rc);
	}
	else {
		g.FillRectangle(&backGradientReverse, rc);
		if(!IsMouseOver()) {
			g.FillRectangle(&disabledBrush, rc);
		}
	}
	
	// Draw icon
	if(_image!=0) {
		Pixels margin = (rc.GetHeight()-KImgHeight)/2;
		g.DrawImage(_image, Area(rc.GetLeft()+margin, rc.GetTop()+margin, KImgWidth, KImgHeight));
	}
	
	// Draw border
	SolidBrush border(theme->GetActiveStartColor());
	rc.Narrow(0,0,1,1);
	Pen borderPen(&border,1.0f);
	g.DrawRectangle(&borderPen, rc);

	// Draw text
	Font* fnt = theme->GetGUIFontBold();
	SolidBrush textBrush(theme->GetTextColor());
	StringFormat sf;
	sf.SetLineAlignment(StringAlignmentNear);
	g.DrawString(_text.c_str(), (unsigned int)_text.length(), fnt,PointF(KImgWidth+5, 2.0f), &sf, &textBrush);
}

void ButtonWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		_down = true;
		Repaint();
	}
	else if(ev==MouseEventLUp) {
		ref<Listener> listener = _listener;
		if(listener) {
			listener->Notify(this, NotificationClick);
		}

		_down = false;
		Repaint();
	}
	else if(ev==MouseEventMove) {
		if(ISVKKEYDOWN(VK_LBUTTON)) {
			_down = true;
		}
		Repaint();
	}
	else if(ev==MouseEventLeave) {
		_down = false;
		Repaint();
	}
	ChildWnd::OnMouse(ev,x,y);
}

LRESULT ButtonWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	return ChildWnd::Message(msg,wp,lp);
}

/* StateButtonWnd */
StateButtonWnd::StateButtonWnd( const wchar_t* imageOn, const wchar_t* imageOff, const wchar_t* imageOther):
ButtonWnd(imageOn) {
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

void StateButtonWnd::Paint(Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();

	SolidBrush backBr(IsMouseOver()?theme->GetActiveEndColor():theme->GetTimeBackgroundColor());
	g.FillRectangle(&backBr, rc);
	
	// Choose and draw icon
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
		// TODO make changeable at runtime
		static const Pixels KImgHeight = 16;
		static const Pixels KImgWidth = 16;

		Pixels margin = (rc.GetHeight()-KImgHeight)/2;
		g.DrawImage(img, Area(margin,margin,KImgHeight, KImgWidth));
	}
}

LRESULT StateButtonWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_MOUSEMOVE) {
		TRACKMOUSEEVENT evt;
		evt.cbSize = sizeof(evt);
		evt.dwFlags = TME_LEAVE;
		evt.dwHoverTime = 0;
		evt.hwndTrack = GetWindow();
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
