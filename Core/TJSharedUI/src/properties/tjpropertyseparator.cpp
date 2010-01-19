#include "../../include/tjsharedui.h"
#include "../../include/properties/tjproperties.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

PropertySeparator::PropertySeparator(const std::wstring& group, bool collapsed): Property(group), _collapsed(collapsed) {
}

PropertySeparator::~PropertySeparator() {
}

ref<Wnd> PropertySeparator::GetWindow() {
	return 0;
}

void PropertySeparator::Update() {
}

int PropertySeparator::GetHeight() {
	return 17;
}

void PropertySeparator::SetCollapsed(bool c) {
	_collapsed = c;
}

bool PropertySeparator::IsCollapsed() const {
	return _collapsed;
}