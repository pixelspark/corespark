#ifndef _TJTHEME_H
#define _TJTHEME_H

#include <Gdiplus.h>

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED Theme {
	public:
		Theme();
		virtual ~Theme();
		virtual std::wstring GetName();
		virtual Gdiplus::Color GetBackgroundColor();
		virtual Gdiplus::Color GetTimeBackgroundColor();
		virtual Gdiplus::Color GetPropertyBackgroundColor();
		virtual Gdiplus::Color GetLineColor();
		virtual Gdiplus::Color GetTextColor();
		virtual Gdiplus::Color GetActiveTrackColor();

		virtual Gdiplus::Color GetActiveStartColor();
		virtual Gdiplus::Color GetActiveEndColor();

		virtual Gdiplus::Color GetSplitterStartColor();
		virtual Gdiplus::Color GetSplitterEndColor();

		virtual Gdiplus::Color GetCurrentPositionColor();
		virtual Gdiplus::Color GetTrackDescriptionTextColor();
		virtual Gdiplus::Color GetFaderColor();
		virtual Gdiplus::Font* GetGUIFont();
		virtual Gdiplus::Font* GetGUIFontBold();

	protected:
		Gdiplus::Font* _font;
		Gdiplus::Font* _fontBold;
};

class BrightTheme: public Theme {
	public:
		BrightTheme();
		virtual ~BrightTheme();
		virtual std::wstring GetName();
		virtual Gdiplus::Color GetBackgroundColor();
		virtual Gdiplus::Color GetTimeBackgroundColor();
		virtual Gdiplus::Color GetPropertyBackgroundColor();
		virtual Gdiplus::Color GetLineColor();
		virtual Gdiplus::Color GetTextColor();
		virtual Gdiplus::Color GetActiveTrackColor();

		virtual Gdiplus::Color GetActiveStartColor();
		virtual Gdiplus::Color GetActiveEndColor();

		virtual Gdiplus::Color GetSplitterStartColor();
		virtual Gdiplus::Color GetSplitterEndColor();
		virtual Gdiplus::Color GetCurrentPositionColor();
		virtual Gdiplus::Color GetTrackDescriptionTextColor();
		virtual Gdiplus::Color GetFaderColor();
};

#pragma warning(pop)
#endif