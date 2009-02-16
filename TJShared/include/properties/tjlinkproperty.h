#ifndef _TJLINKPROPERTY_H
#define _TJLINKPROPERTY_H


namespace tj {
	namespace shared {
		class LinkPropertyWnd;

		class EXPORTED LinkProperty: public Property {
			public:
				LinkProperty(const std::wstring& name, const std::wstring& text, const std::wstring& iconrid = L"");
				virtual ~LinkProperty();
				virtual ref<Wnd> GetWindow();
				virtual void Update();
				virtual void OnClicked();
				virtual void SetText(const std::wstring& text);

				Listenable<ButtonWnd::NotificationClicked> EventClicked;

			protected:
				ref<LinkPropertyWnd> _wnd;
				std::wstring _text;
				std::wstring _icon;
		};

		class EXPORTED LinkPropertyWnd: public ButtonWnd, public Listener<ButtonWnd::NotificationClicked> {
			public:
				LinkPropertyWnd(const std::wstring& text, ref<LinkProperty> lp, const std::wstring& icon);
				virtual ~LinkPropertyWnd();
				virtual void OnCreated();
				virtual void Notify(ref<Object> source, const ButtonWnd::NotificationClicked& nc);

			protected:
				weak<LinkProperty> _lp;
		};
	}
}

#endif