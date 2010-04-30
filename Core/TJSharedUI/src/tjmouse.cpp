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
#include "../res/resource.h"
using namespace tj::shared::graphics;
using namespace tj::shared;

ref<Mouse> Mouse::_mouse;

#ifdef TJ_OS_WIN
class WindowsMouse: public Mouse {
	public:
		WindowsMouse();
		virtual ~WindowsMouse();
		virtual void SetCursorType(const Cursor& c);
		virtual void SetCursorHidden(bool t);
		virtual bool IsCursorHidden() const;
		virtual bool IsButtonDown(Button b);
		virtual Coord GetCursorPosition(ref<Wnd> w);

	protected:
		bool _cursorHidden;
		std::map<Cursor, HCURSOR> _cursors;
};

WindowsMouse::WindowsMouse(): _cursorHidden(false) {
	_cursors[CursorDefault] = LoadCursor(0, IDC_ARROW);
	_cursors[CursorBusy] = LoadCursor(0, IDC_WAIT);
	_cursors[CursorSizeNorthSouth] = LoadCursor(0, IDC_SIZENS);
	_cursors[CursorSizeEastWest] = LoadCursor(0, IDC_SIZEWE);

	// TODO: change the "TJSharedUI" literal to some macro; does VC++ provide something like MODULE_NAME or someting?
	// GetModuleHandle(NULL) doesn't work, since that returns the handle to the main process (i.e. TJShow.exe/TJingle.exe).
	_cursors[CursorHand] = LoadCursor(GetModuleHandle(L"TJSharedUI"), MAKEINTRESOURCE(IDC_GRABHAND));
	_cursors[CursorHandGrab] = LoadCursor(GetModuleHandle(L"TJSharedUI"), MAKEINTRESOURCE(IDC_GRAB));
	_cursors[CursorBeam] = LoadCursor(0, IDC_IBEAM);
	_cursors[CursorSizeAll] = LoadCursor(0, IDC_SIZEALL);
}

WindowsMouse::~WindowsMouse() {
	DestroyCursor(_cursors[CursorHand]);
	DestroyCursor(_cursors[CursorHandGrab]);
}

bool WindowsMouse::IsButtonDown(Button b) {
	int vk = 0;
	switch(b) {
		case Mouse::ButtonMiddle:
			vk = VK_MBUTTON;
			break;

		case Mouse::ButtonRight:
			vk = VK_RBUTTON;
			break;

		default:
		case Mouse::ButtonLeft:
			vk = VK_LBUTTON;
			break;
	}

	return ((GetAsyncKeyState(vk) & 0x8000) == 0x8000);
}

Coord WindowsMouse::GetCursorPosition(ref<Wnd> w) {
	POINT p;
	if(GetCursorPos(&p)) {
		if(w) {
			ScreenToClient(w->GetWindow(), &p);
		}
		strong<Theme> theme = ThemeManager::GetTheme();
		float df = theme->GetDPIScaleFactor();
		return Coord(Pixels(p.x / df), Pixels(p.y / df));
	}
	return Coord(-1,-1);
}

void WindowsMouse::SetCursorHidden(bool t) {
	// ShowCursor uses a 'show cursor counter' internally. This means that we should only call it once;
	// Calling it when the cursor is already hidden will cause the counter to increment to two or even higher,
	// and subsequent attempts to show the cursor again will fail (until after the second time it is tried).
	if(_cursorHidden!=t) {
		_cursorHidden = t;
		ShowCursor(t);
	}
}

bool WindowsMouse::IsCursorHidden() const {
	return _cursorHidden;
}

void WindowsMouse::SetCursorType(const Cursor& c) {
	if(!_cursorHidden) {
		std::map<Cursor, HCURSOR>::iterator it = _cursors.find(c);
		if(it!=_cursors.end()) {
			SetCursor(it->second);
		}
	}
}

MouseCapture::MouseCapture(): _isCapturing(false) {
}

MouseCapture::~MouseCapture() {
	StopCapturing();
}

void MouseCapture::StartCapturing(strong<Mouse> mouse, strong<Wnd> wnd) {
	_isCapturing = true;
	SetCapture(wnd->GetWindow());
}

void MouseCapture::StopCapturing() {
	if(_isCapturing) {
		_isCapturing = false;
		ReleaseCapture();
	}
}

#endif

Mouse::~Mouse() {
}

strong<Mouse> Mouse::Instance() {
	if(!_mouse) {
		#ifdef TJ_OS_WIN
			_mouse = GC::Hold(new WindowsMouse());
		#endif

		#ifdef TJ_OS_MAC
			#error MacMouse not implemented
			_mouse = GC::Hold(new MacMouse());
		#endif
	}
	return _mouse;
}