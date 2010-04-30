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