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

				SplitterWnd(Orientation o);
				virtual ~SplitterWnd();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Layout();
				virtual void Update();
				void SetRatio(float f);
				virtual bool IsSplitter();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				void Collapse(CollapseMode cm = CollapseFirst);
				void Expand();
				void SetOrientation(Orientation o);
				Orientation GetOrientation() const;

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
				virtual void OnSettingsChanged();
				virtual Area GetBarArea();

				const static Pixels KBarHeight;
				const static float KSnapMargin;

			private:
				ref<Wnd> _a;
				ref<Wnd> _b;
				float _ratio;
				float _ratioBeforeDragging;
				float _defaultRatio;
				bool _dragging;
				CollapseMode _collapse;
				Orientation _orientation;
		};
	}
}

#endif