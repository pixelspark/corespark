#ifndef _TJTOOLBARWND_H
#define _TJTOOLBARNWD_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED ToolbarItem {
	public:
		ToolbarItem(int command=0, Gdiplus::Bitmap* bmp=0, std::wstring text=L"", bool separator=false);
		ToolbarItem(int command, std::wstring icon, std::wstring text=L"", bool separator=false);
		~ToolbarItem();
		bool IsSeparator() const;
		void SetSeparator(bool s);
		virtual int GetCommand() const;
		virtual Gdiplus::Bitmap* GetIcon();
		virtual std::wstring GetText() const;

	protected:
		int _command;
		Gdiplus::Bitmap* _icon;
		bool _separator;
		std::wstring _text;
};

class EXPORTED StateToolbarItem: public ToolbarItem {
	public:
		StateToolbarItem(int command, std::wstring iconOn, std::wstring iconOff, std::wstring text=L"");
		virtual ~StateToolbarItem();
		virtual void SetState(bool on);
		virtual Gdiplus::Bitmap* GetIcon();
		bool IsOn() const;

	protected:
		bool _on;
		Gdiplus::Bitmap* _onImage;
};

class EXPORTED ToolbarWnd: public ChildWnd {
	public:
		ToolbarWnd(HWND parent);
		virtual ~ToolbarWnd();
		virtual wchar_t GetPreferredHotkey();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void Paint(Gdiplus::Graphics& g);
		virtual void Layout();
		virtual void Add(ref<ToolbarItem> item);
		virtual void OnCommand(int c);
		virtual void Fill(LayoutFlags f, tj::shared::Rectangle& r);
		virtual int GetTotalButtonWidth();

	protected:
		std::vector< ref<ToolbarItem> > _items;
		static const int KButtonSize = 24;
		static const int KIconWidth = 16;
		static const int KIconHeight = 16;
		bool _in;
		int _idx;
};

#pragma warning(pop)

#endif