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