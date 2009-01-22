#include "../../include/ui/tjui.h" 
using namespace tj::shared;
using namespace tj::shared::graphics;

namespace tj {
	namespace shared {
		namespace themes {
			class BrightTheme: public Theme {
				public:
					BrightTheme(float dpi=1.0f);
					virtual ~BrightTheme();
					virtual std::wstring GetName() const;
					virtual graphics::Color GetColor(const Theme::ColorIdentifier& ci) const;
					virtual graphics::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const;
			};

			class VistaTheme: public Theme {
				public:
					VistaTheme(float dpi);
					virtual ~VistaTheme();
					virtual std::wstring GetName() const;
					virtual graphics::Color GetColor(const ColorIdentifier& ci) const;
					virtual graphics::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const;
			};
		}
	}
}

using namespace tj::shared::themes;

// Bright theme
BrightTheme::BrightTheme(float dpi): Theme(dpi) {
}

BrightTheme::~BrightTheme() {
}

Color BrightTheme::GetColor(const Theme::ColorIdentifier& ci) const {
	switch(ci) {
		case ColorEditBackground:
		case ColorBackground:
		case ColorPropertyBackground:
		case ColorTabButtonBackground:
			return Color(255,255,255);

		case ColorTimeBackground:
			return Color(245,245,245);

		case ColorShadowed:
			return ChangeAlpha(GetColor(ColorShadow),255/4);

		case ColorShadow:
			return Color(127, 0,0,0);

		case ColorLine:
			return Color(150,150,150);

		case ColorText:
			return Color(0,0,0);

		case ColorActiveTrack:
			return Color(227,227,227); // 240,240,240

		case ColorActiveStart:
		case ColorToolbarStart:
			return Color(149,149,149);

		case ColorActiveEnd:
		case ColorToolbarEnd:
			return Color(176,176,176);

		case ColorSplitterStart:
			return Color(204,204,204);

		case ColorSplitterEnd:
			return Color(255,255,255);
		
		case ColorCurrentPosition:
			return Color(0,0,0);

		case ColorDescriptionText:
			return Color(120,120,120);

		case ColorFader:
			return Color(0,0,0);

		case ColorFocus:
			return Color(200, 0,0,255);

		case ColorTimeSelectionEnd:
			return Color(100,0,0,100);

		case ColorTimeSelectionStart:
			return Color(50, 0, 0, 100);

		case ColorDisabledOverlay:
			return Color(200,255,255,255);

		case ColorTabButtonStart:
			return Color(100,145,145,145);

		case ColorTabButtonEnd:
			return Color(100,185,185,185);

		case ColorHighlightStart:
			return Color(255, 255,100,0);

		case ColorHighlightEnd:
			return Color(100, 255, 100, 0);

		case ColorGlassStart:
			return Color(50,255,255,255);

		case ColorGlassEnd:
			return Color(100,255,255,255);

		case ColorProgressBackgroundStart:
			return Color(201,201,201);

		case ColorProgressBackgroundEnd:
			return Color(255,255,255);

		case ColorProgressGlassStart:
			return Color(211,255,255,255);

		case ColorProgressGlassEnd:
			return Color(43,255,255,255);

		case ColorProgress:
			return Color(0,211,40);

		default:
			return Theme::GetColor(ci);
	}
}

std::wstring BrightTheme::GetName() const { 
	if(_dpi==1.0f) {
		return TL(theme_bright);
	}
	else {
		return TL(theme_bright) + std::wstring(L" (") + Stringify(_dpi*Theme::KDefaultDPI)+L" dpi)"; 
	}
}	

Brush* BrightTheme::GetApplicationBackgroundBrush(HWND root, HWND child) const {
	RECT rootrc, childrc;
	GetWindowRect(root, &rootrc);
	GetWindowRect(child, &childrc);

	graphics::LinearGradientBrush* lbr = new graphics::LinearGradientBrush(PointF(0.0f, -float(childrc.top-rootrc.top)), PointF(0.0f,float(rootrc.bottom-rootrc.top)), Color(255,255,255), Color(204,204,204));
	lbr->SetWrapMode(WrapModeClamp);
	REAL factors[3] = {1.0f, 0.0f, 0.0f};
	REAL positions[3] = {0.0f, 0.25f ,1.0f};
	lbr->SetBlend(factors,positions, 3);

	return lbr;
}

