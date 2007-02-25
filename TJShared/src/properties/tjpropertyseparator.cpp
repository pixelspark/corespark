#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

PropertySeparator::PropertySeparator(std::wstring group): Property(group) {
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