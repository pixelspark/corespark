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
 
 #include "../include/tjupdater.h" 
#include <commctrl.h>
using namespace tj::updater;

void UpdaterLog::Write(const std::wstring& msg) {
	int n = GetLastError();
	std::wostringstream wos;
	wos << L'[' << std::setw(8) << n << L']' << L' ' << msg << L'\r' << L'\n';
	OutputDebugString(wos.str().c_str());
}

void UpdaterSettings::SetSetting(const std::wstring& k, const std::wstring& v) {
	_settings[k] = v;
}