graphics::Color VistaTheme::GetColor(const Theme::ColorIdentifier& ci) const {
	switch(ci) {
		case ColorHighlightStart:
			return Color(255, 145,175,194);

		case ColorHighlightEnd:
			return Color(100, 145,175,194);

		case ColorToolbarStart:
			return Color(24,106,119);

		case ColorToolbarEnd:
			return Color(87,164,171);

		case ColorGlassStart:
			return Color(50,255,255,255);

		case ColorGlassEnd:
			return Color(100,255,255,255);

		case ColorBackground:
		case ColorEditBackground:
			return Color(255,255,255);

		case ColorActiveTrack:
			return Color(231,241,244);

		case ColorTabButtonBackground:
			return Color(130,255,255,255);

		case ColorTimeBackground:
		case ColorPropertyBackground:
			return Color(241,251,254);

		case ColorText:
			return Color(0,0,0);

		case ColorActiveStart:
			return Color(200,165,195,214); // 200,145,175,194

		case ColorActiveEnd:
		case ColorLine:
			return Color(150,178,195);

		case ColorFocus:
			return Color::DarkBlue;

		case ColorSplitterStart:
			return Color(255,255,255);

		case ColorSplitterEnd:
			return Color(255,255,255);

		case ColorCurrentPosition:
			return Color(0,0,0);

		case ColorShadowed:
			return ChangeAlpha(GetColor(ColorShadow),255/4);

		case ColorShadow:
			return Color(127,0,0,0);

		case ColorDescriptionText:
			return Color(0,0,0);

		case ColorFader:
			return Color(0,0,0);

		case ColorTimeSelectionEnd:
			return Color(100,0,0,100);

		case ColorTimeSelectionStart:
			return Color(50, 0, 0, 100);

		case ColorTabButtonStart:
			return Color(0,0,0,0);

		case ColorTabButtonEnd:
			return Color(0,0,0,0);

		case ColorDisabledOverlay:
			return Color(200,255,255,255);

		default:
			return Theme::GetColor(ci);
	}
}

graphics::Brush* VistaTheme::GetApplicationBackgroundBrush(HWND root, HWND child) const {
	RECT rootrc, childrc;
	GetWindowRect(root, &rootrc);
	GetWindowRect(child, &childrc);

	graphics::LinearGradientBrush* lbr = new graphics::LinearGradientBrush(PointF(0.0f, -float(childrc.top-rootrc.top)), PointF(0.0f,float(rootrc.bottom-rootrc.top)), Color(242,251,254), GetColor(ColorActiveEnd));
	lbr->SetWrapMode(WrapModeClamp);
	REAL factors[3] = {1.0f, 0.0f, 0.0f};
	REAL positions[3] = {0.0f, 0.25f ,1.0f};
	lbr->SetBlend(factors,positions, 3);

	return lbr;
}

VistaTheme::VistaTheme(float dpi): Theme(dpi) {
}

VistaTheme::~VistaTheme() {
}

std::wstring VistaTheme::GetName() const {
	if(_dpi==1.0f) {
		return TL(theme_vista);
	}
	else {
		return TL(theme_vista) + std::wstring(L"(")+Stringify(_dpi*Theme::KDefaultDPI)+L" dpi)";
	}
}

namespace tj {
	namespace shared {
		namespace themes {
			// Order is important (needed when saving theme choice to settings)!
			ThemeSubscription _theme_subscription(GC::Hold(new Theme(Theme::KDefaultDPI)));
			ThemeSubscription _subscription_bright_theme(ref<Theme>(GC::Hold(new BrightTheme(Theme::KDefaultDPI))));
			ThemeSubscription _subscription_vista_theme(ref<Theme>(GC::Hold(new VistaTheme(Theme::KDefaultDPI))));

			// All weird DPI themes
			ThemeSubscription _theme_subscription_large(GC::Hold(new Theme(Theme::KDefaultDPI*1.5f)));
			ThemeSubscription _subscription_bright_theme_large(ref<Theme>(GC::Hold(new BrightTheme(Theme::KDefaultDPI*1.5f))));
			ThemeSubscription _theme_subscription_small(GC::Hold(new Theme(Theme::KDefaultDPI/1.5f)));
			ThemeSubscription _subscription_vista_theme_large(ref<Theme>(GC::Hold(new VistaTheme(Theme::KDefaultDPI*1.5f))));
		}
	}
}