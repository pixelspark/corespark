#ifndef _TJPROPERTYGRID_H
#define _TJPROPERTYGRID_H

namespace tj {
	namespace shared {
		class EXPORTED PropertyGridWnd: public ChildWnd {
			public:
				PropertyGridWnd(bool withPath=true);
				virtual ~PropertyGridWnd();
				void Update();
				void Inspect(ref<Inspectable> isp, ref<Path> p = ref<Path>());
				void Layout();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				void Clear();
				void SetNameWidth(Pixels w);
				virtual void OnScroll(ScrollDirection dir);
				virtual void OnSize(const Area& ar);
				void SetShowHints(bool h);
				bool GetShowHints() const;
				void FocusFirstProperty();

			protected:
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSettingsChanged();
				Pixels GetPathHeight() const;

				std::vector< ref<Property> > _properties;
				ref<TooltipWnd> _tw;
				ref<Inspectable> _subject;
				ref<PathWnd> _path;
				bool _isDraggingSplitter;
				bool _showHints;
				Pixels _nameWidth;
				Icon _expandIcon, _collapseIcon;
				MouseCapture _capture;

				const static Pixels KPathHeight;
				const static Pixels KMinimumNameColumnWidth;
				const static Pixels KPropertyMargin;
		};
	}
}

#endif