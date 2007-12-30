#ifndef _TJLISTENER_H
#define _TJLISTENER_H

namespace tj {
	namespace shared {
		enum Notification {
			NotificationNop = 0,
			NotificationChanged,
			NotificationUpdate,
			NotificationClick,
		};

		class Wnd;

		/** Listener **/
		class EXPORTED Listener {
			public:
				virtual ~Listener();
				virtual void Notify(Wnd* source, Notification evt) = 0;
		};
	}
}

#endif