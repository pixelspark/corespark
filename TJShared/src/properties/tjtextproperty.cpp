#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

TextProperty::TextProperty(std::wstring name, std::wstring* value, Pixels height): Property(name, true) {
	assert(value!=0);
	_value = value;
	_wnd = 0;
	_height = height;
}

TextProperty::~TextProperty() {
	DestroyWindow(_wnd);
}

std::wstring TextProperty::GetValue() {
	return *_value;
}
HWND TextProperty::GetWindow() {
	return _wnd;
}
HWND TextProperty::Create(HWND parent) {
	if(_wnd!=0) return _wnd;
	_wnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, TJ_PROPERTY_EDIT_CLASS_NAME, Stringify(*_value).c_str(), WS_CHILD|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);
	if(_wnd==0) {
		Throw(L"Property window creation failed", ExceptionTypeError);
	}
	return _wnd;
}

// Called when the value in the edit window has changed (EN_CHANGED)
void TextProperty::Changed() {
	if(_value!=0) {
		int l = GetWindowTextLength(_wnd);
		wchar_t* cp = new wchar_t[l+2];
		GetWindowText(_wnd, cp, l+1);
		
		std::wstring cs(cp);
		delete[] cp;
		
		*_value = cs;
	}
}

// Called when a repaint is about to begin and the value in the window needs to be updated
void TextProperty::Update() {
	if(_value!=0) {
		SetWindowText(_wnd, (*_value).c_str());
	}
}

Pixels TextProperty::GetHeight() {
	return IsExpanded()?_height:(Property::GetHeight()*2);
}
