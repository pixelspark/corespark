#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

PropertySeparator::PropertySeparator(std::wstring group, bool collapsed): Property(group), _collapsed(collapsed) {
}

PropertySeparator::~PropertySeparator() {
}

std::wstring PropertySeparator::GetValue() {
	return L"";
}

HWND PropertySeparator::GetWindow() {
	return 0;
}

HWND PropertySeparator::Create(HWND parent) {
	return 0;
}

void PropertySeparator::Changed() {
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