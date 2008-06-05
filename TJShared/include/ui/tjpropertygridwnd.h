#ifndef _TJPROPERTYGRID_H
#define _TJPROPERTYGRID_H

namespace tj {
	namespace shared {
		class EXPORTED PropertyGridWnd: public ChildWnd {
			public:
				PropertyGridWnd(bool withPath=true);
				virtual ~PropertyGridWnd();
				void Update();

				template<typename T> void AddProperty(std::string name, T* x) {
					GenericProperty<T>* p = new GenericProperty<T>(name,x);
					ShowWindow(p->Create(GetWindow()), SW_SHOW);
					_properties.push_back(GC::Hold(dynamic_cast<Property*>(p)));
					Layout();
				}

				void Inspect(ref<Inspectable> isp, ref<Path> p = 0);
				LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				void Layout();
				void ClearThemeCache();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				void Clear();
				void SetNameWidth(int w);
				virtual void OnScroll(ScrollDirection dir);
				virtual void OnSize(const Area& ar);
				void SetShowHints(bool h);
				bool GetShowHints() const;
				void FocusFirstProperty();

				HBRUSH _editBackground;
				HFONT _editFont;

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

				const static Pixels KPathHeight;
				const static Pixels KMinimumNameColumnWidth;
				const static Pixels KPropertyMargin;
		};
	}
}

#endif