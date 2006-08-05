#ifndef _TJPROPERTYGRID_H
#define _TJPROPERTYGRID_H
#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED PropertyGridWnd: public ChildWnd {
	public:
		PropertyGridWnd(HWND parent);
		virtual ~PropertyGridWnd();
		void Update();

		template<typename T> void AddProperty(std::string name, T* x) {
			GenericProperty<T>* p = new GenericProperty<T>(name,x);
			ShowWindow(p->Create(_wnd), SW_SHOW);
			_properties.push_back(GC::Hold(dynamic_cast<Property*>(p)));
			Layout();
		}

		void Inspect(ref<Inspectable> isp);
		void Inspect(Inspectable* isp);
		inline HWND GetWindow() { return _wnd; }
		LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		void Layout();
		void ClearThemeCache();
		virtual wchar_t GetPreferredHotkey();
		void EnterHotkeyMode();
		void LeaveHotkeyMode(wchar_t key=L'\0');
		virtual void Paint(Gdiplus::Graphics& g);
		void Clear();

		HBRUSH _editBackground;
		HFONT _editFont;

	protected:
		std::vector< ref<Property> > _properties;
		int _nameWidth;
		ref<Inspectable> _subject;
		bool _isDraggingSplitter;
};

#pragma warning(pop)
#endif