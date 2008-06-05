#ifndef _TJTHEME_H
#define _TJTHEME_H

namespace tj {
	namespace shared {
		class EXPORTED Icons {
			public:
				enum IconIdentifier {
					IconChecked = 0,
					IconColorChooser,
					IconFile,
					IconBack,
					IconForward,
					IconStop,
					IconReload,
					IconGo,
					IconRadioChecked,
					IconOK,
					IconPathSeparator,
					IconExpand,
					IconCollapse,
					IconTabAdd,
					IconTabClose,
					IconTip,
					IconSearch,
					IconDownArrow,
					IconGrabber,
					IconExceptionMessage,
					IconExceptionNotify,
					IconExceptionError,
					IconExceptionFatal,
					_IconLast,
				};

				static std::wstring GetIconPath(IconIdentifier i);

			protected:
				static const wchar_t* _paths[_IconLast];
		};

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
				virtual Area MeasureText(const std::wstring& text, graphics::Font* font) const;
				virtual graphics::Color GetLinkColor() const;
				virtual graphics::Color GetBackgroundColor() const;
				virtual graphics::Color GetVideoBackgroundColor() const;
				virtual graphics::Color GetEditBackgroundColor() const;
				virtual graphics::Color GetTimeBackgroundColor() const;
				virtual graphics::Color GetPropertyBackgroundColor() const;
				virtual graphics::Color GetLineColor() const;
				virtual graphics::Color GetTextColor() const;
				virtual graphics::Color GetActiveTrackColor() const;
				virtual graphics::Color GetTabButtonBackgroundColor() const;
				virtual graphics::Color GetHintColor() const;
				virtual graphics::Color GetFocusColor() const;

				virtual graphics::Color GetActiveStartColor() const;
				virtual graphics::Color GetActiveEndColor() const;

				virtual graphics::Color GetSplitterStartColor() const;
				virtual graphics::Color GetSplitterEndColor() const;

				virtual graphics::Color GetCurrentPositionColor() const;
				virtual graphics::Color GetTrackDescriptionTextColor() const;
				virtual graphics::Color GetFaderColor() const;

				virtual graphics::Color GetSliderColorStart(int i) const;
				virtual graphics::Color GetSliderColorEnd(int i) const;

				virtual graphics::Color GetTimeSelectionColorEnd() const;
				virtual graphics::Color GetTimeSelectionColorStart() const;

				virtual graphics::Color GetTabButtonColorStart() const;
				virtual graphics::Color GetTabButtonColorEnd() const;

				virtual graphics::Color GetDisabledOverlayColor() const;
				virtual graphics::Color GetCommandMarkerColor() const;

				virtual graphics::Color GetHighlightColorStart() const;
				virtual graphics::Color GetHighlightColorEnd() const;

				virtual graphics::Color GetGlassColorStart() const;
				virtual graphics::Color GetGlassColorEnd() const;

				virtual graphics::Color GetToolbarColorStart() const;
				virtual graphics::Color GetToolbarColorEnd() const;

				// ProgressWnd
				virtual graphics::Color GetProgressBackStart() const;
				virtual graphics::Color GetProgressBackEnd() const;
				virtual graphics::Color GetProgressGlassStart() const;
				virtual graphics::Color GetProgressGlassEnd() const;
				virtual graphics::Color GetProgressColor() const;

				// Fonts
				virtual graphics::Font* GetGUIFont() const;
				virtual graphics::Font* GetGUIFontBold() const;
				virtual graphics::Font* GetLinkFont() const;
				virtual graphics::Font* GetGUIFontSmall() const;

				// Cursors
				virtual HCURSOR GetGrabCursor() const;
				virtual HCURSOR GetGrabbedCursor() const;
				virtual graphics::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const;
				
				/* This needs a little explanation. This is the 'scale factor' to convert from 'logical pixels' 
				(which we define as one pixel on a normal, 96 DPI computer screen) to 'device pixels', which is
				what you use to paint in Windows. So when you're on a 96 DPI screen, this returns 1.0f. When you're
				on 120 dpi, this returns 1.25f. */
				virtual float GetDPIScaleFactor() const;

				// Measures
				virtual Pixels GetMeasureInPixels(Theme::Measure m) const;

				static graphics::Color ChangeAlpha(graphics::Color col, int a);
				
				enum {
					SliderNormal = 0,
					SliderSubmix,
					SliderMaster,
					SliderMacro,
					SliderAlpha,
				};

				const static float KDefaultDPI; // One logical pixel equals 1/KDefaultDPI inches

				// utility methods
				virtual void DrawToolbarBackground(graphics::Graphics& g, const Area& rc);
				virtual void DrawToolbarBackground(graphics::Graphics& g, float x, float y, float w, float h);
				virtual void DrawFocusRectangle(graphics::Graphics& g, const Area& rc);
				virtual void DrawFocusEllipse(graphics::Graphics& g, const Area& rc);

			protected:
				mutable graphics::Font* _font;
				mutable graphics::Font* _fontBold;
				mutable graphics::Font* _fontSmall;
				mutable graphics::Font* _fontLink;
				HCURSOR _grab, _grabbed;
				float _dpi;
		};
	}
}

#endif