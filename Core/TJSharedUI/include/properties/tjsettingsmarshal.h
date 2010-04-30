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
 
 #ifndef _TJ_SETTINGSMARSHAL_H
#define _TJ_SETTINGSMARSHAL_H

namespace tj {
	namespace shared {
		/** SettingsMarshal is a temporary setting value that is changed by the property grid. When the settings window is closed,
		 the marshal is destructed and will save the new value to Settings */
		template<typename T> class SettingsMarshal {
		public:
			SettingsMarshal(ref<Settings> st, const String& key) {
				_value = StringTo<T>(st->GetValue(key), T());
				_settings = st;
				_key = key;
			}
			
			SettingsMarshal(ref<Settings> st, const String& key, const T& defaultValue) {
				_value = StringTo<T>(st->GetValue(key, Stringify(defaultValue)), T());
				_settings = st;
				_key = key;
			}
			
			~SettingsMarshal() {
				_settings->SetValue(_key, Stringify(_value));
			}
			
			bool IsChanged() const {
				return StringTo<T>(_settings->GetValue(_key), _value)!=_value;
			}
			
			inline ref<Property> CreateProperty(const String& title, ref<Inspectable> holder, const String& hint = L"") {
				ref<Property> p = GC::Hold(new GenericProperty<T>(title, holder, &_value, _value));
				if(hint.length()>0) {
					p->SetHint(hint);
				}
				return p;
			}
			
			T _value;
			
		protected:
			ref<Settings> _settings;
			String _key;
		};
		
		
		/* Boolean flags are handled by Settings in a separate way */
		template<> SettingsMarshal<bool>::SettingsMarshal(ref<Settings> st, const String& key) {
			_value = st->GetFlag(key);
			_settings = st;
			_key = key;
		}
		
		template<> SettingsMarshal<bool>::SettingsMarshal(ref<Settings> st, const String& key, const bool& defaultVal) {
			_value = st->GetFlag(key, defaultVal);
			_settings = st;
			_key = key;
		}
		
		template<> SettingsMarshal<bool>::~SettingsMarshal() {
			_settings->SetFlag(_key, _value);
		}
	}
}

#endif