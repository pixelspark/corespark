#ifndef _TJPANE_H
#define _TJPANE_H
#pragma warning(push)
#pragma warning(disable: 4251)

class RootWnd;

class EXPORTED Pane: public virtual Object {
	friend class TabWnd;
	friend class RootWnd;

	public:
		Pane(std::wstring, ref<Wnd>, bool detached, bool closable, std::wstring icon=L"");
		virtual ~Pane();
		ref<Wnd> GetWindow();
		Gdiplus::Bitmap* GetIcon();
		std::wstring GetTitle() const;
		bool IsClosable() const;
		virtual void SetTitle(std::wstring c);
		bool HasIcon() const;

	protected:
		std::wstring _title;
		ref<Wnd> _wnd;
		bool _detached;
		bool _fullscreen;
		bool _closable;
		Gdiplus::Bitmap* _icon;
};

class FloatingPane: public Wnd {
	friend class RootWnd;

	public:
		FloatingPane(RootWnd* rw, ref<Pane> pane, TabWnd* source);
		virtual ~FloatingPane();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void Layout();
		virtual void Paint(Gdiplus::Graphics& g);
		virtual wchar_t GetPreferredHotkey();

	protected:
		ref<Pane> _pane;
		TabWnd* _source;
		RootWnd* _root;
		bool _dragging;
};

#pragma warning(pop)
#endif