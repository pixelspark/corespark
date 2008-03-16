#ifndef _TJTOOLBARWND_H
#define _TJTOOLBARNWD_H

namespace tj {
	namespace shared {

		class EXPORTED ToolbarItem {
			public:
				ToolbarItem(int command=0, Gdiplus::Bitmap* bmp=0, std::wstring text=L"", bool separator=false);
				ToolbarItem(int command, std::wstring icon, std::wstring text=L"", bool separator=false);
				~ToolbarItem();
				bool IsSeparator() const;
				void SetSeparator(bool s);
				virtual int GetCommand() const;
				virtual Icon& GetIcon();
				virtual std::wstring GetText() const;

			protected:
				int _command;
				Icon _icon;
				bool _separator;
				std::wstring _text;
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
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual void Layout();
				virtual void Add(ref<ToolbarItem> item);
				virtual void OnCommand(int c);
				virtual void Fill(LayoutFlags f, Area& r, bool direct = true);
				virtual Pixels GetTotalButtonWidth() const;
				virtual void SetBackground(bool t);
				virtual void SetBackgroundColor(Gdiplus::Color c);
				virtual bool HasTip() const;
				virtual void SetTip(ref<Wnd> tipWindow);

			protected:
				virtual Pixels GetButtonX(int command);
				virtual Area GetFreeArea() const; // returns the area which child classes can freely use to paint on
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSize(const Area& ns);
				virtual bool CanShowHints();
				virtual void DrawToolbarButton(Gdiplus::Graphics& g, Pixels x, Icon& icon, const Area& rc, ref<Theme> theme, bool over, bool down, bool separator=false);

				std::vector< ref<ToolbarItem> > _items;
				static const int KIconWidth = 16;
				static const int KIconHeight = 16;
				bool _in;
				int _idx;
				bool _bk;
				Gdiplus::Color _bkColor;
				Icon _tipIcon;
				ref<Wnd> _tip;
		};

		class EXPORTED SearchToolbarWnd: public ToolbarWnd, public Listener {
			public:
				SearchToolbarWnd();
				virtual ~SearchToolbarWnd();
				virtual void Layout();
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual void Notify(Wnd* src, Notification n);

			protected:
				virtual void SetSearchBoxRightMargin(Pixels r);
				virtual void OnSearchChange(const std::wstring& q);
				virtual void OnCreated();
				Area GetSearchBoxArea() const;
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
