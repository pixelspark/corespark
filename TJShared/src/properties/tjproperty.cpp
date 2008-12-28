#include "../../include/properties/tjproperties.h"
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

void Property::SetExpandable(bool t) {
	_expandable = t;
	if(!_expandable) _expanded = false;
}

Pixels Property::GetHeight() {
	strong<Theme> theme = ThemeManager::GetTheme();
	return theme->GetMeasureInPixels(Theme::MeasurePropertyHeight);
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

/** GenericProperty specializations **/
ref<Wnd> GenericProperty<bool>::GetWindow() {
	if(!_wnd) {
		_wnd = GC::Hold(new PropertyCheckBoxWnd(_value, _alsoSet));
	}
	return _wnd;
}

void GenericProperty<bool>::Update() {
	ref<CheckboxWnd> cb = _wnd;

	if(cb) {
		cb->SetChecked(*_value);
		cb->Update();
	}
}

void GenericProperty<Time>::Update() {
	if(_wnd) {
		_wnd->SetText(_value->Format());
	}
}