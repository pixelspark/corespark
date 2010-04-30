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
using namespace tj::shared::graphics;

TextProperty::TextProperty(const std::wstring& name, ref<Inspectable> holder, std::wstring* value, Pixels height): GenericProperty<std::wstring>(name, holder, value, L"") {
	_height = height;
	SetExpandable(true);
	SetMultiline(true);
}

TextProperty::~TextProperty() {
}

Pixels TextProperty::GetHeight() {
	return IsExpanded()?_height:(Property::GetHeight()*2);
}

/** SuggestionProperty **/
SuggestionProperty::SuggestionProperty(const std::wstring& name, ref<Inspectable> holder, std::wstring* value, bool multiLine): Property(name), _holder(holder), _multiLine(multiLine), _value(value) {
	if(value==0) Throw(L"Property value pointer cannot be null", ExceptionTypeWarning);
	SetExpandable(multiLine);
}

SuggestionProperty::~SuggestionProperty() {
	_value = 0;
}

void SuggestionProperty::Notify(ref<Object> source, const EditWnd::EditingNotification& ev) {
	ref<Inspectable> is = _holder;
	if(is && _value!=0L && _wnd) {
		EditWnd::EditingType type = ev.GetType();
		if(type==EditWnd::EditingTextChanged) {
			*_value = _wnd->GetText();
			is->OnPropertyChanged(reinterpret_cast<void*>(_value));
		}
		else if(type==EditWnd::EditingStarted) {
			_oldValue = *_value;
		}
		else if(type==EditWnd::EditingEnded) {
			if((*_value) != _oldValue) {
				UndoBlock::AddChange(GC::Hold(new PropertyChange<String>(is, GetName(), _value, _oldValue, *_value)));
			}
		}
	}
}

ref<Wnd> SuggestionProperty::GetWindow() {
	if(!_wnd) {
		ref<SuggestionEditWnd> ew = GC::Hold(new SuggestionEditWnd(_multiLine));
		ew->EventEditing.AddListener(this);
		ew->SetBorder(true);
		_wnd = ew;
		Update();
	}
	return _wnd;
}

void SuggestionProperty::Update() {
	if(_wnd) {
		_wnd->SetText(*_value);
		_wnd->Repaint();
	}
}

Pixels SuggestionProperty::GetHeight() {
	return IsExpanded() ? (Property::GetHeight()*5) : (Property::GetHeight()*2);
}

strong<Menu> SuggestionProperty::GetSuggestionMenu() {
	GetWindow();
	return _wnd->GetSuggestionMenu();
}

void SuggestionProperty::SetSuggestionMode(SuggestionEditWnd::SuggestionMode sm) {
	GetWindow();
	_wnd->SetSuggestionMode(sm);
}