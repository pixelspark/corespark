#ifndef _TJTHEME_H
#define _TJTHEME_H

#include <Gdiplus.h>

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED Theme {
	public:
		Theme();
		virtual ~Theme();
		virtual std::wstring GetName() const;
		virtual Gdiplus::Color GetBackgroundColor() const;
		virtual Gdiplus::Color GetTimeBackgroundColor() const;
		virtual Gdiplus::Color GetPropertyBackgroundColor() const;
		virtual Gdiplus::Color GetLineColor() const;
		virtual Gdiplus::Color GetTextColor() const;
		virtual Gdiplus::Color GetActiveTrackColor() const;

		virtual Gdiplus::Color GetActiveStartColor() const;
		virtual Gdiplus::Color GetActiveEndColor() const;

		virtual Gdiplus::Color GetSplitterStartColor() const;
		virtual Gdiplus::Color GetSplitterEndColor() const;

		virtual Gdiplus::Color GetCurrentPositionColor() const;
		virtual Gdiplus::Color GetTrackDescriptionTextColor() const;
		virtual Gdiplus::Color GetFaderColor() const;
		virtual Gdiplus::Font* GetGUIFont() const;
		virtual Gdiplus::Font* GetGUIFontBold() const;

		virtual Gdiplus::Color GetSliderColorStart(int i) const;
		virtual Gdiplus::Color GetSliderColorEnd(int i) const;

		virtual Gdiplus::Color GetDisabledOverlayColor() const;
		virtual Gdiplus::Color GetCommandMarkerColor() const;

		enum {
			SliderNormal = 0,
			SliderSubmix,
			SliderMaster,
			SliderMacro,
		};

	protected:
		mutable Gdiplus::Font* _font;
		mutable Gdiplus::Font* _fontBold;
};

class BrightTheme: public Theme {
	public:
		BrightTheme();
		virtual ~BrightTheme();
		virtual std::wstring GetName() const;
		virtual Gdiplus::Color GetBackgroundColor() const;
		virtual Gdiplus::Color GetTimeBackgroundColor() const;
		virtual Gdiplus::Color GetPropertyBackgroundColor() const;
		virtual Gdiplus::Color GetLineColor() const;
		virtual Gdiplus::Color GetTextColor() const;
		virtual Gdiplus::Color GetActiveTrackColor() const;

		virtual Gdiplus::Color GetActiveStartColor() const;
		virtual Gdiplus::Color GetActiveEndColor() const;

		virtual Gdiplus::Color GetSplitterStartColor() const;
		virtual Gdiplus::Color GetSplitterEndColor() const;
		virtual Gdiplus::Color GetCurrentPositionColor() const;
		virtual Gdiplus::Color GetTrackDescriptionTextColor() const;
		virtual Gdiplus::Color GetFaderColor() const;

		virtual Gdiplus::Color GetDisabledOverlayColor() const;
};

#pragma warning(pop)
#endif