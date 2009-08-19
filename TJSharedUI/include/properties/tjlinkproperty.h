#ifndef _TJLINKPROPERTY_H
#define _TJLINKPROPERTY_H


namespace tj {
	namespace shared {
		class LinkPropertyWnd;

		class EXPORTED LinkProperty: public Property {
			public:
				LinkProperty(const String& name, const String& text, const String& iconrid = L"");
				virtual ~LinkProperty();
				virtual ref<Wnd> GetWindow();
				virtual void Update();
				virtual void OnClicked();
				virtual void SetText(const String& text);

				Listenable<ButtonWnd::NotificationClicked> EventClicked;

			protected:
				ref<LinkPropertyWnd> _wnd;
				String _text;
				String _icon;
		};

		class EXPORTED LinkPropertyWnd: public ButtonWnd, public Listener<ButtonWnd::NotificationClicked> {
			public:
				LinkPropertyWnd(const String& text, ref<LinkProperty> lp, const String& icon);
				virtual ~LinkPropertyWnd();
				virtual void OnCreated();
				virtual void Notify(ref<Object> source, const ButtonWnd::NotificationClicked& nc);

			protected:
				weak<LinkProperty> _lp;
		};
	}
}

#endif