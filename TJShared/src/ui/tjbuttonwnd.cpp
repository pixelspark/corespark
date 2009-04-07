#include "../../include/ui/tjui.h" 
using namespace tj::shared::graphics;
using namespace tj::shared;

ButtonWnd::ButtonWnd(const ResourceIdentifier& icon, const std::wstring& text): _disabled(false), _down(false), _icon(icon), _text(text) {
	SetWantMouseLeave(true);
}

ButtonWnd::~ButtonWnd() {
}

void ButtonWnd::Fill(LayoutFlags f, Area& r, bool direct) {
	strong<Theme> theme = ThemeManager::GetTheme();
	Pixels h = theme->GetMeasureInPixels(Theme::MeasureToolbarHeight);
	if(f==LayoutTop) {
		if(direct) Move(r.GetLeft(), r.GetTop(), r.GetWidth(), h);
		r.Narrow(0,h,0,0);
	}
	else if(f==LayoutBottom) {
		if(direct) Move(r.GetLeft(), (r.GetTop()+r.GetHeight())-h, r.GetWidth(), h);
		r.Narrow(0,0,0,h);
	}
	else {
		ChildWnd::Fill(f, r, direct);
	}
}

void ButtonWnd::OnSize(const Area& ns) {
	Repaint();
	ChildWnd::OnSize(ns);
}

void ButtonWnd::SetDisabled(bool d) {
	_disabled = d;
	Repaint();
}

bool ButtonWnd::IsDisabled() const {
	return _disabled;
}


void ButtonWnd::SetText(const wchar_t* t) {
	_text = t;
	Repaint();
}

void ButtonWnd::OnKey(Key k, wchar_t t, bool down, bool isAccelerator) {
	if(!isAccelerator) {
		if(k==KeyCharacter && t==L' ') {
			if(down) {
				_down = true;
			}
			else {
				if(!_disabled) {
					EventClicked.Fire(ref<Object>(this), NotificationClicked());
				}
				_down = false;
			}
			Repaint();
		}
	}
}

void ButtonWnd::OnFocus(bool f) {
	Repaint();
}

void ButtonWnd::Paint(Graphics& g, strong<Theme> theme) {
	Area rc = GetClientArea();

	// TODO: make changeable at runtime
	static const Pixels KImgHeight = 16;
	static const Pixels KImgWidth = 16;

	// Fill background
	LinearGradientBrush backGradient(PointF(0.0f, 0.0f), PointF(0.0f, (float)rc.GetHeight()), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
	LinearGradientBrush shadowGradient(PointF(0.0f, (float)rc.GetHeight()-1), PointF(0.0f, 0.0f), Color(0,0,0,0), Color(50,0,0,0));
	LinearGradientBrush highlightGradient(PointF(0.0f, (float)rc.GetHeight()/2), PointF(0.0f, 0.0f), Color(0,255,255,255), Color(50,255,255,255));
	SolidBrush disabledBrush(theme->GetColor(Theme::ColorDisabledOverlay));

	g.FillRectangle(&backGradient, rc);
	Area shadowRC = rc;
	shadowRC.Narrow(0,0,0,rc.GetHeight()/2);
	g.FillRectangle(&disabledBrush, rc);

	if(_down) {
		g.FillRectangle(&shadowGradient, rc);
	}
	else {
		g.FillRectangle(&highlightGradient, shadowRC);
	}
	
	// Draw icon
	Pixels margin = (rc.GetHeight()-KImgHeight)/2;
	Area iconArea(rc.GetLeft()+margin, rc.GetTop()+margin, KImgWidth, KImgHeight);
	theme->DrawHighlightEllipse(g, iconArea);
	_icon.Paint(g, iconArea);
	
	// Draw border
	SolidBrush border(theme->GetColor(Theme::ColorActiveEnd));
	rc.Narrow(0,0,1,1);
	Pen borderPen(&border,1.0f);
	g.DrawRectangle(&borderPen, rc);

	// Draw second border
	SolidBrush secondBorder(theme->GetColor(Theme::ColorTimeBackground));
	rc.Narrow(0,0,1,1);
	Pen secondBorderPen(&secondBorder, 1.0f);
	g.DrawRectangle(&secondBorderPen, rc);

	// Draw text
	Font* fnt = theme->GetGUIFontBold();
	SolidBrush textBrush(theme->GetColor(Theme::ColorText));
	SolidBrush shadowBrush(theme->GetColor(Theme::ColorBackground));
	StringFormat sf;
	sf.SetLineAlignment(StringAlignmentNear);
	sf.SetLineAlignment(StringAlignmentCenter);
	sf.SetTrimming(StringTrimmingEllipsisPath);
	sf.SetFormatFlags(StringFormatFlagsLineLimit);

	g.DrawString(_text.c_str(), (unsigned int)_text.length(), fnt,RectF(float(rc.GetLeft()+margin+KImgWidth+margin)+1.0f, (float)rc.GetTop()+1.0f, (float)rc.GetRight(), (float)rc.GetBottom()), &sf, &shadowBrush);
	g.DrawString(_text.c_str(), (unsigned int)_text.length(), fnt,RectF(float(rc.GetLeft()+margin+KImgWidth+margin), (float)rc.GetTop(), (float)rc.GetRight(), (float)rc.GetBottom()), &sf, &textBrush);

	if(_disabled) {
		g.FillRectangle(&disabledBrush, rc);
	}
}

void ButtonWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		Focus();
		_down = true;
		Repaint();
	}
	else if(ev==MouseEventLUp) {
		if(!_disabled) {
			EventClicked.Fire(ref<Object>(this), NotificationClicked());
		}

		_down = false;
		Repaint();
	}
	else if(ev==MouseEventMove) {
		SetWantMouseLeave(true);
		if(IsKeyDown(KeyMouseLeft)) {
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

/* StateButtonWnd */
StateButtonWnd::StateButtonWnd(const ResourceIdentifier& imageOn, const ResourceIdentifier& imageOff, const ResourceIdentifier& imageOther): ButtonWnd(imageOn), _offIcon(imageOff), _otherIcon(imageOther) {
	_on = Off;
}

StateButtonWnd::~StateButtonWnd() {
}

void StateButtonWnd::Paint(Graphics& g, strong<Theme> theme) {
	Area rc = GetClientArea();

	SolidBrush backBr(IsMouseOver()?theme->GetColor(Theme::ColorActiveEnd):theme->GetColor(Theme::ColorTimeBackground));
	g.FillRectangle(&backBr, rc);
	
	// Choose and draw icon
	Icon* img = 0;
	switch(_on) {
		case On:
			img = &_icon;
			break;
		case Off:
		default:			
			img = &_offIcon;
			break;
		case Other:
			img = &_otherIcon;
	}
	
	if(img!=0) {
		// TODO make changeable at runtime
		static const Pixels KImgHeight = 16;
		static const Pixels KImgWidth = 16;

		Pixels margin = (rc.GetHeight()-KImgHeight)/2;
		img->Paint(g, Area(margin,margin,KImgHeight, KImgWidth));
	}
}

void StateButtonWnd::SetOn(ButtonState o) {
	_on = o;
	Repaint();
}
