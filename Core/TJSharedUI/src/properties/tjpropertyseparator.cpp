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