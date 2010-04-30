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

#ifdef TJ_OS_WIN
	// TODO: is this used anywhere?
	#include <windowsx.h>
#endif

using namespace tj::shared::graphics;
using namespace tj::shared;

const float GridWnd::KMinimumColumnWidth = 0.075f;

/** GridWnd **/
GridWnd::GridWnd(): _allowResize(true), _draggingCol(-1), _dragStartX(0), _showHeader(true) {
}

GridWnd::~GridWnd() {
}

void GridWnd::SetAllowColumnResizing(bool r) {
	_allowResize = r;
}

bool GridWnd::IsColumnResizingAllowed() const {
	return _allowResize && _cols.size() > 1;
}

void GridWnd::SetShowHeader(bool t) {
	_showHeader = t;
	Update();
}

void GridWnd::OnSettingsChanged() {
	ref<Settings> st = GetSettings();

	bool changed = false;
	std::map<int,Column>::iterator it = _cols.begin();
	while(it!=_cols.end()) {
		float newSize = StringTo<float>(st->GetValue(L"col-"+Stringify(it->first)+L".width", Stringify(it->second._width)), it->second._width);
		if(it->second._width!=newSize) {
			it->second._width = newSize;
			changed = true;
		}

		it->second._visible = st->GetFlag(L"col-"+Stringify(it->first)+L".visible", it->second._visible);
		++it;
	}

	if(changed) {
		OnColumnSizeChanged();
		Repaint();
	}
}

void GridWnd::CheckAndUpdateColumnWidths() {
	float totalWidth = 0.0f;
	std::map<int,Column>::const_iterator it = _cols.begin();
	while(it!=_cols.end()) {
		const Column& ci = it->second;
		if(ci._visible) {
			totalWidth += ci._width;
		}
		++it;
	}

	if(totalWidth > 1.0f) {
		std::map<int,Column>::iterator cit = _cols.begin();
		while(cit!=_cols.end()) {
			Column& ci = cit->second;
			if(ci._visible) {
				ci._width = ci._width / totalWidth;
			}
			++cit;
		}
	}
}

Area GridWnd::GetClientArea() const {
	Area rc = ChildWnd::GetClientArea();
	rc.Narrow(0, GetHeaderHeight(), 0, 0);
	return rc;
}

void GridWnd::SetColumnVisible(int id, bool v) {
	if(_cols.size()>1) {
		std::map<int, Column>::iterator it = _cols.find(id);
		if(it!=_cols.end()) {
			it->second._visible = v;

			ref<Settings> st = GetSettings();
			if(st) {
				st->SetFlag(L"col-"+Stringify(id)+L".visible", v);
			}
		}
		else {
			Throw(L"Column could not be found", ExceptionTypeError);
		}
		CheckAndUpdateColumnWidths();
		OnColumnSizeChanged();
		Repaint();
	}
}

bool GridWnd::IsColumnVisible(int id) const {
	if(_cols.size()<2) {
		const std::pair<int,Column>& first = *(_cols.begin());
		return first.first == id;
	}

	std::map<int, Column>::const_iterator it = _cols.find(id);
	if(it!=_cols.end()) {
		return it->second._visible;
	}
	Throw(L"Column could not be found", ExceptionTypeError);
}

Pixels GridWnd::GetHeaderHeight() const {
	if(!_showHeader) return 0;
	strong<Theme> theme = ThemeManager::GetTheme();
	return theme->GetMeasureInPixels(Theme::MeasureListHeaderHeight);
}

void GridWnd::DoContextMenu(Pixels x, Pixels y) {
	if(y < GetHeaderHeight()) {
		ContextMenu cm;
		std::map<int, Column>::const_iterator it = _cols.begin();
		while(it!=_cols.end()) {
			const std::pair<int, Column>& data = *it;
			if(data.second._title.length()>0) { // columns without a title usually show icons and should not be hidden
				cm.AddItem(data.second._title, data.first+1, false, data.second._visible);
			}
			++it;
		}

		int r = cm.DoContextMenu(ref<Wnd>(this), x, y);
		if(r>0) {
			SetColumnVisible(r-1, !IsColumnVisible(r-1));
		}
		Update();
	}
}

void GridWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(y < GetHeaderHeight() && y > 0) {
		if(ev==MouseEventLDown) {
			if(IsColumnResizingAllowed()) {
				_dragStartX = x;
				Area r = ChildWnd::GetClientArea();
				Mouse::Instance()->SetCursorType(CursorSizeEastWest);
				int cx = 0;
				std::map<int, Column>::iterator it = _cols.begin();
				_draggingCol = -1;
				while(it!=_cols.end()) {
					Column& col = it->second;
					if(col._visible) {
					cx += int(r.GetWidth()*col._width);
						if(x<cx) {
							_draggingCol = it->first;
							break;
						}
					}
					++it;
				}

				if(_draggingCol>=0) {
					_capture.StartCapturing(Mouse::Instance(), ref<Wnd>(this));
				}
			}
		}
		else if(ev==MouseEventRDown) {
			DoContextMenu(x,y);
		}
		else if(ev==MouseEventMove) {
			if(IsColumnResizingAllowed()) {
				Pixels ch = GetHeaderHeight();

				if(y<ch) {
					if(_draggingCol>=0) {
						int dx = x - _dragStartX;
						_dragStartX = x;
						Area r = ChildWnd::GetClientArea();
						if(r.GetWidth()!=0) {
							float colWidthChange = float(dx)/float(r.GetWidth());
							_cols[_draggingCol]._width += colWidthChange; ///
							if(_cols[_draggingCol]._width<=KMinimumColumnWidth) {
								_cols[_draggingCol]._width = KMinimumColumnWidth;
							}
							OnColumnSizeChanged();
						}
					}
					Repaint();
				}
			}
		}
		else if(ev==MouseEventLUp) {
			// Update the width of the column in settings
			ref<Settings> st = GetSettings();
			if(st) {
				st->SetValue(L"col-"+Stringify(_draggingCol)+L".width", Stringify(GetColumnWidth(_draggingCol)));
			}

			// Cancel the drag stuff
			_draggingCol = -1;
			_capture.StopCapturing();
			Mouse::Instance()->SetCursorType(CursorDefault);	
		}
	}
}

void GridWnd::SetColumnWidth(int id, float w) {
	std::map<int, Column>::iterator it = _cols.find(id);
	if(it!=_cols.end()) {
		it->second._width = w;

		ref<Settings> st = GetSettings();
		if(st) {
			st->SetValue(L"col-"+Stringify(id)+L".width", Stringify(w));
		}

		OnColumnSizeChanged();
		Repaint();
	}
}

void GridWnd::OnContextMenu(Pixels x, Pixels y) {
	DoContextMenu(x,y);
}

void GridWnd::OnColumnSizeChanged() {
}

float GridWnd::GetColumnX(int id) {
	// If there is only one column, it is always the leftmost
	if(_cols.size()<2) {
		return 0.0f;
	}

	float cx = 0.0f;
	std::map<int,Column>::iterator it = _cols.begin();
	while(it!=_cols.end()) {
		Column& col = it->second;
		if(col._visible) {
			if(it->first==id) {
				return cx;
			}
			cx += col._width;
		}
		++it;
	}
	return 0.0f;
}

float GridWnd::GetColumnWidth(int id) {
	// If there is only one column, it is 100% wide and it cannot be resized
	if(_cols.size()<2) {
		return 1.0f;
	}

	if(_cols.find(id)!=_cols.end()) {
		return _cols[id]._width;
	}
	return 0.0f;
}

void GridWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	if(_showHeader) {
		// draw columns
		SolidBrush borderBrush(theme->GetColor(Theme::ColorActiveStart));
		Pen border(&borderBrush, 1.0f);
		Area area = ChildWnd::GetClientArea();
		Pixels headHeight = GetHeaderHeight();
		Area headRc = area;
		headRc.SetHeight(headHeight);

		LinearGradientBrush lbr(PointF(0.0f, float(area.GetTop())), PointF(0.0f, float(area.GetTop()+headHeight)), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
		g.FillRectangle(&lbr, headRc);

		SolidBrush disabledBr(theme->GetColor(Theme::ColorDisabledOverlay));
		g.FillRectangle(&disabledBr, headRc);
		g.DrawLine(&border, 0, headHeight, area.GetWidth(), headHeight);

		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);
		sf.SetLineAlignment(StringAlignmentCenter);
		sf.SetTrimming(StringTrimmingEllipsisCharacter);

		LinearGradientBrush colBr(PointF(0.0f, 4.0f), PointF(0.0f, float(headHeight-8)), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
		SolidBrush separatorBrush(theme->GetColor(Theme::ColorActiveEnd));
		Pen separator(&separatorBrush);

		float x = float(area.GetLeft());
		std::map<int,Column>::iterator it = _cols.begin();
		while(it!=_cols.end()) {
			Column& col = it->second;
			if(col._visible) {
				float w = col._width*area.GetWidth();
				if(_cols.size()<2) {
					w = float(area.GetWidth());
				}
				g.DrawString(col._title.c_str(), (int)col._title.length(), theme->GetGUIFontBold(), RectF(x+3.0f, (float)area.GetTop()+4.0f, w, headHeight-8.0f), &sf, &colBr);
				g.DrawLine(&separator, x+w, area.GetTop()+2.0f, x+w, area.GetTop()+headHeight-4.0f);
				x += w;
			}
			++it;
		}
	}
}

