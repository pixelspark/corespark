#ifndef _TJSHARED_SETTINGS_H
#define _TJSHARED_SETTINGS_H

#include "internal/tjpch.h"
#include "tjthread.h"
#include <map>

namespace tj {
	namespace shared {
		class Property;

		class EXPORTED Settings: public virtual Object {
			public:
				virtual ~Settings();
				virtual bool GetFlag(const String& key) const = 0;
				virtual bool GetFlag(const String& key, bool defaultValue) const = 0;
				virtual String GetValue(const String& key) const = 0;
				virtual String GetValue(const String& key, const String& defaultValue) const = 0;
				virtual void SetValue(const String& key, const String& value) = 0;
				virtual void SetFlag(const String& key, bool t) = 0;
				virtual ref<Settings> GetNamespace(const String& ns) = 0;
		};

		class EXPORTED SettingsStorage: public Settings {
			public:
				SettingsStorage();
				virtual ~SettingsStorage();
				virtual void LoadFile(const String& path);
				virtual void SaveFile(const String& path) const;
				virtual void Load(TiXmlElement* you);
				virtual void Save(TiXmlElement* parent) const;

				// Settings implementation
				virtual bool GetFlag(const String& key) const;
				virtual bool GetFlag(const String& key, bool defaultValue) const;
				virtual String GetValue(const String& key) const;
				virtual void SetValue(const String& key, const String& value);
				virtual void SetFlag(const String& key, bool t);
				virtual ref<Settings> GetNamespace(const String& ns);
				virtual String GetValue(const String& key, const String& defaultValue) const;

				// Utility functions
				static String GetSettingsPath(const String& vendor, const String& app, const String& file);

			protected:
				std::map< String, String > _data;
				CriticalSection _lock;
		};
	}
}

#endif