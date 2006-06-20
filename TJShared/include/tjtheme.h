#ifndef _TJTHEME_H
#define _TJTHEME_H

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
		virtual Gdiplus::Font* GetGUIFontSmall() const;

		virtual Gdiplus::Color GetSliderColorStart(int i) const;
		virtual Gdiplus::Color GetSliderColorEnd(int i) const;

		virtual Gdiplus::Color GetTimeSelectionColorEnd() const;
		virtual Gdiplus::Color GetTimeSelectionColorStart() const;

		virtual Gdiplus::Color GetTabButtonColorStart() const;
		virtual Gdiplus::Color GetTabButtonColorEnd() const;

		virtual Gdiplus::Color GetDisabledOverlayColor() const;
		virtual Gdiplus::Color GetCommandMarkerColor() const;

		virtual Gdiplus::Color GetHighlightColorStart() const;
		virtual Gdiplus::Color GetHighlightColorEnd() const;
		
		virtual HCURSOR GetGrabCursor() const;
		virtual HCURSOR GetGrabbedCursor() const;
		virtual Gdiplus::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const;

		static Gdiplus::Color ChangeAlpha(Gdiplus::Color col, int a);
		
		enum {
			SliderNormal = 0,
			SliderSubmix,
			SliderMaster,
			SliderMacro,
		};

	protected:
		mutable Gdiplus::Font* _font;
		mutable Gdiplus::Font* _fontBold;
		mutable Gdiplus::Font* _fontSmall;
		HCURSOR _grab, _grabbed;
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

		virtual Gdiplus::Color GetTimeSelectionColorEnd() const;
		virtual Gdiplus::Color GetTimeSelectionColorStart() const;

		virtual Gdiplus::Color GetTabButtonColorStart() const;
		virtual Gdiplus::Color GetTabButtonColorEnd() const;

		virtual Gdiplus::Color GetDisabledOverlayColor() const;
		virtual Gdiplus::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const;

		virtual Gdiplus::Color GetHighlightColorStart() const;
		virtual Gdiplus::Color GetHighlightColorEnd() const;
};

#pragma warning(pop)
#endif