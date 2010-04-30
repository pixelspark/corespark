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
 
 #include "../../include/tjsharedui.h"
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

ref<PropertySet> Inspectable::GetProperties() {
	return null;
}

void Inspectable::OnPropertyChanged(void* member) {
}

class PropertyCheckBoxWnd: public CheckboxWnd {
	public:
		PropertyCheckBoxWnd(ref<Inspectable> holder, bool* value): _holder(holder), _value(value) {
			assert(value!=0);
		}

		virtual ~PropertyCheckBoxWnd() {
		}

		virtual void SetChecked(bool t) {
			ref<Inspectable> holder = _holder;
			if(holder && _value!=0L && (*_value)!=t) {
				CheckboxWnd::SetChecked(t);
				UndoBlock::AddAndDoChange(GC::Hold(new PropertyChange<bool>(holder, L"", _value, *_value, t)));
			}
		}

		virtual void Update() {
			CheckboxWnd::SetChecked(*_value);
			CheckboxWnd::Update();
		}

	protected:
		bool* _value;
		weak<Inspectable> _holder;
};

/** GenericProperty specializations **/
ref<Wnd> GenericProperty<bool>::GetWindow() {
	if(!_wnd) {
		_wnd = GC::Hold(new PropertyCheckBoxWnd(_holder, _value));
	}
	return _wnd;
}

void GenericProperty<bool>::Update() {
	ref<CheckboxWnd> cb = _wnd;
	if(cb) {
		ref<Inspectable> is = _holder;
		if(is) {
			cb->SetChecked(*_value);
			cb->Update();
		}
	}
}

void GenericProperty<Time>::Update() {
	if(_wnd) {
		_wnd->SetText(_value->Format());
	}
}