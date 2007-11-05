#include "../../include/tjshared.h"
#include <windowsx.h>
using namespace tj::shared;

std::vector< ref<Theme> > ThemeManager::_themes;
ref<Theme> ThemeManager::_theme;
bool ThemeManager::_friendlyTime = false;
ref<SettingsStorage> ThemeManager::_layoutSettings;

ref<Theme> ThemeManager::GetTheme() {
	return _theme;
}

void ThemeManager::AddTheme(ref<Theme> th) {
	_themes.push_back(th);
	if(!_theme) {
		_theme = th; // select this theme if no theme is selected yet
	}
}

ref<SettingsStorage> ThemeManager::GetLayoutSettings() {
	if(!_layoutSettings) {
		_layoutSettings = GC::Hold(new SettingsStorage());
	}
	return _layoutSettings;
}

void ThemeManager::RemoveTheme(ref<Theme> thm) {
	std::vector< ref<Theme> >::iterator it = _themes.begin();
	while(it!=_themes.end()) {
		ref<Theme> th = *it;
		if(th==thm) {
			_themes.erase(it);
			return;
		}
		++it;
	}
}

int ThemeManager::GetThemeId() {
	std::vector< ref<Theme> >::iterator it = _themes.begin();
	int idx = 0;
	while(it!=_themes.end()) {
		ref<Theme> th = *it;
		if(th==_theme) {
			return idx;
		}
		idx++;
		++it;
	}

	return -1;
}

void ThemeManager::ListThemes(std::vector< ref<Theme> >& lst) {
	std::vector< ref<Theme> >::iterator it = _themes.begin();
	while(it!=_themes.end()) {
		ref<Theme> theme = *it;
		lst.push_back(theme);
		++it;
	}
}

void ThemeManager::SelectTheme(ref<Theme> th) {
	_theme = th;
}

void ThemeManager::SelectTheme(int n) {
	try {
		_theme = _themes.at(n);
	}
	catch(...) {
		// n out of range
	}
}

bool ThemeManager::IsFriendlyTime() {
	return _friendlyTime;
}

void ThemeManager::SetFriendlyTime(bool t) {
	_friendlyTime = t;
}

/* ThemeSubscription */
ThemeSubscription::ThemeSubscription(ref<Theme> thm) {
	_theme = thm;
	ThemeManager::AddTheme(thm);
}

ThemeSubscription::~ThemeSubscription() {
	ThemeManager::RemoveTheme(_theme);
}