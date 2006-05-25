#include "../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

ThemeSubscription _subscription_theme(ref<Theme>(GC::Hold(new Theme())));
ThemeSubscription _subscription_bright_theme(ref<Theme>(GC::Hold(new BrightTheme())));

Theme::Theme() {
	_font = 0;
	_fontBold = 0;
	_fontSmall = 0;
}

Theme::~Theme() {
	delete _font;
	delete _fontBold;
	delete _fontSmall;
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
		_fontSmall = new Font(L"Tahoma", 8, FontStyleRegular, UnitPixel, 0);
	}
	return _fontSmall;
}


std::wstring Theme::GetName() const { 
	return std::wstring(L"Donker (Standaard)");
}

Color Theme::GetBackgroundColor() const {
	return Color(0,0,0);
}

Color Theme::GetTimeBackgroundColor() const {
	return Color(20,20,20);
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

Color Theme::GetSplitterStartColor() const {
	return Color(0,0,0);
}

Color Theme::GetSplitterEndColor() const {
	return Color(0,0,0);
}

Color Theme::GetCurrentPositionColor() const {
	return Color(255,255,255);
}

Color Theme::GetTrackDescriptionTextColor() const {
	return Color(120,120,120);
}

Gdiplus::Color Theme::GetTimeSelectionColorEnd() const {
	return Color(50,255,255,255);
}

Gdiplus::Color Theme::GetTimeSelectionColorStart() const {
	return Color(0, 255, 255, 255);
}

Color Theme::GetSliderColorStart(int i) const {
	switch(i) {
		case SliderMaster:
			return Color(255, 200,25,25);

		case SliderSubmix:
			return Color(255, 25, 200, 25);

		case SliderMacro:
			return Color(255, 60, 60, 110);

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
	
// Bright theme

BrightTheme::BrightTheme() {
}

BrightTheme::~BrightTheme() {
}

Color BrightTheme::GetBackgroundColor() const {
	return Color(255,255,255);
}

Color BrightTheme::GetTimeBackgroundColor() const {
	return Color(236,233,216);
}

Color BrightTheme::GetPropertyBackgroundColor() const {
	return Color(255,255,255);
}

Color BrightTheme::GetLineColor() const {
	return Color(150,150,150);
}

Color BrightTheme::GetTextColor() const {
	return Color(0,0,0);
}

Color BrightTheme::GetActiveTrackColor() const {
	return Color(206,203,186);
}

Color BrightTheme::GetActiveStartColor() const {
	return Color(136,136,136);
}

Color BrightTheme::GetActiveEndColor() const {
	return Color(150,150,150);
}

Color BrightTheme::GetSplitterStartColor() const {
	return Color(236,233,216);
}

Color BrightTheme::GetSplitterEndColor() const {
	return Color(236,233,216);
}

Color BrightTheme::GetCurrentPositionColor() const {
	return Color(0,0,0);
}

Color BrightTheme::GetTrackDescriptionTextColor() const {
	return Color(120,120,120);
}

Color BrightTheme::GetFaderColor() const {
	return Color(0,0,0);
}

Gdiplus::Color BrightTheme::GetTimeSelectionColorEnd() const {
	return Color(100,0,0,100);
}

Gdiplus::Color BrightTheme::GetTimeSelectionColorStart() const {
	return Color(50, 0, 0, 100);
}

Color BrightTheme::GetDisabledOverlayColor() const {
	return Color(200,255,255,255);
}

std::wstring BrightTheme::GetName() const { return std::wstring(L"Licht"); }

Brush* BrightTheme::GetApplicationBackgroundBrush(HWND root, HWND child) const {
	return new SolidBrush(GetTimeBackgroundColor());
}

Color BrightTheme::GetTabButtonColorStart() const {
	return Color(100,255,255,255);
}

Color BrightTheme::GetTabButtonColorEnd() const {
	return Color(100,255,255,255);
}