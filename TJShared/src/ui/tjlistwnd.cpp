#include "../../include/ui/tjui.h" 
#include <windowsx.h>
using namespace tj::shared::graphics;
using namespace tj::shared;

const float ListWnd::KMinimumColumnWidth = 0.075f;

ListWnd::ListWnd(): ChildWnd(L"") {
	SetVerticallyScrollable(true);
	_draggingCol = -1;
	_dragStartX = 0;
	_selected = -1;
	_showHeader = true;
	SetStyle(WS_TABSTOP);
}

ListWnd::~ListWnd() {
}

void ListWnd::SetShowHeader(bool t) {
	_showHeader = t;
	Update();
}

void ListWnd::OnSettingsChanged() {
	// Load settings!
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

void ListWnd::SetColumnVisible(int id, bool v) {
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
	OnColumnSizeChanged();
	Repaint();
}

bool ListWnd::IsColumnVisible(int id) const {
	std::map<int, Column>::const_iterator it = _cols.find(id);
	if(it!=_cols.end()) {
		return it->second._visible;
	}
	Throw(L"Column could not be found", ExceptionTypeError);
}

Pixels ListWnd::GetHeaderHeight() const {
	if(!_showHeader) return false;
	ref<Theme> theme = ThemeManager::GetTheme();
	return theme->GetMeasureInPixels(Theme::MeasureListHeaderHeight);
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

void ListWnd::Paint(graphics::Graphics &g, ref<Theme> theme) {
	Area area = GetClientArea();
	Pixels headHeight = GetHeaderHeight();

	SolidBrush back(theme->GetColor(Theme::ColorBackground));
	SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
	g.FillRectangle(&back, area);
	Pen border(theme->GetColor(Theme::ColorActiveStart), 1.0f);
	Pen lineBorder(theme->GetColor(Theme::ColorActiveEnd), 1.0f);
	bool hasFocus = HasFocus(true);

	int n = GetItemCount();
	if(n>0) {
		// draw items, if they fit
		int h = -int(GetVerticalPos());
		Pixels itemHeight = GetItemHeight();
		SolidBrush colorEven(theme->GetColor(Theme::ColorTimeBackground));
		LinearGradientBrush colorSelected(PointF(0.0f, float(h+headHeight)), PointF(0.0f, float(h+headHeight+itemHeight)), theme->GetColor(Theme::ColorTimeSelectionStart), theme->GetColor(Theme::ColorTimeSelectionEnd));

		for(int a=0;a<n;a++) {
			if(h>area.GetHeight()) {
				break;
			}
			else if(h>=-2*itemHeight) {
				Area rowArea(area.GetLeft(), area.GetTop()+headHeight+h+1, area.GetWidth(), itemHeight);
				// draw a background if odd
				if(a==_selected) {
					g.FillRectangle(&colorSelected, rowArea);
					if(!hasFocus) g.FillRectangle(&disabled, rowArea);
				}
				else if(a%2==1) {
					g.FillRectangle(&colorEven, rowArea);
				}
				
				// draw item
				PaintItem(a, g, rowArea);
				g.DrawLine(&lineBorder, 0, h+itemHeight+headHeight, area.GetRight(), h+itemHeight+headHeight);
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
		emptyTextArea.Narrow(0,int(headHeight*1.5f),0,0);
		g.DrawString(_emptyText.c_str(), (int)_emptyText.length(), theme->GetGUIFont(), emptyTextArea, &sf, &descBrush);
	}

	// Draw shadow
	Area shadowArea = area;
	shadowArea.Narrow(0,_showHeader ? headHeight : 0, 0, 0);
	theme->DrawInsetRectangleLight(g, shadowArea);

	// draw columns
	if(_showHeader) {
		theme->DrawToolbarBackground(g, (float)area.GetLeft(), (float)area.GetTop(), (float)area.GetWidth(), (float)headHeight, 0.5f);
		g.DrawLine(&border, 0, headHeight, area.GetWidth(), headHeight);

		StringFormat sf;
		sf.SetAlignment(StringAlignmentNear);
		sf.SetLineAlignment(StringAlignmentCenter);
		sf.SetTrimming(StringTrimmingEllipsisCharacter);
		//SolidBrush colBr(theme->GetColor(Theme::ColorText));
		LinearGradientBrush colBr(PointF(0.0f, 4.0f), PointF(0.0f, float(headHeight-8)), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
		Pen separator(theme->GetColor(Theme::ColorActiveEnd));

		float x = float(area.GetLeft());
		std::map<int,Column>::iterator it = _cols.begin();
		while(it!=_cols.end()) {
			Column& col = it->second;
			if(col._visible) {
				float w = col._width*area.GetWidth();
				g.DrawString(col._title.c_str(), (int)col._title.length(), theme->GetGUIFontBold(), RectF(x+3.0f, (float)area.GetTop()+4.0f, w, headHeight-8.0f), &sf, &colBr);
				g.DrawLine(&separator, x+w, area.GetTop()+2.0f, x+w, area.GetTop()+headHeight-4.0f);
				x += w;
			}
			++it;
		}
	}
}

void ListWnd::SetSelectedRow(int r) {
	_selected = r;
	Repaint();
}

int ListWnd::GetSelectedRow() const {
	return _selected;
}

float ListWnd::GetColumnX(int id) {
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

float ListWnd::GetColumnWidth(int id) {
	if(_cols.find(id)!=_cols.end()) {
		return _cols[id]._width;
	}
	return 0.0f;
}

void ListWnd::OnSize(const Area& ns) {
	int h = (GetItemCount()+1)*GetItemHeight();
	SetVerticalScrollInfo(Range<int>(0, h), ns.GetHeight());

	// update size/scrolls
	Layout();
	Repaint();
}

void ListWnd::OnScroll(ScrollDirection dir) {
	Layout();
	Repaint();
}

void ListWnd::SetColumnWidth(int id, float w) {
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

LRESULT ListWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_MOUSEWHEEL) {
		int delta = GET_WHEEL_DELTA_WPARAM(wp);
		Area a = GetClientArea();
		
		if(a.GetHeight()<(1+GetItemCount())*GetItemHeight()) {
			if(delta<0) {
				
				SetVerticalPos(min(int(GetVerticalPos())+10, (1+GetItemCount())*GetItemHeight()-a.GetHeight()));
			}
			else {
				SetVerticalPos(max(int(GetVerticalPos())-10, 0));
			}
		}
		OnScroll(ScrollDirectionVertical);
	}
	return ChildWnd::Message(msg,wp,lp);
}

void ListWnd::OnFocus(bool f) {
	Repaint();
}

void ListWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		Focus();
	}

	if(ev==MouseEventRDown && y<=GetHeaderHeight()) {
		DoContextMenu(x,y);
	}
	else if(ev==MouseEventLDown||ev==MouseEventLDouble||ev==MouseEventRDown) {
		Pixels ch = GetHeaderHeight();
		if(y<ch && ev!=MouseEventLDouble) {
			_dragStartX = x;
			Area r = GetClientArea();
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
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
				SetCapture(GetWindow());
			}
		}
		else {
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
	else if(ev==MouseEventMove) {
		Pixels ch = GetHeaderHeight();

		if(y<ch) {
			if(_draggingCol>=0) {
				int dx = x - _dragStartX;
				_dragStartX = x;
				Area r = GetClientArea();
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
	else if(ev==MouseEventLUp) {
		// Update the width of the column in settings
		ref<Settings> st = GetSettings();
		if(st) {
			st->SetValue(L"col-"+Stringify(_draggingCol)+L".width", Stringify(GetColumnWidth(_draggingCol)));
		}

		// Cancel the drag stuff
		_draggingCol = -1;
		ReleaseCapture();
		SetCursor(LoadCursor(NULL, IDC_ARROW));		
	}
}

void ListWnd::OnContextMenu(Pixels x, Pixels y) {
	OnMouse(MouseEventRDown, x, y);
	OnMouse(MouseEventRUp, x, y);
}

void ListWnd::DoContextMenu(Pixels x, Pixels y) {
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

int ListWnd::GetRowIDByHeight(int y) {
	int ih = GetItemHeight();
	if(ih==0) ih = 1;
	return (y - GetHeaderHeight() + int(GetVerticalPos()))/ih;
}

void ListWnd::OnClickItem(int id, int col, Pixels x, Pixels y) {
}

void ListWnd::OnRightClickItem(int id, int col) {
}

void ListWnd::OnDoubleClickItem(int id, int col) {
}

int ListWnd::GetItemHeight() {
	ref<Theme> theme = ThemeManager::GetTheme();
	return theme->GetMeasureInPixels(Theme::MeasureListItemHeight);
}

Area ListWnd::GetRowArea(int rid) {
	Area client = GetClientArea();
	int h = -int(GetVerticalPos())+GetHeaderHeight();
	int ih = GetItemHeight();

	h += (rid*ih);
	if(h>0 && h<client.GetBottom()) {
		return Area(client.GetLeft(), client.GetTop()+h, client.GetWidth(), ih);
	}
	return Area(0,0,0,0);
}

void ListWnd::AddColumn(std::wstring name, int id, float w, bool visible) {
	_cols[id] = Column(name);
	_cols[id]._visible = visible;
	if(w>0.0f) {
		_cols[id]._width = w;
	}
}

// Column
ListWnd::Column::Column(std::wstring title) {
	_title = title;
	_width = 0.2f;
	_visible = true;
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
		ref<Theme> theme = ThemeManager::GetTheme();
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