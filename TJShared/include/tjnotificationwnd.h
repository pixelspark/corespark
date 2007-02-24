#ifndef _TJNOTIFICATIONWND_H
#define _TJNOTIFICATIONWND_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED NotificationWnd: public Wnd {
	public:
		NotificationWnd(std::wstring text, std::wstring icon, int time, int h, RootWnd* parent);
		virtual ~NotificationWnd();
		virtual void Show(bool t);
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void Paint(Gdiplus::Graphics& g);
		int GetIndex();

		enum {
			NotificationTimeoutForever = 0,
			NotificationTimeoutDefault = -1,
		};
		
	protected:
		std::wstring _text;
		Gdiplus::Bitmap* _icon;
		RootWnd* _root;
		int _index;

		const static int DefaultNotificationHeight;
		const static int DefaultNotificationMargin;	
		const static int DefaultNotificationTimeout;
};

#pragma warning(pop)

#endif