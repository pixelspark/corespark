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
#include "../resource.h"
#include <windows.h> 
#include <windowsx.h>
#include <shellapi.h>
using namespace tj::updater;

#define WM_UPDATERSTART (WM_USER+1)
#define WM_TRAYMESSAGE	(WM_USER+2)

INT_PTR UpdaterDialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_INITDIALOG) {
		return TRUE;
	}
	else if(msg==WM_COMMAND) {
		if(LOWORD(wp)==IDCANCEL) {
			PostQuitMessage(-1);
		}
		else if(LOWORD(wp)==IDOK) {
			PostMessage(wnd, WM_UPDATERSTART, 0, 0);
		}
	}
	else if(msg==WM_TRAYMESSAGE) {
		if(LOWORD(lp)==NIN_BALLOONUSERCLICK || LOWORD(lp)==NIN_SELECT || LOWORD(lp)==WM_LBUTTONDBLCLK || LOWORD(lp)==WM_RBUTTONDOWN) {
			ShowWindow(wnd, SW_SHOW);
		}
	}
	else if(msg==WM_CLOSE) {
		PostQuitMessage(-1);
	}
	return FALSE;
}

UpdaterDialog::UpdaterDialog() {
	HINSTANCE inst = GetModuleHandle(NULL);
	_wnd = CreateDialog(inst, MAKEINTRESOURCE(IDD_UPDATES), GetDesktopWindow(), (DLGPROC)UpdaterDialogProc);
}

UpdaterDialog::~UpdaterDialog() {
	NOTIFYICONDATA nid;
	nid.cbSize = NOTIFYICONDATA_V3_SIZE;
	nid.uID = 1;
	nid.hWnd = _wnd;
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

bool UpdaterDialog::DoModal(Updater& up) {
	// Update data from updater
	HWND list = GetDlgItem(_wnd, IDC_UPDATELIST);
	ListBox_ResetContent(list);

	std::wstring infoText = UpdaterSettings::GetSetting(L"lang.balloon-text", L"Please click here for more information.");
	std::wstring infoTitle = UpdaterSettings::GetSetting(L"lang.balloon-title", L"Updates are available");
	NotifyUser(infoTitle, infoText);
	
	std::deque<UpdatableComponent>::const_iterator it = up._components.begin();
	while(it!=up._components.end()) {
		const UpdatableComponent& uc = *it;
		if(uc.UpdatesAvailable()) {
			std::deque<Update>::const_iterator uit = uc._updates.begin();
			while(uit!=uc._updates.end()) {
				const Update& upd = *uit;
				ListBox_AddString(list, upd.GetDescription().c_str());
				++uit;
			}
		}
		++it;
	}

	// Showtime
	MSG msg;
	while(GetMessage(&msg,0,0,0)!=0) {
		if(msg.hwnd==_wnd && msg.message==WM_UPDATERSTART) {
			// Start the updater
			ShowWindow(_wnd, SW_HIDE);
			std::wstring infoText = UpdaterSettings::GetSetting(L"lang.install.balloon-text", L"During the process, you cannot use the software; you can safely do other things on your computer, however. Please follow the on-screen instructions for the updates.");
			std::wstring infoTitle = UpdaterSettings::GetSetting(L"lang.install.balloon-title", L"Updates are being installed");
			NotifyUser(infoTitle, infoText);
			return true;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	ShowWindow(_wnd, SW_HIDE);
	return false;
}

void UpdaterDialog::NotifyUser(const std::wstring& infoTitle, const std::wstring& infoText) {
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = NOTIFYICONDATA_V3_SIZE;
	nid.hWnd = _wnd;
	nid.uID = 1;
	Shell_NotifyIcon(NIM_DELETE, &nid);

	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_INFO;
	nid.uCallbackMessage = WM_TRAYMESSAGE;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
	nid.dwState = 0;
	nid.dwStateMask = 0;
	nid.uTimeout = 10000;
	nid.dwInfoFlags = NIIF_INFO;

	wcscpy_s(nid.szTip, 128, L"");
	wcscpy_s(nid.szInfo, 256, infoText.c_str());
	wcscpy_s(nid.szInfoTitle, 64, infoTitle.c_str());
	Shell_NotifyIcon(NIM_ADD, &nid);
}
