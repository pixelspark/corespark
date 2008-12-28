#ifndef _TJIMAGEBUTTON_H
#define _TJIMAGEBUTTON_H

namespace tj {
	namespace shared {
		class EXPORTED ButtonWnd: public ChildWnd {
			public:
				ButtonWnd(const ResourceIdentifier& iconRid, const std::wstring& text = L"");
				virtual ~ButtonWnd();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void Fill(LayoutFlags lf, Area& rect, bool direct = true);
				virtual void SetText(const wchar_t* t);
				virtual void SetDisabled(bool d);
				virtual bool IsDisabled() const;

				struct NotificationClicked {
				};

				Listenable<NotificationClicked> EventClicked;
				
			protected:
				virtual void OnKey(Key k, wchar_t t, bool down, bool isAccelerator);
				virtual void OnFocus(bool f);

				std::wstring _text;
				Icon _icon;
				bool _down;
				bool _disabled;
		};

		class EXPORTED StateButtonWnd: public ButtonWnd {
			public:
				enum ButtonState {On, Off, Other};
				StateButtonWnd(const ResourceIdentifier& imageOn, const ResourceIdentifier& imageOff, const ResourceIdentifier& imageOther);
				virtual ~StateButtonWnd();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				void SetOn(ButtonState o);
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				
			protected:
				Icon _offIcon;
				Icon _otherIcon;
				ButtonState _on;
		};
	}
}

#endif