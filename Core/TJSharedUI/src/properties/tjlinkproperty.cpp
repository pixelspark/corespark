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

LinkProperty::LinkProperty(const std::wstring& name, const std::wstring& text, const std::wstring& icon): Property(name, false), _text(text), _icon(icon) {
}

LinkProperty::~LinkProperty() {
}

ref<Wnd> LinkProperty::GetWindow() {
	if(!_wnd) {
		_wnd = GC::Hold(new LinkPropertyWnd(_text, this, _icon));
	}
	return _wnd;
}

void LinkProperty::Update() {
}

void LinkProperty::OnClicked() {
}

void LinkProperty::SetText(const std::wstring& text) {
	_text = text;
	if(_wnd) {
		_wnd->SetText(text.c_str());
	}
}

/* LinkPropertyWnd */
LinkPropertyWnd::LinkPropertyWnd(const std::wstring& text, ref<LinkProperty> lp, const std::wstring& icon): ButtonWnd(icon, text), _lp(lp) {
}

LinkPropertyWnd::~LinkPropertyWnd() {
}

void LinkPropertyWnd::OnCreated() {
	EventClicked.AddListener(this);
}

void LinkPropertyWnd::Notify(ref<Object> source, const ButtonWnd::NotificationClicked& nc) {
	ref<LinkProperty> lp = _lp;
	if(lp) {
		lp->OnClicked();
		lp->EventClicked.Fire(this,nc);
	}
}