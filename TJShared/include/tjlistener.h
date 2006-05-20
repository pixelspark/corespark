#ifndef _TJLISTENER_H
#define _TJLISTENER_H

enum Notification {
	NotificationNop = 0,
	NotificationChanged,
	NotificationUpdate,
	NotificationClick,
};

class Wnd;

class EXPORTED Listener {
	public:
		virtual void Notify(Wnd* source, Notification evt) = 0;
};

class EXPORTED ListenerWrapper: public Listener {
	public:
		ListenerWrapper(Listener* lp);
		virtual ~ListenerWrapper();
		virtual void Notify(Wnd* source, Notification evt);

	protected:
		Listener* _listener;
};

#endif