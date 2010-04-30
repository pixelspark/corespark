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
 
 #ifndef _TJTHEMEMGR_H
#define _TJTHEMEMGR_H

namespace tj {
	namespace shared {
		class EXPORTED ThemeManager: public virtual Object {
			public:
				static strong<Theme> GetTheme();
				static strong<SettingsStorage> GetLayoutSettings();
				static void AddTheme(strong<Theme> thm);
				static void RemoveTheme(ref<Theme> thm);
				static void SelectTheme(strong<Theme> thm);
				static void SelectTheme(int n);
				static void ListThemes(std::vector< ref<Theme> >& lst);
				static int GetThemeId();

				struct ThemeChangeNotification {
					public:
						ThemeChangeNotification(ref<Theme> newTheme);
						ref<Theme> GetNewTheme();

					private:
						ref<Theme> _newTheme;
				};
				static Listenable<ThemeChangeNotification> EventThemeChanged;

				// friendly time
				static bool IsFriendlyTime();
				static void SetFriendlyTime(bool t);

			protected:
				static void LoadBuiltinThemes();
				static std::vector< strong<Theme> > _themes;
				static ref<Theme> _theme;
				static ref<SettingsStorage> _layoutSettings;
				static bool _friendlyTime;
				static bool _themesLoaded;
		};

		class EXPORTED ThemeSubscription {
			public:
				ThemeSubscription(ref<Theme> thm);
				virtual ~ThemeSubscription();

			protected:
				ref<Theme> _theme;
		};
	}
}

#endif