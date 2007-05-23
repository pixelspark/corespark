#include "../../include/tjshared.h"
#include "../../resource.h"
using namespace Gdiplus;
using namespace tj::shared;

const float Theme::KDefaultDPI = 96.0f;

Theme::Theme(float dpi) {
	_dpi = dpi/KDefaultDPI;
	_font = 0;
	_fontBold = 0;
	_fontSmall = 0;
	_grab = LoadCursor(GetModuleHandle(L"TJShared"), MAKEINTRESOURCE(IDC_GRABHAND));
	_grabbed = LoadCursor(GetModuleHandle(L"TJShared"), MAKEINTRESOURCE(IDC_GRAB));
}

Theme::~Theme() {
	delete _font;
	delete _fontBold;
	delete _fontSmall;
	DestroyCursor(_grab);
	DestroyCursor(_grabbed);
}

/** DPI Stuff */
float Theme::GetDPIScaleFactor() const {
	return _dpi;
}

void Theme::DrawToolbarBackground(Gdiplus::Graphics& g, float x, float y, float w, float h) {
	Area rc(RectF(x,y,w,h));

	SolidBrush zwart(GetBackgroundColor());
	g.FillRectangle(&zwart, rc);

	LinearGradientBrush br(PointF(x, y), PointF(x, y+float(rc.GetHeight())), GetToolbarColorStart(), GetToolbarColorEnd());
	SolidBrush dbr(GetDisabledOverlayColor());
	g.FillRectangle(&br, rc);
	g.FillRectangle(&dbr, rc);

	LinearGradientBrush glas(PointF(x,y), PointF(x,y+float(rc.GetHeight())/2.0f), GetGlassColorStart(), GetGlassColorEnd());
	g.FillRectangle(&glas, RectF(x,y, float(rc.GetWidth()), float(rc.GetHeight())/2.0f));
}

int Theme::GetMeasureInPixels(Measure m) const {
	switch(m) {
		case MeasureToolbarHeight:
		case MeasureListHeaderHeight:
			return 24;

		case MeasureListItemHeight:
			return 19;

		case MeasureMaximumSnappingDistance:
			return 2;
	}

	return -1;
}

HCURSOR Theme::GetGrabCursor() const {
	return _grab;
}

HCURSOR Theme::GetGrabbedCursor() const {
	return _grabbed;
}

Gdiplus::Color Theme::ChangeAlpha(Gdiplus::Color col, int a) {
	return Gdiplus::Color(a, col.GetR(), col.GetG(), col.GetB());
}

Gdiplus::Font* Theme::GetGUIFontBold() const {
	if(_fontBold==0) {
		_fontBold = new Font(L"Tahoma", 11, FontStyleBold, UnitPixel, 0);
	}
	return _fontBold;
}

Gdiplus::Font* Theme::GetGUIFont() const {
	if(_font==0) {
		_font = new Font(L"Tahoma", 11, FontStyleRegular, UnitPixel, 0);
	}
	return _font;
}

Gdiplus::Font* Theme::GetGUIFontSmall() const {
	if(_fontSmall==0) {
		_fontSmall = new Font(L"Tahoma", 9, FontStyleRegular, UnitPixel, 0);
	}
	return _fontSmall;
}


std::wstring Theme::GetName() const { 
	if(_dpi!=1.0f) {
		return TL(theme_default) + std::wstring(L" (") + Stringify(_dpi*KDefaultDPI) + L" dpi) ";
	}
	else {
		return TL(theme_default);
	}
}

Color Theme::GetBackgroundColor() const {
	return Color(0,0,0);
}

Color Theme::GetEditBackgroundColor() const {
	return GetBackgroundColor();
}

Color Theme::GetTimeBackgroundColor() const {
	return Color(15,15,15);
}

Color Theme::GetPropertyBackgroundColor() const {
	return Color(0,0,0);
}

Color Theme::GetLineColor() const {
	return Color(50,50,50);
}

Color Theme::GetTextColor() const {
	return Color(255,255,255);
}

