#ifndef _TJTHEMEMGR_H
#define _TJTHEMEMGR_H

namespace tj {
	namespace shared {
		class EXPORTED ThemeManager: public virtual Object {
			public:
				static ref<Theme> GetTheme();
				static ref<SettingsStorage> GetLayoutSettings();
				static void AddTheme(ref<Theme> thm);
				static void RemoveTheme(ref<Theme> thm);
				static void SelectTheme(ref<Theme> thm);
				static void SelectTheme(int n);
				static void ListThemes(std::vector< ref<Theme> >& lst);
				static int GetThemeId();

				// friendly time
				static bool IsFriendlyTime();
				static void SetFriendlyTime(bool t);

			protected:
				static std::vector< ref<Theme> > _themes;
				static ref<Theme> _theme;
				static ref<SettingsStorage> _layoutSettings;
				static bool _friendlyTime;
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