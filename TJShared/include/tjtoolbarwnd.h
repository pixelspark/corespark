#ifndef _TJTOOLBARWND_H
#define _TJTOOLBARNWD_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED ToolbarItem {
	public:
		ToolbarItem(int command=0, Gdiplus::Bitmap* bmp=0);
		ToolbarItem(int command, std::wstring icon);
		~ToolbarItem();
		bool IsSeparator() const;
		void SetSeparator(bool s);
		virtual int GetCommand() const;
		virtual Gdiplus::Bitmap* GetIcon();

	protected:
		int _command;
		Gdiplus::Bitmap* _icon;
		bool _separator;
};

class EXPORTED StateToolbarItem: public ToolbarItem {
	public:
		StateToolbarItem(int command, std::wstring iconOn, std::wstring iconOff);
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

	protected:
		std::vector< ref<ToolbarItem> > _items;
		static const int ButtonSize = 24;
		bool _in;

};

#pragma warning(pop)

#endif