/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
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
					IconSubMenu,
					IconTabAddActive,
					IconTabCloseActive,
					IconKeyboard,
					IconUnchecked,
					_IconLast,
				};

				static String GetIconPath(IconIdentifier i);

			protected:
				static const wchar_t* _paths[_IconLast];
		};

		class EXPORTED Theme: public virtual Object {
			public:
				Theme(float dpi);
				virtual ~Theme();
				virtual String GetName() const;

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
				virtual Area MeasureText(const String& text, graphics::Font* font) const;
				virtual String GetGUIFontName() const;

				#ifdef TJ_OS_WIN
					virtual graphics::Brush* GetApplicationBackgroundBrush(HWND root, HWND child) const;
				#endif
			
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
				float _dpi;
		};

		class EXPORTED TokenizedTextPainter {
			public:
				TokenizedTextPainter(const Area& rc, strong<Theme> theme);
				~TokenizedTextPainter();
				void DrawToken(graphics::Graphics& g, const std::wstring& txt, bool bold, graphics::Brush* textBrush, graphics::Brush* backgroundBrush = 0, graphics::Pen* borderPen = 0, bool shadow = false);
				void DrawToken(graphics::Graphics& g, const std::wstring& txt, graphics::Font* font, graphics::Brush* textBrush, graphics::Brush* backgroundBrush = 0, graphics::Pen* borderPen = 0, graphics::Brush* shadowBrush = 0);
				void SetMargin(const Pixels& m);
				Pixels GetMargin() const;

			protected:
				Area _rc;
				strong<Theme> _theme;
				Pixels _margin;

			private:
				const static Pixels KDefaultMargin;
		};
	}
}

#endif