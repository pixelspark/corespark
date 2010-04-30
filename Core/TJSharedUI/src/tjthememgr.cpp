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
 
 #include "../include/tjsharedui.h"
using namespace tj::shared;

std::vector< strong<Theme> > ThemeManager::_themes;
ref<Theme> ThemeManager::_theme;
bool ThemeManager::_friendlyTime = false;
bool ThemeManager::_themesLoaded = false;
ref<SettingsStorage> ThemeManager::_layoutSettings;
Listenable<ThemeManager::ThemeChangeNotification> ThemeManager::EventThemeChanged;

ThemeManager::ThemeChangeNotification::ThemeChangeNotification(ref<Theme> n): _newTheme(n) {
}

ref<Theme> ThemeManager::ThemeChangeNotification::GetNewTheme() {
	return _newTheme;
}

strong<Theme> ThemeManager::GetTheme() {
	if(!_themesLoaded) {
		_themesLoaded = true;
		LoadBuiltinThemes();
	}
	return _theme;
}

void ThemeManager::AddTheme(strong<Theme> th) {
	_themes.push_back(th);
	if(!_theme) {
		_theme = th; // select this theme if no theme is selected yet
	}
}

strong<SettingsStorage> ThemeManager::GetLayoutSettings() {
	if(!_layoutSettings) {
		_layoutSettings = GC::Hold(new SettingsStorage());
	}
	return _layoutSettings;
}

void ThemeManager::RemoveTheme(ref<Theme> thm) {
	std::vector< strong<Theme> >::iterator it = _themes.begin();
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
	std::vector< strong<Theme> >::iterator it = _themes.begin();
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
	std::vector< strong<Theme> >::iterator it = _themes.begin();
	while(it!=_themes.end()) {
		ref<Theme> theme = *it;
		lst.push_back(theme);
		++it;
	}
}

void ThemeManager::SelectTheme(strong<Theme> th) {
	_theme = th;
	EventThemeChanged.Fire(null, ThemeChangeNotification(th));
}

void ThemeManager::SelectTheme(int n) {
	if(n<0 || n>=int(_themes.size())) {
		return;
	}

	_theme = _themes.at(n);
	EventThemeChanged.Fire(null, ThemeChangeNotification(_theme));
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