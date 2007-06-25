#ifndef _TJTHEME_H
#define _TJTHEME_H

namespace tj {
	namespace shared {
		class EXPORTED Theme: public virtual Object {
			public:
				Theme(float dpi);
				virtual ~Theme();
				virtual std::wstring GetName() const;

				enum Measure {
					MeasureToolbarHeight = 1,
					MeasureListItemHeight,
					MeasureListHeaderHeight,
					MeasureMaximumSnappingDistance,
				};

				// Colors
				virtual Gdiplus::Color GetBackgroundColor() const;
				virtual Gdiplus::Color GetVideoBackgroundColor() const;
				virtual Gdiplus::Color GetEditBackgroundColor() const;
				virtual Gdiplus::Color GetTimeBackgroundColor() const;
				virtual Gdiplus::Color GetPropertyBackgroundColor() const;
				virtual Gdiplus::Color GetLineColor() const;
				virtual Gdiplus::Color GetTextColor() const;
				virtual Gdiplus::Color GetActiveTrackColor() const;
				virtual Gdiplus::Color GetTabButtonBackgroundColor() const;

				virtual Gdiplus::Color GetActiveStartColor() const;
				virtual Gdiplus::Color GetActiveEndColor() const;

				virtual Gdiplus::Color GetSplitterStartColor() const;
				virtual Gdiplus::Color GetSplitterEndColor() const;

				virtual Gdiplus::Color GetCurrentPositionColor() const;
				virtual Gdiplus::Color GetTrackDescriptionTextColor() const;
				virtual Gdiplus::Color GetFaderColor() const;

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

				virtual Gdiplus::Color GetGlassColorStart() const;
				virtual Gdiplus::Color GetGlassColorEnd() const;

				virtual Gdiplus::Color GetToolbarColorStart() const;
				virtual Gdiplus::Color GetToolbarColorEnd() const;

				// ProgressWnd
				virtual Gdiplus::Color GetProgressBackStart() const;
				virtual Gdiplus::Color GetProgressBackEnd() const;
				virtual Gdiplus::Color GetProgressGlassStart() const;
				virtual Gdiplus::Color GetProgressGlassEnd() const;
				virtual Gdiplus::Color GetProgressColor() const;

				// Fonts
				virtual Gdiplus::Font* GetGUIFont() const;
				virtual Gdiplus::Font* GetGUIFontBold() const;
				virtual Gdiplus::Font* GetGUIFontSmall() const;

				// Cursors
				virtual HCURSOR GetGrabCursor() const;
				virtual HCURSOR GetGrabbedCursor() const;
				virtual Gdiplus::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const;
				
				/* This needs a little explanation. This is the 'scale factor' to convert from 'logical pixels' 
				(which we define as one pixel on a normal, 96 DPI computer screen) to 'device pixels', which is
				what you use to paint in Windows. So when you're on a 96 DPI screen, this returns 1.0f. When you're
				on 120 dpi, this returns 1.25f. */
				virtual float GetDPIScaleFactor() const;

				// Measures
				virtual Pixels GetMeasureInPixels(Theme::Measure m) const;

				static Gdiplus::Color ChangeAlpha(Gdiplus::Color col, int a);
				
				enum {
					SliderNormal = 0,
					SliderSubmix,
					SliderMaster,
					SliderMacro,
					SliderAlpha,
				};

				const static float KDefaultDPI; // One logical pixel equals 1/KDefaultDPI inches

				// utility methods
				virtual void DrawToolbarBackground(Gdiplus::Graphics& g, float x, float y, float w, float h);

			protected:
				mutable Gdiplus::Font* _font;
				mutable Gdiplus::Font* _fontBold;
				mutable Gdiplus::Font* _fontSmall;
				HCURSOR _grab, _grabbed;
				float _dpi;
		};
	}
}

#endif