Color Theme::GetActiveTrackColor() const {
	return Color(25,25,25);
}

Color Theme::GetActiveStartColor() const {
	//return Color(255, 110,110,110);
	return Color(230,130,130,130);
}

Color Theme::GetCommandMarkerColor() const {
	return Color(255,0,0);
}

Color Theme::GetActiveEndColor() const {
	return Color(230, 90,90,90); // 255,70,70,70
}

Color Theme::GetHighlightColorStart() const {
	return Color(255, 127, 75, 0);
}

Color Theme::GetHighlightColorEnd() const {
	
	return Color(255, 255,150,0);
}

Color Theme::GetSplitterStartColor() const {
	return Color(0,0,0);
}

Color Theme::GetSplitterEndColor() const {
	return Color(0,0,0);
}

Color Theme::GetToolbarColorStart() const {
	return GetActiveStartColor();
}

Color Theme::GetToolbarColorEnd() const {
	return GetActiveEndColor();
}

Color Theme::GetGlassColorStart() const {
	return Color(0,255,255,255);
}

Color Theme::GetGlassColorEnd() const {
	return Color(15,255,255,255);
}

Color Theme::GetCurrentPositionColor() const {
	return Color(255,255,255);
}

Color Theme::GetTrackDescriptionTextColor() const {
	return Color(120,120,120);
}

Gdiplus::Color Theme::GetTimeSelectionColorEnd() const {
	return Color(100,100,50, 0); //Color(50,255,255,255);
}

Gdiplus::Color Theme::GetTimeSelectionColorStart() const {
	return Color(100,200,100, 0); //Color(0, 255, 255, 255);
}

Color Theme::GetSliderColorStart(int i) const {
	switch(i) {
		case SliderMaster:
			return Color(255, 200,25,25);

		case SliderSubmix:
			return Color(255, 25, 200, 25);

		case SliderMacro:
			return Color(255, 60, 60, 110);

		case SliderAlpha:
			return Color(200, 200, 60);

		default:
			return GetActiveStartColor();
	}
}

Color Theme::GetSliderColorEnd(int i) const {
	switch(i) {
		case SliderMaster:
			return Color(150, 200,25,25);

		case SliderSubmix:
			return Color(150, 25, 200, 25);

		case SliderMacro:
			return Color(150, 60, 60, 110);

		case SliderAlpha:
			return Color(150, 150, 60);

		default:
			return GetActiveEndColor();
	}
}

Color Theme::GetFaderColor() const {
	return Color(255,255,255);
}

Color Theme::GetDisabledOverlayColor() const {
	return Color(200,0,0,0);
}

Color Theme::GetTabButtonColorStart() const {
	return Color(100,110,110,110);
}

Color Theme::GetTabButtonColorEnd() const {
	return Color(100,70,70,70);
}

Brush* Theme::GetApplicationBackgroundBrush(HWND root, HWND child) const {
	RECT rootrc, childrc;
	GetWindowRect(root, &rootrc);
	GetWindowRect(child, &childrc);

	Gdiplus::LinearGradientBrush* lbr = new Gdiplus::LinearGradientBrush(PointF(0.0f, -float(childrc.top-rootrc.top)), PointF(0.0f,float(rootrc.bottom-rootrc.top)), Color(0,0,0), Color(90,90,90));
	lbr->SetWrapMode(WrapModeClamp);
	REAL factors[3] = {1.0f, 0.0f, 0.0f};
	REAL positions[3] = {0.0f, 0.25f ,1.0f};
	lbr->SetBlend(factors,positions, 3);

	return lbr;
}

Color Theme::GetProgressBackStart() const {
	return Color(201,201,201);
}

Color Theme::GetProgressBackEnd() const {
	return Color(255,255,255);
}

Color Theme::GetProgressGlassStart() const {
	return Color(211,255,255,255);
}

Color Theme::GetProgressGlassEnd() const {
	return Color(43,255,255,255);
}

Color Theme::GetProgressColor() const {
	return Color(0,211,40);
}