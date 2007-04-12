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
				ToolbarWnd(HWND parent);
				virtual ~ToolbarWnd();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Paint(Gdiplus::Graphics& g);
				virtual void Layout();
				virtual void Add(ref<ToolbarItem> item);
				virtual void OnCommand(int c);
				virtual void Fill(LayoutFlags f, Area& r);
				virtual int GetTotalButtonWidth();
				virtual void SetBackground(bool t);
				virtual void SetBackgroundColor(Gdiplus::Color c);

			protected:
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				std::vector< ref<ToolbarItem> > _items;
				static const int KIconWidth = 16;
				static const int KIconHeight = 16;
				bool _in;
				int _idx;
				bool _bk;
				Gdiplus::Color _bkColor;
		};
	}
}

#endif