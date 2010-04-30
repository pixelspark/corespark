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
 
 #ifndef _TJSHARED_SETTINGS_H
#define _TJSHARED_SETTINGS_H

#include "tjsharedinternal.h"
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
				static String GetSystemSettingsPath(const String& vendor, const String& app, const String& file);

			protected:
				std::map< String, String > _data;
				CriticalSection _lock;
		};
	}
}

#endif