void GridWnd::AddColumn(std::wstring name, int id, float w, bool visible) {
	_cols[id] = Column(name);
	_cols[id]._visible = visible;
	if(w>0.0f) {
		_cols[id]._width = w;
	}
}

// Column
GridWnd::Column::Column(std::wstring title) {
	_title = title;
	_width = 0.2f;
	_visible = true;
}

/** ListWnd **/
ListWnd::ListWnd(): _selected(-1) {
	SetVerticallyScrollable(true);
	SetStyle(WS_TABSTOP);
}

ListWnd::~ListWnd() {
}

void ListWnd::OnSettingsChanged() {
	GridWnd::OnSettingsChanged();
}

void ListWnd::DrawTag(Graphics& g, Area rc, const std::wstring& text, strong<Theme> theme) {
	LinearGradientBrush tag(PointF(0.0f, (float)rc.GetTop()-1.0f), PointF(0.0f, (float)rc.GetBottom()+1.0f), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
	g.FillRectangle(&tag, rc);
	Pen border(theme->GetColor(Theme::ColorActiveStart), 1.0f);
	g.DrawRectangle(&border, rc);

	StringFormat sf;
	sf.SetAlignment(StringAlignmentCenter);
	sf.SetLineAlignment(StringAlignmentCenter);
	SolidBrush tbr(theme->GetColor(Theme::ColorBackground));
	g.DrawString(text.c_str(), (int)text.length(), theme->GetGUIFontSmall(), rc, &sf, &tbr);
}

void ListWnd::DrawCellText(graphics::Graphics& g, graphics::StringFormat* sf, graphics::SolidBrush* br, graphics::Font* font, int col, Area row, const std::wstring& str) {
	Column& column = _cols[col];
	if(column._visible) {
		sf->SetFormatFlags(sf->GetFormatFlags()|StringFormatFlagsLineLimit);
		row.SetX((Pixels)(GetColumnX(col)*row.GetWidth()));
		row.SetWidth((Pixels)(GetColumnWidth(col)*row.GetWidth()));
		row.Narrow(0,1,0,1);
		g.DrawString(str.c_str(), (int)str.length(), font, row, sf, br);
	}
}

void ListWnd::DrawCellIcon(graphics::Graphics& g, int col, Area row, Icon& icon) {
	Column& column = _cols[col];
	if(column._visible) {
		graphics::Image* bitmap = icon.GetBitmap();
		if(bitmap==0) return;

		Area cell(Pixels(GetColumnX(col)*row.GetWidth()), row.GetTop(), Pixels(GetColumnWidth(col)*row.GetWidth()), row.GetHeight());
		cell.SetX(cell.GetX() + cell.GetWidth()/2 - (bitmap->GetWidth()/2));
		cell.SetWidth(bitmap->GetWidth());

		g.DrawImage(bitmap, cell);
	}
}

void ListWnd::DrawCellDownArrow(graphics::Graphics& g, int col, const Area& row) {
	Column& column = _cols[col];
	if(column._visible) {
		Icon ic(Icons::GetIconPath(Icons::IconDownArrow));

		Area cell(Pixels((GetColumnX(col)+GetColumnWidth(col))*row.GetWidth()-16), row.GetTop(), 16, 16);
		g.DrawImage(ic, cell);
	}
}

void ListWnd::SetEmptyText(const std::wstring& t) {
	_emptyText = t;
	Repaint();
}

std::wstring ListWnd::GetEmptyText() const {
	return _emptyText;
}

void ListWnd::Paint(graphics::Graphics &g, strong<Theme> theme) {
	Area area = GetClientArea();
	SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
	g.Clear(theme->GetColor(Theme::ColorBackground));

	SolidBrush borderBrush(theme->GetColor(Theme::ColorActiveStart));
	SolidBrush lineBrush(theme->GetColor(Theme::ColorActiveEnd));
	Pen border(&borderBrush, 1.0f);
	Pen lineBorder(&lineBrush, 1.0f);
	bool hasFocus = HasFocus(true);

	int n = GetItemCount();
	if(n>0) {
		// draw items, if they fit
		int h = -int(GetVerticalPos()) + area.GetTop();
		Pixels itemHeight = GetItemHeight();
		SolidBrush colorEven(theme->GetColor(Theme::ColorTimeBackground));
		LinearGradientBrush colorSelected(PointF(0.0f, float(h)), PointF(0.0f, float(h+itemHeight)), theme->GetColor(Theme::ColorTimeSelectionStart), theme->GetColor(Theme::ColorTimeSelectionEnd));

		for(int a=0;a<n;a++) {
			if(h>area.GetBottom()) {
				break;
			}
			else if(h>=-2*itemHeight) {
				Area rowArea(area.GetLeft(), h+1, area.GetWidth(), itemHeight);
				// draw a background if odd
				if(a==_selected) {
					g.FillRectangle(&colorSelected, rowArea);
					if(!hasFocus) g.FillRectangle(&disabled, rowArea);
				}
				else if(a%2==1) {
					g.FillRectangle(&colorEven, rowArea);
				}
				
				// draw item
				PaintItem(a, g, rowArea, *this);
				g.DrawLine(&lineBorder, 0, h+itemHeight, area.GetRight(), h+itemHeight);
			}
			h += itemHeight;
		}
	}
	else {
		// draw the 'empty text'
		StringFormat sf;
		sf.SetAlignment(StringAlignmentCenter);
		SolidBrush descBrush(theme->GetColor(Theme::ColorHint));
		Area emptyTextArea = area;
		emptyTextArea.Narrow(0,int(GetHeaderHeight()*0.5f),0,0);
		g.DrawString(_emptyText.c_str(), (int)_emptyText.length(), theme->GetGUIFont(), emptyTextArea, &sf, &descBrush);
	}

	GridWnd::Paint(g, theme);

	Area rc = GetClientArea();
	theme->DrawInsetRectangleLight(g, rc);
}

void ListWnd::SetSelectedRow(int r) {
	_selected = r;
	Repaint();
}

int ListWnd::GetSelectedRow() const {
	return _selected;
}

void ListWnd::OnSize(const Area& ns) {
	int h = (GetItemCount()+1)*GetItemHeight();
	SetVerticalScrollInfo(Range<int>(0, h), ns.GetHeight());

	// update size/scrolls
	Layout();
	Repaint();
}

void ListWnd::OnMouseWheelMove(WheelDirection wd) {
	Area a = GetClientArea();
	
	if(a.GetHeight()<(1+GetItemCount())*GetItemHeight()) {
		if(wd==WheelDirectionDown) {
			SetVerticalPos(min(int(GetVerticalPos())+10, (1+GetItemCount())*GetItemHeight()-a.GetHeight()));
		}
		else {
			SetVerticalPos(max(int(GetVerticalPos())-10, 0));
		}
	}

	OnScroll(ScrollDirectionVertical);
}

void ListWnd::OnScroll(ScrollDirection dir) {
	Layout();
	Repaint();
}

void ListWnd::OnColumnSizeChanged() {
}

void ListWnd::OnKey(Key k, wchar_t t, bool down, bool isAccelerator) {
	if(k==KeyUp && down && _selected>0) {
		SetSelectedRow(_selected-1);
	}
	else if(k==KeyDown && down && _selected<(GetItemCount()-1)) {
		SetSelectedRow(_selected+1);
	}
}

void ListWnd::OnFocus(bool f) {
	Repaint();
}

void ListWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	Area rc = GetClientArea();
	if(y < rc.GetTop()) {
		GridWnd::OnMouse(ev, x, y);
	}
	else {
		if(ev==MouseEventLDown||ev==MouseEventLDouble||ev==MouseEventRDown) {
			Focus();
			Pixels ch = GetHeaderHeight();
			Area area = GetClientArea();
			int idx = GetRowIDByHeight(y);

			// find column
			std::map<int,Column>::iterator it = _cols.begin();
			Pixels cx = 0;
			while(it!=_cols.end()) {
				Column& col = it->second;
				if(col._visible) {
					Pixels px = cx;
					cx += Pixels(col._width*area.GetWidth());
					if(x<cx) {
						if(ev==MouseEventRDown) {
							OnRightClickItem(idx, it->first);
						}
						else if(ev==MouseEventLDouble) {
							OnDoubleClickItem(idx, it->first);
						}
						else {
							Area row = GetRowArea(idx);
							OnClickItem(idx, it->first, x-px, y-row.GetTop());
						}
						break;
					}
				}
				++it;
			}
		}	
	}
}

