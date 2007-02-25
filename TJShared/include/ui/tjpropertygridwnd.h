#ifndef _TJPROPERTYGRID_H
#define _TJPROPERTYGRID_H

namespace tj {
	namespace shared {
		class EXPORTED PropertyGridWnd: public ChildWnd {
			public:
				PropertyGridWnd(HWND parent);
				virtual ~PropertyGridWnd();
				void Update();

				template<typename T> void AddProperty(std::string name, T* x) {
					GenericProperty<T>* p = new GenericProperty<T>(name,x);
					ShowWindow(p->Create(GetWindow()), SW_SHOW);
					_properties.push_back(GC::Hold(dynamic_cast<Property*>(p)));
					Layout();
				}

				void Inspect(ref<Inspectable> isp, ref<Path> p = 0);
				void Inspect(Inspectable* isp, ref<Path> p = 0);
				LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				void Layout();
				void ClearThemeCache();
				virtual void Paint(Gdiplus::Graphics& g);
				void Clear();

				HBRUSH _editBackground;
				HFONT _editFont;

			protected:
				std::vector< ref<Property> > _properties;
				int _nameWidth;
				ref<Inspectable> _subject;
				ref<PathWnd> _path;
				bool _isDraggingSplitter;

				const static int KPathHeight = 22;
		};
	}
}

#endif