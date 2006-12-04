#include "../include/tjshared.h"
#include "../resource.h"
using namespace Gdiplus;
using namespace tj::shared;

ThemeSubscription _subscription_theme(ref<Theme>(GC::Hold(new Theme())));
ThemeSubscription _subscription_bright_theme(ref<Theme>(GC::Hold(new BrightTheme())));

Theme::Theme() {
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
	return TL(theme_default);
}

Color Theme::GetBackgroundColor() const {
	return Color(0,0,0);
}

Color Theme::GetEditBackgroundColor() const {
	return GetBackgroundColor();
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

/*Gdiplus::Color Theme::GetProgressBackStart() const {
	return Color(0,0,0);
}

Gdiplus::Color Theme::GetProgressBackEnd() const {
	return Color(54,54,54);
}

Gdiplus::Color Theme::GetProgressGlassStart() const {
	return Color(211,255,255,255);
}
Gdiplus::Color Theme::GetProgressGlassEnd() const {
	return Color(43,255,255,255);
}

Gdiplus::Color Theme::GetProgressColor() const {
	return Color(0,44,215);
} */

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

	
// Bright theme
BrightTheme::BrightTheme() {
}

BrightTheme::~BrightTheme() {
}

Color BrightTheme::GetBackgroundColor() const {
	return Color(255,255,255);
}

Color BrightTheme::GetTimeBackgroundColor() const {
	return Color(235,235,235);
}

Color BrightTheme::GetEditBackgroundColor() const {
	return GetBackgroundColor();
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
	return Color(230,230,230);
}

Color BrightTheme::GetActiveStartColor() const {
	return Color(139,139,139);
}

Color BrightTheme::GetActiveEndColor() const {
	return Color(166,166,166);
}

Color BrightTheme::GetSplitterStartColor() const {
	return Color(204,204,204);
}

Color BrightTheme::GetSplitterEndColor() const {
	return Color(255,255,255);
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

std::wstring BrightTheme::GetName() const { return TL(theme_bright); }

Color BrightTheme::GetTabButtonColorStart() const {
	return Color(100,145,145,145);
}

Color BrightTheme::GetTabButtonColorEnd() const {
	return Color(100,185,185,185);
}

Color BrightTheme::GetHighlightColorStart() const {
	return Color(255, 255,100,0);
}

Color BrightTheme::GetHighlightColorEnd() const {
	return Color(100, 255, 100, 0);
}

Color BrightTheme::GetToolbarColorStart() const {
	return GetActiveStartColor();
}

Color BrightTheme::GetToolbarColorEnd() const {
	return GetActiveEndColor();
}

Color BrightTheme::GetGlassColorStart() const {
	return Color(50,255,255,255);
}

Color BrightTheme::GetGlassColorEnd() const {
	return Color(100,255,255,255);
}

Color BrightTheme::GetProgressBackStart() const {
	return Color(201,201,201);
}

Color BrightTheme::GetProgressBackEnd() const {
	return Color(255,255,255);
}

Color BrightTheme::GetProgressGlassStart() const {
	return Color(211,255,255,255);
}

Color BrightTheme::GetProgressGlassEnd() const {
	return Color(43,255,255,255);
}

Color BrightTheme::GetProgressColor() const {
	return Color(0,211,40);
}
	

Brush* BrightTheme::GetApplicationBackgroundBrush(HWND root, HWND child) const {
	RECT rootrc, childrc;
	GetWindowRect(root, &rootrc);
	GetWindowRect(child, &childrc);

	Gdiplus::LinearGradientBrush* lbr = new Gdiplus::LinearGradientBrush(PointF(0.0f, -float(childrc.top-rootrc.top)), PointF(0.0f,float(rootrc.bottom-rootrc.top)), Color(255,255,255), Color(204,204,204));
	lbr->SetWrapMode(WrapModeClamp);
	REAL factors[3] = {1.0f, 0.0f, 0.0f};
	REAL positions[3] = {0.0f, 0.25f ,1.0f};
	lbr->SetBlend(factors,positions, 3);

	return lbr;
}

/** Vista theme */

class VistaTheme: public Theme {
	public:
		VistaTheme() {
		}

		virtual ~VistaTheme() {
		}

		virtual std::wstring GetName() const {
			return TL(theme_vista);
		}

		virtual Gdiplus::Color GetBackgroundColor() const {
			return Color(255,255,255);
		}

		virtual Gdiplus::Color GetTimeBackgroundColor() const {
			return Color(241,251,254);
		}
		virtual Gdiplus::Color GetPropertyBackgroundColor() const {
			return GetTimeBackgroundColor();
		}

		virtual Gdiplus::Color GetLineColor() const {
			return GetActiveEndColor();
		}

		virtual Gdiplus::Color GetTextColor() const {
			return Color(0,0,0);
		}

		virtual Gdiplus::Color GetActiveTrackColor() const {
			return GetBackgroundColor();
		}

		virtual Gdiplus::Color GetEditBackgroundColor() const {
			return Color(255,255,255);
		}

		virtual Gdiplus::Color GetActiveStartColor() const {
			//return Color(242,251,254);
			return GetActiveEndColor();
		}

		virtual Gdiplus::Color GetActiveEndColor() const {
			return Color(145,175,194);
		}

		virtual Gdiplus::Color GetSplitterStartColor() const {
			return Color(255,255,255);
		}

		virtual Gdiplus::Color GetSplitterEndColor() const {
			return Color(255,255,255);
		}

		virtual Gdiplus::Color GetCurrentPositionColor() const {
			return Color(0,0,0);
		}

		virtual Gdiplus::Color GetTrackDescriptionTextColor() const {
			return Color(0,0,0);
		}

		virtual Gdiplus::Color GetFaderColor() const {
			return Color(0,0,0);
		}

		virtual Gdiplus::Color GetTimeSelectionColorEnd() const {
			return Color(50,255,255,255);
		}
		
		virtual Gdiplus::Color GetTimeSelectionColorStart() const {
			return Color(0,255,255,255);
		}

		virtual Gdiplus::Color GetTabButtonColorStart() const {
			return Color(0,0,0,0);
		}

		virtual Gdiplus::Color GetTabButtonColorEnd() const {
			return Color(0,0,0,0);
		}

		virtual Gdiplus::Color GetDisabledOverlayColor() const {
			return Color(200,255,255,255);
		}

		virtual Gdiplus::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const {
			RECT rootrc, childrc;
			GetWindowRect(root, &rootrc);
			GetWindowRect(child, &childrc);

			Gdiplus::LinearGradientBrush* lbr = new Gdiplus::LinearGradientBrush(PointF(0.0f, -float(childrc.top-rootrc.top)), PointF(0.0f,float(rootrc.bottom-rootrc.top)), Color(242,251,254), GetActiveEndColor());
			lbr->SetWrapMode(WrapModeClamp);
			REAL factors[3] = {1.0f, 0.0f, 0.0f};
			REAL positions[3] = {0.0f, 0.25f ,1.0f};
			lbr->SetBlend(factors,positions, 3);

			return lbr;
		}

		virtual Gdiplus::Color GetHighlightColorStart() const {
			return Color(255, 145,175,194);
		}

		virtual Gdiplus::Color GetHighlightColorEnd() const {
			return Color(100, 145,175,194);
		}

		virtual Gdiplus::Color GetToolbarColorStart() const {
			return Color(24,106,119);
		}

		virtual Gdiplus::Color GetToolbarColorEnd() const {
			return Color(87,164,171);
		}

		virtual Color GetGlassColorStart() const {
			return Color(50,255,255,255);
		}

		virtual Color GetGlassColorEnd() const {
			return Color(100,255,255,255);
		}
};

ThemeSubscription _subscription_vista_theme(ref<Theme>(GC::Hold(new VistaTheme())));