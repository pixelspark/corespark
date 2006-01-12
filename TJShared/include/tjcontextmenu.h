#ifndef _TJCONTEXTMENU_H
#define _TJCONTEXTMENU_H

class EXPORTED ContextMenu {
	public:
		ContextMenu();
		virtual ~ContextMenu();
		int DoContextMenu(HWND wnd, int x, int y);
		void AddItem(std::wstring name, int command, bool hilite=false);

	protected:
		HMENU _menu;
		int _index;
};

#endif