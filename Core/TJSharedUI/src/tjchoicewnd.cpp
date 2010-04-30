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
 
 #include "../include/tjsharedui.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

/** Choice **/
Choice::Choice(const std::wstring& title, const std::wstring& text, const std::wstring& image): _title(title), _text(text), _image(image) {
	_w = 72;
	_h = 50;
}

Choice::~Choice() {
}

const std::wstring& Choice::GetTitle() const {
	return _title;
}

const std::wstring& Choice::GetText() const {
	return _text;
}

Pixels Choice::GetWidth() const {
	return _w;
}

Pixels Choice::GetHeight() const {
	return _h;
}

void Choice::SetSize(Pixels w, Pixels h) {
	_w = w;
	_h = h;
}

Icon& Choice::GetImage() {
	return _image;
}

/** ChoiceListWnd **/
ChoiceListWnd::ChoiceListWnd() {
	SetShowHeader(false);
	AddColumn(L"", KColTricks, 1.0f, true);
	_itemHeight = 54;
}

ChoiceListWnd::~ChoiceListWnd() {
}

int ChoiceListWnd::GetItemHeight() {
	return _itemHeight;
}

void ChoiceListWnd::OnClickItem(int id, int col, Pixels x, Pixels y) {
	SetSelectedRow(id);
}

void ChoiceListWnd::SetItemHeight(Pixels h) {
	_itemHeight = h;
	Update();
}

int ChoiceListWnd::GetItemCount() {
	return (int)_choices.size();
}

void ChoiceListWnd::PaintItem(int id, graphics::Graphics &g, tj::shared::Area &row, const ColumnInfo& ci) {
	strong<Theme> theme = ThemeManager::GetTheme();
	ref<Choice> choice = _choices.at(id);
	if(choice) {
		Pixels iw = choice->GetWidth();
		Pixels ih = choice->GetHeight();
		choice->GetImage().Paint(g, Area(row.GetLeft()+2, row.GetTop()+2, iw, ih), true);

		SolidBrush tbr(theme->GetColor(Theme::ColorText));
		const std::wstring& title = choice->GetTitle();
		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);
		Area titleRC = row;
		titleRC.Narrow(iw+2,2,0,2);
		g.DrawString(title.c_str(), (int)title.length(), theme->GetGUIFontBold(), titleRC, &sf, &tbr);
		
		const std::wstring& text = choice->GetText();
		titleRC.Narrow(0,15,0,0);
		g.DrawString(text.c_str(), (int)text.length(), theme->GetGUIFontSmall(), titleRC, &sf, &tbr);
	}
}

void ChoiceListWnd::AddChoice(ref<Choice> tr) {
	if(tr) {
		_choices.push_back(tr);
	}
}

/** ChoiceWnd **/
ChoiceWnd::ChoiceWnd() {
	_choices = GC::Hold(new ChoiceListWnd());
	Add(_choices);
}

ChoiceWnd::~ChoiceWnd() {
}

void ChoiceWnd::SetTitle(const std::wstring& title) {
	_title = title;
}

bool ChoiceWnd::HasHeader() const {
	return _title.length()>0;
}

void ChoiceWnd::Paint(Graphics& g, strong<Theme> theme) {
	Area rc = GetClientArea();

	// Draw header
	if(HasHeader()) {
		Area header = rc;
		header.SetHeight(theme->GetMeasureInPixels(Theme::MeasureToolbarHeight));

		LinearGradientBrush back(PointF(0.0f, 0.0f), PointF(0.0f, (float)header.GetHeight()), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
		g.FillRectangle(&back, header);

		SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
		g.FillRectangle(&disabled, header);

		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);
		SolidBrush tbr(theme->GetColor(Theme::ColorText));
		g.DrawString(_title.c_str(), (int)_title.length(), theme->GetGUIFontBold(), PointF(float(header.GetLeft()+2.0f), header.GetTop()+5.0f), &sf, &tbr);
	}

	rc.Narrow(0,0,1,1);
	SolidBrush border(theme->GetColor(Theme::ColorActiveStart));
	Pen pn(&border, 1.0f);
	g.DrawRectangle(&pn, rc);
}

void ChoiceWnd::Layout() {
	if(_choices) {
		Area rc = GetClientArea();
		strong<Theme> theme = ThemeManager::GetTheme();
		rc.Narrow(1,HasHeader()?theme->GetMeasureInPixels(Theme::MeasureToolbarHeight):0,1,1);
		_choices->Fill(LayoutFill,rc);
	}
}

void ChoiceWnd::OnSize(const Area& ns) {
	Layout();
}

void ChoiceWnd::AddChoice(ref<Choice> tr) {
	_choices->AddChoice(tr);
	SetSize(200, (_choices->GetItemCount()+1)*_choices->GetItemHeight());
}
