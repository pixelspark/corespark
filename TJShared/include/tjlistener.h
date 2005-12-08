#ifndef _TJLISTENER_H
#define _TJLISTENER_H

enum Notification {
	NotificationNop = 0,
	NotificationUpdate,
};

class Wnd;

class EXPORTED Listener {
	public:
		virtual void Notify(Wnd* source, Notification evt) = 0;
};

#endif