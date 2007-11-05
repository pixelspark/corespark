#include "../../include/tjshared.h"
using namespace tj::shared;

Property::Property(const std::wstring& name): _name(name) {
}

Property::~Property() {
}

std::wstring Property::GetName() {
	return _name;
}

int Property::GetHeight() {
	return 17;
}

const std::wstring& Property::GetHint() const {
	return _hint;
}

void Property::SetHint(const std::wstring& h) {
	_hint = h;
}

Inspectable::~Inspectable() {
}

HWND GenericProperty<Time>::Create(HWND parent) {
	if(GetWindow()!=0) return GetWindow();
	_wnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, TJ_PROPERTY_EDIT_TIME_CLASS_NAME, Stringify(*_value).c_str(), ES_AUTOHSCROLL|WS_CHILD, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);
	if(_wnd==0) {
		Throw(L"Property window creation failed", ExceptionTypeError);
	}
	return GetWindow();
}

void GenericProperty<Time>::Changed() {
	int l = GetWindowTextLength(_wnd);
	wchar_t* cp = new wchar_t[l+2];
	GetWindowText(_wnd, cp, l+1);
	std::wstring value(cp);
	delete[] cp;

	Time timeVal;
	std::wistringstream wis(value);
	wis >> timeVal;

	if(*_value!=timeVal) {
		(*_value) = timeVal;
		if(_alsoSet!=0) {
			(*_alsoSet) = timeVal;
		}
	}
}

void GenericProperty<Time>::Update() {
	std::wstring val = _value->Format();
	SetWindowText(_wnd, val.c_str());
}

HWND GenericProperty<bool>::Create(HWND parent) {
	if(_wnd!=0) return _wnd;
	_wnd = ::CreateWindowEx(0, (const wchar_t*)L"BUTTON", Stringify(*_value).c_str(), WS_TABSTOP|BS_AUTOCHECKBOX|WS_CHILD, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);
	if(_wnd!=0) {
		SetWindowLong(_wnd, GWL_USERDATA, (LONG)(long long)this);
	}
	else {
		Throw(L"Property window creation failed", ExceptionTypeError);
	}
	return _wnd;
}

// for numeric edit stuff, spinner boxes and lots of other candy
HWND GenericProperty<unsigned int>::Create(HWND parent) {
	if(_wnd!=0) return _wnd;
	_wnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, TJ_PROPERTY_EDIT_NUMERIC_CLASS_NAME, Stringify(*_value).c_str(), WS_CHILD|ES_AUTOHSCROLL, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);
	if(_wnd==0) {
		Throw(L"Property window creation failed", ExceptionTypeError);
	}
	return _wnd;
}

HWND GenericProperty<int>::Create(HWND parent) {
	if(_wnd!=0) return _wnd;
	_wnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, TJ_PROPERTY_EDIT_NUMERIC_CLASS_NAME, Stringify(*_value).c_str(), WS_CHILD|ES_AUTOHSCROLL, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);
	if(_wnd==0) {
		Throw(L"Property window creation failed", ExceptionTypeError);
	}
	return _wnd;
}

void GenericProperty<std::wstring>::Changed() {
	int l = GetWindowTextLength(_wnd);
	wchar_t* cp = new wchar_t[l+2];
	GetWindowText(_wnd, cp, l+1);
	
	std::wstring value(cp);
	delete[] cp;

	if(*_value!=value) {
		(*_value) = value;
		if(_alsoSet!=0) {
			(*_alsoSet) = value;
		}
	}
}

void GenericProperty<bool>::Changed() {
	LRESULT st = SendMessage(_wnd, BM_GETCHECK, 0, 0);
	bool value = _default;
	if(st==BST_CHECKED) {
		value = true;
	}
	else if(st==BST_UNCHECKED) {
		value = false;
	}

	(*_value) = value;
	if(_alsoSet!=0) {
		(*_alsoSet) = value;
	}

	if(value) {
		SetWindowText(_wnd, Language::Get(L"yes"));
	}
	else {
		SetWindowText(_wnd, Language::Get(L"no"));
	}
}

void GenericProperty<bool>::Update() {
	SendMessage(_wnd, BM_SETCHECK, (*_value)?BST_CHECKED:BST_UNCHECKED,0L);

	if((*_value)) {
		SetWindowText(_wnd, Language::Get(L"yes"));
	}
	else {
		SetWindowText(_wnd, Language::Get(L"no"));
	}
}