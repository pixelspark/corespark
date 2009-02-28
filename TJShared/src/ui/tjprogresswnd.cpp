#include "../../include/ui/tjui.h" 

using namespace tj::shared::graphics;
using namespace tj::shared;
#include <math.h>

ProgressWnd::ProgressWnd(): ChildWnd(L"") {
	_indeterminate = false;
	_value = 0.0f;
}

ProgressWnd::~ProgressWnd() {
}

void ProgressWnd::OnTimer(unsigned int id) {
	Update();
}

void ProgressWnd::Update() {
	if(_indeterminate) {
		_value += 0.005f;
		if(_value>1.0f) _value = -1.0f;
		Repaint();
	}
}

void ProgressWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	float v = fabs(_value);
	
	Area rc = GetClientArea();	
	LinearGradientBrush back(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetColor(Theme::ColorProgressBackgroundStart), theme->GetColor(Theme::ColorProgressBackgroundEnd));
	g.FillRectangle(&back, rc);

	Pen border(theme->GetColor(Theme::ColorActiveStart), 1.0f);
	rc.Narrow(0,0,1,1);
	g.DrawRectangle(&border, rc);

	// draw block
	Color progress = theme->GetColor(Theme::ColorProgress);
	Color green = Theme::ChangeAlpha(progress, 255);
	Color trans = Theme::ChangeAlpha(progress, 0);
	int center = int(v * float(rc.GetWidth()));
	LinearGradientBrush lbr(PointF(float(center-56), 0.0f), PointF(float(center), 0.0f), trans, green);
	LinearGradientBrush rbr(PointF(float(center), 0.0f), PointF(float(center+56), 0.0f), green, trans);

	
	if(!_indeterminate) {
		SolidBrush greenBrush(green);
		g.FillRectangle(&greenBrush, RectF(1.0f, 1.0f, float(center), float(rc.GetHeight()-1) ));
		g.FillRectangle(&rbr, RectF(float(center), 1.0f, 54.0f, float(rc.GetHeight()-1)));
	}
	else {
		g.FillRectangle(&lbr, RectF(float(center-54), 1.0f, 54.0f, float(rc.GetHeight()-1)));
		g.FillRectangle(&rbr, RectF(float(center), 1.0f, 54.0f, float(rc.GetHeight()-1)));
	}

	// draw glass look
	LinearGradientBrush glass(PointF(0.0f, 0.0f), PointF(0.0f, 12), theme->GetColor(Theme::ColorProgressGlassStart), theme->GetColor(Theme::ColorProgressGlassEnd));
	g.FillRectangle(&glass, RectF(1.0f, 1.0f, float(rc.GetWidth()-2), 10.0f));
}

void ProgressWnd::SetValue(float x) {
	_value = x;
	Repaint();
}

void ProgressWnd::SetIndeterminate(bool t) {
	_indeterminate = t;
	if(_indeterminate) {
		StartTimer(Time(10), 0);
	}
	else {
		StopTimer(0);
	}
	Update();
}