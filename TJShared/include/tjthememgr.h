#ifndef _TJTHEMEMGR_H
#define _TJTHEMEMGR_H

/* disable complaints about dll-interface for std::vector, which should not be reachable
from 'clients' anyway */
#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED ThemeManager {
	public:
		static ref<Theme> GetTheme();
		static void AddTheme(ref<Theme> thm);
		static void RemoveTheme(ref<Theme> thm);
		static void SelectTheme(ref<Theme> thm);
		static void SelectTheme(int n);
		static void ListThemes(HWND list);
		static int GetThemeId();

		// friendly time
		static bool IsFriendlyTime();
		static void SetFriendlyTime(bool t);
	protected:
		static std::vector< ref<Theme> > _themes;
		static ref<Theme> _theme;
		static bool _friendlyTime;
};

class EXPORTED ThemeSubscription {
	public:
		ThemeSubscription(ref<Theme> thm);
		virtual ~ThemeSubscription();

	protected:
		ref<Theme> _theme;
};

#pragma warning(pop)

#endif