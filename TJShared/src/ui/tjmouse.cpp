#include "../../include/ui/tjui.h"
#include "../../include/ui/tjmouse.h"
#include "../../resource.h"
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

	protected:
		bool _cursorHidden;
		std::map<Cursor, HCURSOR> _cursors;
};

WindowsMouse::WindowsMouse(): _cursorHidden(false) {
	_cursors[CursorDefault] = LoadCursor(0, IDC_ARROW);
	_cursors[CursorBusy] = LoadCursor(0, IDC_WAIT);
	_cursors[CursorSizeNorthSouth] = LoadCursor(0, IDC_SIZENS);
	_cursors[CursorSizeEastWest] = LoadCursor(0, IDC_SIZEWE);
	_cursors[CursorHand] = LoadCursor(GetModuleHandle(L"TJShared"), MAKEINTRESOURCE(IDC_GRABHAND));
	_cursors[CursorHandGrab] = LoadCursor(GetModuleHandle(L"TJShared"), MAKEINTRESOURCE(IDC_GRAB));
	_cursors[CursorBeam] = LoadCursor(0, IDC_IBEAM);
	_cursors[CursorSizeAll] = LoadCursor(0, IDC_SIZEALL);
}

WindowsMouse::~WindowsMouse() {
	DestroyCursor(_cursors[CursorHand]);
	DestroyCursor(_cursors[CursorHandGrab]);
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