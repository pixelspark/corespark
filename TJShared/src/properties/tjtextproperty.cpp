#include "../../include/tjshared.h"
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