int ListWnd::GetRowIDByHeight(int y) {
	int ih = GetItemHeight();
	if(ih==0) ih = 1;
	Area rc = GetClientArea();
	return (y - rc.GetTop() + int(GetVerticalPos()))/ih;
}

void ListWnd::OnClickItem(int id, int col, Pixels x, Pixels y) {
}

void ListWnd::OnRightClickItem(int id, int col) {
}

void ListWnd::OnDoubleClickItem(int id, int col) {
}

int ListWnd::GetItemHeight() {
	strong<Theme> theme = ThemeManager::GetTheme();
	return theme->GetMeasureInPixels(Theme::MeasureListItemHeight);
}

Area ListWnd::GetRowArea(int rid) {
	Area client = GetClientArea();
	int ih = GetItemHeight();
	int h = -int(GetVerticalPos()) + (rid*ih);
	
	if(h>=0 && h<client.GetBottom()) {
		return Area(client.GetLeft(), client.GetTop()+h, client.GetWidth(), ih);
	}
	return Area(0,0,0,0);
}

// EditableListWnd
EditableListWnd::EditableListWnd(): _rowEditing(-1) {
}

EditableListWnd::~EditableListWnd() {
}

void EditableListWnd::OnColumnSizeChanged() {
	Layout();
	ListWnd::OnColumnSizeChanged();
}

