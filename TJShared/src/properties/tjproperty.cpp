#include "../../include/tjshared.h"
using namespace tj::shared;

PropertySet::PropertySet() {
}

PropertySet::~PropertySet() {
}

void PropertySet::Add(ref<Property> p) {
	_properties.push_back(p);
}

void PropertySet::MergeAdd(ref<PropertySet> p) {
	if(!p) return;

	std::vector< ref<Property> >::iterator it = p->_properties.begin();
	std::vector< ref<Property> >::iterator end = p->_properties.end();
	while(it!=end) {
		_properties.push_back(*it);
		++it;
	}
}

Property::Property(const std::wstring& name, bool expandable): _name(name) {
	_expandable = expandable;
	_expanded = false;
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

bool Property::IsExpandable() const {
	return _expandable;
}

void Property::SetExpanded(bool t) {
	_expanded = t;
}

bool Property::IsExpanded() {
	return _expanded;
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

// for numeric edit stuff, spinner boxes and lots of other candy
HWND GenericProperty<unsigned int>::Create(HWND parent) {
	if(_wnd!=0) return _wnd;
	_wnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, TJ_PROPERTY_EDIT_NUMERIC_CLASS_NAME, Stringify(*_value).c_str(), WS_TABSTOP|WS_CHILD|ES_AUTOHSCROLL, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);
	if(_wnd==0) {
		Throw(L"Property window creation failed", ExceptionTypeError);
	}
	return _wnd;
}

HWND GenericProperty<int>::Create(HWND parent) {
	if(_wnd!=0) return _wnd;
	_wnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, TJ_PROPERTY_EDIT_NUMERIC_CLASS_NAME, Stringify(*_value).c_str(), WS_TABSTOP|WS_CHILD|ES_AUTOHSCROLL, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);
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

class PropertyCheckBoxWnd: public CheckboxWnd {
	public:
		PropertyCheckBoxWnd(bool* value, bool* also): _value(value), _also(also) {
			assert(value!=0);
		}

		virtual ~PropertyCheckBoxWnd() {
		}

		virtual void SetChecked(bool t) {
			CheckboxWnd::SetChecked(t);
			*_value = t;
			if(_also!=0) {
				*_also = t;
			}
		}

		virtual void Update() {
			CheckboxWnd::SetChecked(*_value);
			CheckboxWnd::Update();
		}

	protected:
		bool* _value;
		bool* _also;
};

HWND GenericProperty<bool>::Create(HWND parent) {
	if(_wnd!=0) return _wnd;
	_wndHold = GC::Hold(new PropertyCheckBoxWnd(_value, _alsoSet));
	_wnd = _wndHold->GetWindow();
	SetParent(_wnd, parent);
	return _wnd;
}

void GenericProperty<bool>::Changed() {
	// Not used, done by PropertyCheckBoxWnd
}

void GenericProperty<bool>::Update() {
	ref<CheckboxWnd> cb = _wndHold;

	if(cb) {
		cb->Update();
	}
}