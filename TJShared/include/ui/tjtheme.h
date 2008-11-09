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
					IconCopy,
					IconCut,
					IconPaste,
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
					MeasureNone = 0,
					MeasureToolbarHeight,
					MeasureListItemHeight,
					MeasureMenuItemHeight,
					MeasureListHeaderHeight,
					MeasureMaximumSnappingDistance,
					MeasureShadowSize,
					MeasurePropertyHeight,
					MeasureDialogHeaderHeight,
					MeasureSliderDraggerWidth,
					MeasureMinimumContextMenuWidth,
					_MeasureLast,
				};

				enum SliderType {
					SliderNormal = 0,
					SliderSubmix,
					SliderMaster,
					SliderMacro,
					SliderAlpha,
				};

				enum ColorIdentifier {
					ColorNone = 0,

					ColorActiveEnd,
					ColorActiveStart,
					ColorActiveTrack,
					ColorBackground,
					ColorCommandMarker,
					ColorCurrentPosition,
					ColorDescriptionText,
					ColorDisabledOverlay,
					ColorEditBackground,
					ColorFader,
					ColorFocus,
					ColorGlassStart,
					ColorGlassEnd,
					ColorHighlightStart,
					ColorHighlightEnd,
					ColorHint,
					ColorLink,
					ColorLine,
					ColorProgress,
					ColorProgressBackgroundEnd,
					ColorProgressBackgroundStart,
					ColorProgressGlassEnd,
					ColorProgressGlassStart,
					ColorPropertyBackground,
					ColorShadow,
					ColorShadowed,
					ColorSplitterEnd,
					ColorSplitterStart,
					ColorTabButtonBackground,
					ColorTabButtonEnd,
					ColorTabButtonStart,
					ColorText,
					ColorTimeBackground,
					ColorTimeSelectionEnd,
					ColorTimeSelectionStart,
					ColorToolbarEnd,
					ColorToolbarStart,
					ColorVideoBackground,
					_ColorLast,
				};

				// Colors
				virtual graphics::Color GetColor(const ColorIdentifier& ci) const;
				static graphics::Color ChangeAlpha(const graphics::Color& col, int a);
				static graphics::Color ChangeAlpha(const graphics::Color& col, float a);
				virtual graphics::Color GetSliderColorStart(const SliderType& i) const;
				virtual graphics::Color GetSliderColorEnd(const SliderType& i) const;

				// Fonts & text
				virtual graphics::Font* GetGUIFont() const;
				virtual graphics::Font* GetGUIFontBold() const;
				virtual graphics::Font* GetLinkFont() const;
				virtual graphics::Font* GetGUIFontSmall() const;
				virtual Area MeasureText(const std::wstring& text, graphics::Font* font) const;

				// Cursors
				virtual HCURSOR GetGrabCursor() const;
				virtual HCURSOR GetGrabbedCursor() const;
				virtual graphics::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const;
				
				/* This needs a little explanation. This is the 'scale factor' to convert from 'logical pixels' 
				(which we define as one pixel on a normal, 96 DPI computer screen) to 'device pixels', which is
				what you use to paint in Windows. So when you're on a 96 DPI screen, this returns 1.0f. When you're
				on 120 dpi, this returns 1.25f. */
				virtual float GetDPIScaleFactor() const;
				const static float KDefaultDPI; // One logical pixel equals 1/KDefaultDPI inches

				// Measures
				virtual Pixels GetMeasureInPixels(Theme::Measure m) const;
				
				// utility methods
				virtual void DrawInsetRectangle(graphics::Graphics& g, const Area& rc);
				virtual void DrawInsetRectangleLight(graphics::Graphics& g, const Area& rc);
				virtual void DrawToolbarBackground(graphics::Graphics& g, const Area& rc);
				virtual void DrawToolbarBackground(graphics::Graphics& g, const Area& rc, float alpha);
				virtual void DrawToolbarBackground(graphics::Graphics& g, float x, float y, float w, float h);
				virtual void DrawToolbarBackground(graphics::Graphics& g, float x, float y, float w, float h, float alpha);
				virtual void DrawFocusRectangle(graphics::Graphics& g, const Area& rc, float alpha = 1.0f);
				virtual void DrawShadowRectangle(graphics::Graphics& g, const Area& rc, float alpha = 1.0f);
				virtual void DrawFocusEllipse(graphics::Graphics& g, const Area& rc, float alpha = 1.0f);
				virtual void DrawHighlightEllipse(graphics::Graphics& g, const Area& c, float alpha = 1.0f);
				virtual void DrawMessageBar(graphics::Graphics& g, const Area& c);

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