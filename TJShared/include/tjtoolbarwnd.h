#ifndef _TJTOOLBARWND_H
#define _TJTOOLBARNWD_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED ToolbarItem {
	public:
		ToolbarItem(int command=0, Gdiplus::Bitmap* bmp=0);
		~ToolbarItem();
		bool IsSeparator() const;
		void SetSeparator(bool s);
		int _command;
		Gdiplus::Bitmap* _icon;
		bool _separator;
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