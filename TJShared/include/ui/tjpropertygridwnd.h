#ifndef _TJPROPERTYGRID_H
#define _TJPROPERTYGRID_H

namespace tj {
	namespace shared {
		class EXPORTED PropertyGridWnd: public ChildWnd {
			public:
				PropertyGridWnd(bool withPath=true);
				virtual ~PropertyGridWnd();
				void Update();
				void Inspect(ref<Inspectable> isp, ref<Path> p = 0);
				void Layout();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
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
				Pixels _nameWidth;
				ref<Inspectable> _subject;
				ref<PathWnd> _path;
				bool _isDraggingSplitter;
				bool _showHints;
				Icon _expandIcon, _collapseIcon;
				ref<TooltipWnd> _tw;

				const static Pixels KPathHeight;
				const static Pixels KMinimumNameColumnWidth;
				const static Pixels KPropertyMargin;
		};
	}
}

#endif