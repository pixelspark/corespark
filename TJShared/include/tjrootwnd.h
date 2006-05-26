#ifndef _TJROOTWND_H
#define _TJROOTWND_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED RootWnd: public Wnd {
	public:
		RootWnd(std::wstring title);
		virtual ~RootWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		ref<FloatingPane> AddFloatingPane(ref<Pane> pane, TabWnd* source);
		void RemoveFloatingPane(ref<Pane> pn);

		// TODO: use weak<TabWnd> ?
		void AddTabWindow(ref<TabWnd> tw);
		void RemoveTabWindow(ref<TabWnd> tw);
		void RevealWindow(ref<Wnd> wnd);
		ref<TabWnd> FindTabWindowAt(int x, int y);
		void SetDragTarget(ref<TabWnd> tw);
		ref<TabWnd> GetDragTarget();

		void AddOrphanPane(ref<Pane> pane);
		std::vector< ref<Pane> >* GetOrphanPanes();
		void RemoveOrphanPane(ref<Pane> pane);
	protected:
		std::vector< ref<FloatingPane> > _floatingPanes;
		std::vector < ref<TabWnd> > _tabWindows;
		std::vector< ref<Pane> > _orphans;
		ref<TabWnd> _dragTarget;
};

#pragma warning(pop)
#endif