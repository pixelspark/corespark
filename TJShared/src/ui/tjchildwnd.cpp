#include "../../include/ui/tjui.h" 
using namespace tj::shared::graphics;
using namespace tj::shared;

ChildWnd::ChildWnd(const wchar_t* title, bool useDB): Wnd(title, 0L, TJ_DEFAULT_CLASS_NAME, useDB) {
	SetWindowLong(GetWindow(), GWL_STYLE, WS_CHILD);
	SetWindowLong(GetWindow(), GWL_EXSTYLE, 0);

	SetStyle(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_TABSTOP);
}

void ChildWnd::SetTabStop(bool ts) {
	if(ts) {
		SetStyle(WS_TABSTOP);
	}
	else {
		UnsetStyle(WS_TABSTOP);
	}
}

bool ChildWnd::GetTabStop() {
	return (GetWindowLong(GetWindow(), GWL_STYLE) & WS_TABSTOP) != 0;
}

CheckboxWnd::CheckboxWnd(): ChildWnd(L""), _readOnly(false), _checkedIcon(Icons::GetIconPath(Icons::IconChecked)), _checked(false) {
	_checkAnimation.SetLength(750);
	_checkAnimation.SetEase(Animation::EasePulse);
}

CheckboxWnd::~CheckboxWnd() {
}

bool CheckboxWnd::IsChecked() const {
	return _checked;
}

void CheckboxWnd::SetChecked(bool t) {
	if(t && !_checked) {
		_checkAnimation.Start();
		StartTimer(Time(50),1);
	}
	_checked = t;
	Repaint();
}

void CheckboxWnd::SetReadOnly(bool r) {
	_readOnly = r;
}

bool CheckboxWnd::IsReadOnly() const {
	return _readOnly;
}

void CheckboxWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		Focus();
		if(!IsReadOnly()) {
			SetChecked(!_checked);
		}
		Repaint();
	}
	else if(ev==MouseEventMove) {
		SetWantMouseLeave(true);
		Repaint();
	}
	else if(ev==MouseEventLeave) {
		Repaint();
	}
}

void CheckboxWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	SolidBrush back(theme->GetColor(Theme::ColorBackground));
	Area rc = GetClientArea();
	g.FillRectangle(&back, rc);
	bool down = IsKeyDown(KeyMouseLeft);

	Pixels margin = (rc.GetHeight()-16)/2;
	LinearGradientBrush lbr(PointF(0.0f, (float)rc.GetTop()), PointF(0.0f, (float)rc.GetBottom()), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
	Pen borderPen(&lbr, 1.0f);

	Area rect(rc.GetLeft()+margin, rc.GetTop()+margin, 16, 16);
	Area borderRect = rect;
	borderRect.Narrow(2,2,2,2);

	theme->DrawInsetRectangleLight(g, borderRect);
	if(HasFocus()) {
		theme->DrawToolbarBackground(g, borderRect);
	}

	g.DrawRectangle(&borderPen, borderRect);
	if(_checked) {
		g.DrawImage(_checkedIcon, rect);
	}

	if(_checkAnimation.IsAnimating()) {
		Area animRect = rect;
		animRect.Widen(4,4,4,4);
		theme->DrawHighlightEllipse(g, animRect, _checkAnimation.GetFraction());
	}

	if(IsReadOnly()) {
		SolidBrush dbr(theme->GetColor(Theme::ColorDisabledOverlay));
		g.FillRectangle(&dbr,rc);
	}
}

void CheckboxWnd::OnTimer(unsigned int id) {
	if(!_checkAnimation.IsAnimating()) {
		StopTimer(id);
	}
	Repaint();
}

void CheckboxWnd::OnSize(const Area& ns) {
	Repaint();
}

void CheckboxWnd::OnFocus(bool f) {
	Repaint();
}

void CheckboxWnd::OnKey(Key k, wchar_t ch, bool down, bool isAccelerator) {
	if(!isAccelerator && down && (k==KeyLeft || k==KeyRight || ch==L' ') && !IsReadOnly()) {
		SetChecked(!_checked);
	}
}