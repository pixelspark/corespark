/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJIMAGEBUTTON_H
#define _TJIMAGEBUTTON_H

namespace tj {
	namespace shared {
		class EXPORTED ButtonWnd: public ChildWnd {
			public:
				ButtonWnd(const ResourceIdentifier& iconRid, const String& text = L"");
				virtual ~ButtonWnd();
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
				virtual void OnSize(const Area& ns);

				String _text;
				Icon _icon;
				bool _down;
				bool _disabled;
		};

		class EXPORTED StateButtonWnd: public ButtonWnd {
			public:
				enum ButtonState {On, Off, Other};
				StateButtonWnd(const ResourceIdentifier& imageOn, const ResourceIdentifier& imageOff, const ResourceIdentifier& imageOther);
				virtual ~StateButtonWnd();
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