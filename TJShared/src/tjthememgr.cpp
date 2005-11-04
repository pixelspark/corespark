#include "../include/tjshared.h"

std::vector< ref<Theme> > ThemeManager::_themes;
ref<Theme> ThemeManager::_theme;

ref<Theme> ThemeManager::GetTheme() {
	return _theme;
}

void ThemeManager::AddTheme(ref<Theme> th) {
	_themes.push_back(th);
	if(!_theme) {
		_theme = th; // select this theme if no theme is selected yet
	}
}

void ThemeManager::RemoveTheme(ref<Theme> thm) {
	std::vector< ref<Theme> >::iterator it = _themes.begin();
	while(it!=_themes.end()) {
		ref<Theme> th = *it;
		if(th==thm) {
			_themes.erase(it);
			return;
		}
		it++;
	}
}

void ThemeManager::ListThemes(HWND list) {
	std::vector< ref<Theme> >::iterator it = _themes.begin();
	while(it!=_themes.end()) {
		ref<Theme> theme = *it;
		ListBox_AddString(list, theme->GetName().c_str());
		it++;
	}
}

void ThemeManager::SelectTheme(ref<Theme> th) {
	_theme = th;
}

void ThemeManager::SelectTheme(int n) {
	_theme = _themes.at(n);
}

/* ThemeSubscription */
ThemeSubscription::ThemeSubscription(ref<Theme> thm) {
	_theme = thm;
	ThemeManager::AddTheme(thm);
}

ThemeSubscription::~ThemeSubscription() {
	ThemeManager::RemoveTheme(_theme);
}