#include "../../include/properties/tjproperties.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

TextProperty::TextProperty(const std::wstring& name, std::wstring* value, Pixels height): GenericProperty<std::wstring>(name, value, 0, L"") {
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
SuggestionProperty::SuggestionProperty(const std::wstring& name, std::wstring* value, bool multiLine): Property(name), _multiLine(multiLine), _value(value) {
	if(value==0) Throw(L"Property value pointer cannot be null", ExceptionTypeWarning);
	SetExpandable(multiLine);
}

SuggestionProperty::~SuggestionProperty() {
	_value = 0;
}

void SuggestionProperty::Notify(ref<Object> source, const EditWnd::NotificationTextChanged& ev) {
	if(_wnd && _value!=0) {
		*_value = _wnd->GetText();
	}
}

ref<Wnd> SuggestionProperty::GetWindow() {
	if(!_wnd) {
		ref<SuggestionEditWnd> ew = GC::Hold(new SuggestionEditWnd(_multiLine));
		ew->EventTextChanged.AddListener(this);
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