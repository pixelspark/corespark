#ifndef _TJSHARED_SETTINGS_H
#define _TJSHARED_SETTINGS_H

namespace tj {
	namespace shared {
		class Property;

		class EXPORTED Settings: public virtual Object {
			public:
				virtual ~Settings();
				virtual bool GetFlag(const std::wstring& key) const = 0;
				virtual bool GetFlag(const std::wstring& key, bool defaultValue) const = 0;
				virtual std::wstring GetValue(const std::wstring& key) const = 0;
				virtual std::wstring GetValue(const std::wstring& key, const std::wstring& defaultValue) const = 0;
				virtual void SetValue(const std::wstring& key, const std::wstring& value) = 0;
				virtual void SetFlag(const std::wstring& key, bool t) = 0;
				virtual ref<Settings> GetNamespace(const std::wstring& ns) = 0;
		};

		class EXPORTED SettingsStorage: public Settings {
			public:
				SettingsStorage();
				virtual ~SettingsStorage();
				virtual void Load(const std::wstring& path);
				virtual void Save(const std::wstring& path) const;

				// Settings implementation
				virtual bool GetFlag(const std::wstring& key) const;
				virtual bool GetFlag(const std::wstring& key, bool defaultValue) const;
				virtual std::wstring GetValue(const std::wstring& key) const;
				virtual void SetValue(const std::wstring& key, const std::wstring& value);
				virtual void SetFlag(const std::wstring& key, bool t);
				virtual ref<Settings> GetNamespace(const std::wstring& ns);
				virtual std::wstring GetValue(const std::wstring& key, const std::wstring& defaultValue) const;

				// Utility functions
				static std::wstring GetSettingsPath(const std::wstring& vendor, const std::wstring& app, const std::wstring& file);

			protected:
				std::map< std::wstring, std::wstring > _data;
				CriticalSection _lock;
		};

		/** SettingsMarshal is a temporary setting value that is changed by the property grid. When the settings window is closed,
		the marshal is destructed and will save the new value to Settings */
		template<typename T> class SettingsMarshal {
			public:
				SettingsMarshal(ref<Settings> st, const std::wstring& key) {
					_value = StringTo<T>(st->GetValue(key), T());
					_settings = st;
					_key = key;
				}

				SettingsMarshal(ref<Settings> st, const std::wstring& key, const T& defaultValue) {
					_value = StringTo<T>(st->GetValue(key, Stringify(defaultValue)), T());
					_settings = st;
					_key = key;
				}

				~SettingsMarshal() {
					_settings->SetValue(_key, Stringify(_value));
				}

				ref<Property> CreateProperty(const std::wstring& title, const std::wstring& hint=L"") {
					ref<Property> p = GC::Hold(new GenericProperty<T>(title, &_value, 0, _value));
					if(hint.length()>0) {
						p->SetHint(hint);
					}
					return p;
				}

				T _value;

			protected:
				ref<Settings> _settings;
				std::wstring _key;
		};

		/* Boolean flags are handled by Settings in a separate way */
		template<> SettingsMarshal<bool>::SettingsMarshal(ref<Settings> st, const std::wstring& key) {
			_value = st->GetFlag(key);
			_settings = st;
			_key = key;
		}

		template<> SettingsMarshal<bool>::SettingsMarshal(ref<Settings> st, const std::wstring& key, const bool& defaultVal) {
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