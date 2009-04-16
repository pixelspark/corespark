#ifndef _TJSPLITTER_H
#define _TJSPLITTER_H

namespace tj {
	namespace shared {
		enum Orientation {
			OrientationHorizontal=0,
			OrientationVertical,
		};

		class EXPORTED SplitterWnd: public ChildWnd {
			friend class ChildEnumerator;

			public:
				enum CollapseMode {
					CollapseNone = 0,	// Not collapsed, area is divided by 'ratio'
					CollapseFirst,		// First pane takes up all space (left/top)
					CollapseSecond,		// Second pane takes all space (right/bottom)
				};

				enum ResizeMode {
					ResizeModeEqually = 0,	// When the splitter is resized, both windows are resized
					ResizeModeRightOrBottom,// When the splitter is resized, the right or bottom window resizes, the other stays the same size
					ResizeModeLeftOrTop,	// When the splitter is resized, the left or top window resizes, the other stays the same size
				};

				SplitterWnd(Orientation o);
				virtual ~SplitterWnd();
				virtual void Layout();
				virtual void Update();
				void SetRatio(float f);
				virtual bool IsSplitter();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				void Collapse(CollapseMode cm = CollapseFirst);
				void Expand();
				void SetOrientation(Orientation o);
				Orientation GetOrientation() const;
				void SetResizeMode(ResizeMode rm);
				ResizeMode GetResizeMode() const;

				// Do not use, use SetFirst and SetSecond instead (Add will throw an exception)
				virtual void Add(ref<Wnd> child);
				virtual void SetFirst(ref<Wnd> child);
				virtual void SetSecond(ref<Wnd> child);
				ref<Wnd> GetFirst();
				ref<Wnd> GetSecond();
				const ref<Wnd> GetFirst() const;
				const ref<Wnd> GetSecond() const;
				virtual String GetTabTitle() const;
				bool IsCollapsed() const;

			protected:
				virtual Pixels GetWidthOfFirstPane(Pixels totalWidth);
				virtual Pixels GetWidthOfSecondPane(Pixels totalWidth);
				void SetWidthOfFirstPane(Pixels paneWidth, Pixels totalWidth);
				void SetWidthOfSecondPane(Pixels paneHeight, Pixels totalWidth);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSize(const Area& ns);
				virtual void OnSettingsChanged();
				virtual Area GetBarArea();

				const static Pixels KBarHeight;
				const static float KSnapMargin;

			private:
				ref<Wnd> _a;
				ref<Wnd> _b;
				float _ratio;
				float _ratioBeforeDragging;
				Pixels _currentWidth;
				float _defaultRatio;
				bool _dragging;
				CollapseMode _collapse;
				Orientation _orientation;
				ResizeMode _resizeMode;
				MouseCapture _capture;
		};
	}
}

#endif