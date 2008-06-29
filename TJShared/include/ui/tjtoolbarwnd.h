#ifndef _TJTOOLBARWND_H
#define _TJTOOLBARNWD_H

namespace tj {
	namespace shared {

		class EXPORTED ToolbarItem: public Element {
			public:
				ToolbarItem(int command=0, graphics::Bitmap* bmp=0, std::wstring text=L"", bool separator=false);
				ToolbarItem(int command, std::wstring icon, std::wstring text=L"", bool separator=false);
				~ToolbarItem();
				bool IsSeparator() const;
				void SetSeparator(bool s);
				virtual int GetCommand() const;
				virtual Icon& GetIcon();
				virtual std::wstring GetText() const;
				void SetEnabled(bool e);
				bool IsEnabled() const;
				void SetActive(bool e);
				bool IsActive() const;
				virtual Area GetPreferredSize() const;
				void SetPreferredSize(Pixels w, Pixels h);

				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme, bool over, bool down);

				static void DrawToolbarButton(graphics::Graphics& g, Icon& icon, const Area& rc, ref<Theme> theme, bool over, bool down, bool separator=false, bool enabled = true);
				static void DrawToolbarButton(graphics::Graphics& g, const Area& rc, ref<Theme> theme, bool over, bool down, bool separator=false);

			protected:
				int _command;
				Icon _icon;
				bool _separator;
				bool _enabled;
				bool _active;
				std::wstring _text;
				Pixels _preferredWidth, _preferredHeight;
		};

		class EXPORTED StateToolbarItem: public ToolbarItem {
			public:
				StateToolbarItem(int command, std::wstring iconOn, std::wstring iconOff, std::wstring text=L"");
				virtual ~StateToolbarItem();
				virtual void SetState(bool on);
				virtual Icon& GetIcon();
				bool IsOn() const;

			protected:
				bool _on;
				Icon _onIcon;
		};

		class EXPORTED ToolbarWnd: public ChildWnd {
			public:
				ToolbarWnd();
				virtual ~ToolbarWnd();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void Layout();
				virtual void Add(ref<ToolbarItem> item, bool alignRight = false);
				virtual void OnCommand(ref<ToolbarItem> item) = 0;
				virtual void Fill(LayoutFlags f, Area& r, bool direct = true);
				virtual void SetBackground(bool t);
				virtual void SetBackgroundColor(graphics::Color c);
				virtual bool HasTip() const;
				virtual void SetTip(ref<Wnd> tipWindow);

			protected:
				virtual Pixels GetButtonX(int command);
				virtual Area GetFreeArea() const; // returns the area which child classes can freely use to paint on
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSize(const Area& ns);
				virtual bool CanShowHints();
				
				std::vector< ref<ToolbarItem> > _items;
				std::vector< ref<ToolbarItem> > _itemsRight;
				Area _freeArea;

				static const int KIconWidth = 16;
				static const int KIconHeight = 16;

				bool _in;
				ref<ToolbarItem> _over;
				bool _bk;
				graphics::Color _bkColor;
				Icon _tipIcon;
				ref<Wnd> _tip;
		};

		class EXPORTED SearchToolbarWnd: public ToolbarWnd, public Listener<EditWnd::NotificationTextChanged> {
			public:
				SearchToolbarWnd();
				virtual ~SearchToolbarWnd();
				virtual void Layout();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void Notify(ref<Object> src, const EditWnd::NotificationTextChanged& data);

			protected:
				virtual void SetSearchBoxRightMargin(Pixels r);
				virtual void OnSearchChange(const std::wstring& q);
				virtual void OnCreated();
				Area GetSearchBoxArea() const;
				bool IsSearchBoxVisible() const;
				virtual void SetSearchBoxSize(Pixels w, Pixels h);
				virtual bool CanShowHints();

			private:
				const static Pixels KDefaultBoxWidth = 100;
				const static Pixels KDefaultBoxHeight = 16;

				ref<EditWnd> _edit;
				Icon _searchIcon;
				Pixels _rightMargin, _searchWidth, _searchHeight;
		};
	}
}

#endif
