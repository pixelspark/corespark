#include "../include/tjshared.h"

ContextMenu::ContextMenu() {
	_index = 0;
	_menu = CreatePopupMenu();
}

ContextMenu::~ContextMenu() {
	DestroyMenu(_menu);
}

int ContextMenu::DoContextMenu(HWND window, int x, int y, bool correct) {
	if(correct) {
		RECT wr;
		GetWindowRect(window, &wr);
		x += wr.left;
		y += wr.top;
	}
	return TrackPopupMenu(_menu, TPM_RETURNCMD|TPM_TOPALIGN|TPM_VERPOSANIMATION, x,y, 0, window, 0);
}

void ContextMenu::AddItem(std::wstring name, int command, bool hilite) {
	MENUITEMINFO mif;
	memset(&mif, 0, sizeof(MENUITEMINFO));

	mif.cbSize = sizeof(MENUITEMINFO);
	mif.fMask = MIIM_ID|MIIM_STRING|MIIM_STATE;

	mif.wID = command;
	mif.fType = MFT_STRING;
	mif.fState = MFS_ENABLED | (hilite?MFS_DEFAULT:0);
	mif.dwTypeData = (LPWSTR)name.c_str();
	mif.cch = (UINT)name.length();
	InsertMenuItem(_menu, 0, FALSE, &mif);
	_index++;
}

void ContextMenu::AddSeparator() {
	MENUITEMINFO mif;
	memset(&mif, 0, sizeof(MENUITEMINFO));

	mif.cbSize = sizeof(MENUITEMINFO);
	mif.fType = MFT_SEPARATOR;
	InsertMenuItem(_menu, 0, FALSE, &mif);
	_index++;
}