#ifndef _TJROOTWND_H
#define _TJROOTWND_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED RootWnd: public Wnd {
	public:
		RootWnd(std::wstring title);
		virtual ~RootWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual ref<FloatingPane> AddFloatingPane(ref<Pane> pane, TabWnd* source);
		virtual void RemoveFloatingPane(ref<Pane> pn);

		// TODO: use weak<TabWnd> ?
		virtual void AddTabWindow(ref<TabWnd> tw);
		virtual void RemoveTabWindow(ref<TabWnd> tw);
		virtual void RevealWindow(ref<Wnd> wnd);

	protected:
		std::vector< ref<FloatingPane> > _floatingPanes;
		std::vector < ref<TabWnd> > _tabWindows;
};

#pragma warning(pop)
#endif