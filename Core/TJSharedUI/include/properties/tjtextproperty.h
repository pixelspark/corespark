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
 
 #ifndef _TJTEXTPROPERTY_H
#define _TJTEXTPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED TextProperty: public GenericProperty<String> {
			public:
				TextProperty(const String& name, ref<Inspectable> holder, String* value, Pixels height = 100);
				virtual ~TextProperty();
				virtual Pixels GetHeight();

			protected:
				Pixels _height;
		};

		class EXPORTED SuggestionProperty: public Property, public Listener<EditWnd::EditingNotification> {
			public:
				SuggestionProperty(const String& name, ref<Inspectable> holder, String* value, bool multiLine = false);
				virtual ~SuggestionProperty();
				virtual void Notify(ref<Object> source, const EditWnd::EditingNotification& ev);
				virtual ref<Wnd> GetWindow();
				virtual void Update();
				virtual Pixels GetHeight();
				virtual strong<Menu> GetSuggestionMenu();
				virtual void SetSuggestionMode(SuggestionEditWnd::SuggestionMode sm);

			protected:
				weak<Inspectable> _holder;
				String* _value;
				String _oldValue;
				ref<SuggestionEditWnd> _wnd;
				bool _multiLine;
		};
	}
}

#endif