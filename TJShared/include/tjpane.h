#ifndef _TJPANE_H
#define _TJPANE_H
#pragma warning(push)
#pragma warning(disable: 4251)

class RootWnd;

class EXPORTED Pane: public virtual Object {
	friend class TabWnd;
	friend class RootWnd;

	public:
		Pane(std::wstring, ref<Wnd>, bool);
		virtual ~Pane();
		ref<Wnd> GetWindow();
		std::wstring GetTitle() const;

	protected:
		std::wstring _title;
		ref<Wnd> _wnd;
		bool _detached;
		bool _fullscreen;
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