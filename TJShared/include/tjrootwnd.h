#ifndef _TJROOTWND_H
#define _TJROOTWND_H

#pragma warning(push)
#pragma warning(disable: 4251)

class NotificationWnd;

class EXPORTED RootWnd: public Wnd {
	friend class NotificationWnd; 

	public:
		RootWnd(std::wstring title, const wchar_t* className=TJ_DEFAULT_CLASS_NAME, bool useDoubleBuffering=true);
		virtual ~RootWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		ref<FloatingPane> AddFloatingPane(ref<Pane> pane, TabWnd* source);
		void RemoveFloatingPane(ref<Pane> pn);
		virtual void Update();

		// TODO: use weak<TabWnd> ?
		void AddTabWindow(ref<TabWnd> tw);
		void RemoveTabWindow(ref<TabWnd> tw);
		void RemoveTabWindow(TabWnd* tw);
		void RevealWindow(ref<Wnd> wnd);
		ref<TabWnd> FindTabWindowAt(int x, int y);
		void SetDragTarget(ref<TabWnd> tw);
		ref<TabWnd> GetDragTarget();

		void AddOrphanPane(ref<Pane> pane);
		std::vector< ref<Pane> >* GetOrphanPanes();
		void RemoveOrphanPane(ref<Pane> pane);
		void RemoveWindow(ref<Wnd> w);
		void RenameWindow(ref<Wnd> w, std::wstring name);

		/* Notification API */
		void AddNotification(std::wstring message, std::wstring icon, int time=-1);
		void RemoveNotification(NotificationWnd* nw);
	protected:
		std::vector< ref<FloatingPane> > _floatingPanes;
		std::vector < ref<TabWnd> > _tabWindows;
		std::vector< ref<Pane> > _orphans;
		std::vector< ref<NotificationWnd> > _notifications;
		ref<TabWnd> _dragTarget;
};

class EXPORTED AddNotificationRunnable: public Runnable {
	public:
		AddNotificationRunnable(RootWnd* root, std::wstring text, std::wstring icon, int time);
		virtual ~AddNotificationRunnable();
		virtual void Run();

	protected:
		RootWnd* _root;
		std::wstring _text;
		std::wstring _icon;
		int _time;
};

#pragma warning(pop)
#endif