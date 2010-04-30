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

std::map<std::wstring, std::wstring> UpdaterSettings::_settings;

std::wstring UpdaterSettings::GetSetting(const std::wstring& key, const std::wstring& defaultValue) {
	std::map<std::wstring, std::wstring>::const_iterator it = _settings.find(key);
	if(it!=_settings.end()) {
		return it->second;
	}
	return defaultValue;
}

struct SingletonApplication {
	struct AlreadyRunningException {
	};

	SingletonApplication(const std::wstring& mutexName) {
		_mutex = OpenMutex(MUTEX_ALL_ACCESS, 0, mutexName.c_str());
		if(!_mutex) {
			_mutex = CreateMutex(0, 0, mutexName.c_str());
			if(!_mutex) {
				throw AlreadyRunningException();
			}
		}
		else {
			throw AlreadyRunningException();
		}
	}

	~SingletonApplication() {
		CloseHandle(_mutex);
	}

	HANDLE _mutex;
};

int WINAPI WinMain(HINSTANCE hi, HINSTANCE hp, LPSTR cmd, int nc) {
	try {
		SingletonApplication sia(L"TJ.Updater.v1.0.Single-Instance");

		// For ShellExecute; see http://msdn.microsoft.com/en-us/library/bb762154(VS.85).aspx
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		std::wostringstream argStream;
		argStream << cmd;
		std::wstring args = argStream.str();

		if(args.length()>0) {
			// Trying to get the version of some file
			if(GetFileAttributes(args.c_str())!=INVALID_FILE_ATTRIBUTES) {
				std::wstring hash;
				if(MD5HashFile(args, hash)) {
					MessageBox(0L, hash.c_str(), L"Updater file version", MB_OK|MB_ICONINFORMATION);
				}
			}
		}
		else {
			Updater updater;

			// Find *.updater.xml files in the current directory and process them
			WIN32_FIND_DATA fd;
			HANDLE find = FindFirstFile(L"updater/*.updater.xml", &fd);
			
			if(find!=INVALID_HANDLE_VALUE) {
				do {
					TiXmlDocument document;
					std::string fn = "updater/"+Mbs(fd.cFileName);
					document.LoadFile(fn);

					TiXmlElement* root = document.FirstChildElement();
					if(root!=0) {
						TiXmlElement* settings = root->FirstChildElement("settings");
						if(settings!=0) {
							TiXmlElement* setting = settings->FirstChildElement("setting");
							while(setting!=0) {
								std::wstring key = GetAttribute<std::wstring>(setting, "key", L"");
								std::wstring val = GetAttribute<std::wstring>(setting, "value", L"");
								UpdaterSettings::SetSetting(key,val);
								setting = setting->NextSiblingElement("setting");
							}
						}
						updater.Load(root);
						updater.FindAvailableUpdates();
						updater.DownloadResources();
					}
				}
				while(FindNextFile(find, &fd) != 0);
				FindClose(find);
			}

			updater.Dump();

			if(updater.UpdatesAvailable()) {
				// Notify user
				InitCommonControls();
				UpdaterDialog ud;
				if(ud.DoModal(updater)) {
					if(updater.InstallUpdates()) {
						std::wstring title = UpdaterSettings::GetSetting(L"lang.success.balloon-title", L"Updates installed successfully!");
						std::wstring text = UpdaterSettings::GetSetting(L"lang.success.balloon-text", L"The software now is up-to-date.");
						ud.NotifyUser(title, text);
					}
					else {
						std::wstring title = UpdaterSettings::GetSetting(L"lang.fail.balloon-title", L"Some updates were not installed correctly");
						std::wstring text = UpdaterSettings::GetSetting(L"lang.fail.balloon-text", L"There could be problems with the software intsallation; please try to update again later. If that fails, try to reinstall the software.");
						ud.NotifyUser(title, text);
					}
					Sleep(10000);
				}
			}
		}
	}
	catch(const SingletonApplication::AlreadyRunningException&) {
		UpdaterLog::Write(L"Updater is already running!");
	}

	return 0;
}