bool EditableListWnd::IsEditing() const {
	return _rowEditing >= 0;
}

int EditableListWnd::GetEditingRow() const {
	return IsEditing() ? _rowEditing: -1;
}

void EditableListWnd::OnEditingStarted(int r) {
}

void EditableListWnd::OnEditingDone(int r) {
}

void EditableListWnd::SetSelectedRow(int r) {
	if(IsEditing() && r!=_rowEditing) {
		EndEditing();
	}
	ListWnd::SetSelectedRow(r);
}

void EditableListWnd::Layout() {
	if(IsEditing()) {
		strong<Theme> theme = ThemeManager::GetTheme();
		Area row = GetRowArea(_rowEditing);

		std::map< int, ref<Property> >::iterator it = _editorProperties.begin();
		while(it!=_editorProperties.end()) {
			ref<Property> p = it->second;
			if(p) {
				ref<Wnd> w = p->GetWindow();
				if(w) {
					Area realRow = row;
					realRow.Widen(0,0,0,1);
					realRow.SetX(Pixels(GetColumnX(it->first)*row.GetWidth()));
					realRow.SetWidth(Pixels(GetColumnWidth(it->first)*row.GetWidth()));

					if(IsColumnVisible(it->first)) {
						w->Move(realRow.GetX(), realRow.GetY(), realRow.GetWidth(), realRow.GetHeight());
						w->Show(true);
					}
					else {
						w->Show(false);
					}
				}
			}
			++it;
		}
	}
	ListWnd::Layout();
}

void EditableListWnd::EndEditing() {
	if(IsEditing()) {
		OnEditingDone(_rowEditing);
		_editorProperties.clear();
		_rowEditing = -1;
		Layout();
		Repaint();
	}
}

void EditableListWnd::SetSelectedRowAndEdit(int r) {
	if(IsEditing()) {
		EndEditing();
	}

	if(r>=0 && r<GetItemCount()) {
		std::map< int, ListWnd::Column >::const_iterator it = _cols.begin();
		while(it!=_cols.end()) {
			ref<Property> pr = GetPropertyForItem(r, it->first);
			if(pr) {
				_editorProperties[it->first] = pr;
				Add(pr->GetWindow(),false);
				pr->Update();
			}
			++it;
		}

		_rowEditing = r;
		Layout();
		SetSelectedRow(r);
		OnEditingStarted(r);
		Repaint();
	}
}

ColumnInfo::~ColumnInfo() {
}