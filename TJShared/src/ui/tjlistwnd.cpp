#include "../../include/tjshared.h"
#include <windowsx.h>
using namespace Gdiplus;
using namespace tj::shared;

const float ListWnd::KMinimumColumnWidth = 0.075f;

ListWnd::ListWnd(HWND parent): ChildWnd(L"", parent, true, true) {
	SetStyle(WS_CLIPCHILDREN);
	_draggingCol = -1;
	_dragStartX = 0;
	_selected = -1;
}

ListWnd::~ListWnd() {
}

int ListWnd::GetHeaderHeightInPixels() const {
	ref<Theme> theme = ThemeManager::GetTheme();
	return theme->GetMeasureInPixels(Theme::MeasureListHeaderHeight);
}

void ListWnd::DrawCellText(Gdiplus::Graphics& g, Gdiplus::StringFormat* sf, Gdiplus::SolidBrush* br, Gdiplus::Font* font, int col, Area row, const std::wstring& str) {
	row.SetX((Pixels)(GetColumnX(col)*row.GetWidth()));
	row.SetWidth((Pixels)(GetColumnWidth(col)*row.GetWidth()));
	g.DrawString(str.c_str(), (int)str.length(), font, row, sf, br);
}

void ListWnd::DrawCellIcon(Gdiplus::Graphics& g, int col, Area row, Icon& icon) {
	Gdiplus::Image* bitmap = icon.GetBitmap();
	if(bitmap==0) return;

	Area cell(Pixels(GetColumnX(col)*row.GetWidth()), row.GetTop(), Pixels(GetColumnWidth(col)*row.GetWidth()), row.GetHeight());
	cell.SetX(cell.GetX() + cell.GetWidth()/2 - (bitmap->GetWidth()/2));
	cell.SetWidth(bitmap->GetWidth());

	g.DrawImage(bitmap, cell);
}

void ListWnd::SetEmptyText(const std::wstring& t) {
	_emptyText = t;
	Repaint();
}

std::wstring ListWnd::GetEmptyText() const {
	return _emptyText;
}

void ListWnd::Paint(Gdiplus::Graphics &g) {
	Area area = GetClientArea();
	ref<Theme> theme = ThemeManager::GetTheme();
	int headHeight = GetHeaderHeightInPixels();

	SolidBrush back(theme->GetBackgroundColor());
	g.FillRectangle(&back, area);
	Pen border(theme->GetActiveStartColor(), 1.0f);
	Pen lineBorder(theme->GetActiveEndColor(), 1.0f);

	int n = GetItemCount();
	if(n>0) {
		// draw items, if they fit
		int h = -int(GetVerticalPos());
		
		int itemHeight = GetItemHeightInPixels();

		SolidBrush colorEven(theme->GetTimeBackgroundColor());
		LinearGradientBrush colorSelected(PointF(0.0f, float(h+headHeight)), PointF(0.0f, float(h+headHeight+itemHeight)), theme->GetTimeSelectionColorStart(), theme->GetTimeSelectionColorEnd());

		for(int a=0;a<n;a++) {
			if(h>area.GetHeight()) {
				break;
			}
			else if(h>=-2*itemHeight) {
				Area rowArea(area.GetLeft(), area.GetTop()+headHeight+h+1, area.GetWidth(), itemHeight);
				// draw a background if odd
				if(a==_selected) {
					g.FillRectangle(&colorSelected, rowArea);
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
		SolidBrush descBrush(theme->GetTrackDescriptionTextColor());
		Area emptyTextArea = area;
		emptyTextArea.Narrow(0,int(headHeight*1.5f),0,0);
		g.DrawString(_emptyText.c_str(), (int)_emptyText.length(), theme->GetGUIFont(), emptyTextArea, &sf, &descBrush);
	}

	// draw columns
	theme->DrawToolbarBackground(g, (float)area.GetLeft(), (float)area.GetTop(), (float)area.GetWidth(), (float)headHeight);
	g.DrawLine(&border, 0, headHeight, area.GetWidth(), headHeight);

	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);
	sf.SetLineAlignment(StringAlignmentCenter);
	sf.SetTrimming(StringTrimmingEllipsisCharacter);
	//SolidBrush colBr(theme->GetTextColor());
	LinearGradientBrush colBr(PointF(0.0f, 4.0f), PointF(0.0f, float(headHeight-8)), theme->GetActiveStartColor(), theme->GetActiveEndColor());
	Pen separator(theme->GetActiveEndColor());

	float x = float(area.GetLeft());
	std::map<int,Column>::iterator it = _cols.begin();
	while(it!=_cols.end()) {
		Column& col = it->second;
		float w = col._width*area.GetWidth();
		g.DrawString(col._title.c_str(), (int)col._title.length(), theme->GetGUIFontBold(), RectF(x+3.0f, (float)area.GetTop()+4.0f, w, headHeight-8.0f), &sf, &colBr);
		g.DrawLine(&separator, x+w, area.GetTop()+2.0f, x+w, area.GetTop()+headHeight-4.0f);
		x += w;
		++it;
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
		if(it->first==id) {
			return cx;
		}
		cx += col._width;
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
	int h = (GetItemCount()+1)*GetItemHeightInPixels();
	if(h>ns.GetHeight()) {
		SetVerticallyScrollable(true);
		SetVerticalScrollInfo(Range<unsigned int>(0, h), ns.GetHeight());
	}	
	else {
		SetVerticallyScrollable(false);
		SetVerticalPos(0);
	}

	// update size/scrolls
	Repaint();
}

void ListWnd::OnScroll(ScrollDirection dir) {
	Repaint();
}

void ListWnd::SetColumnWidth(int id, float w) {
	std::map<int, Column>::iterator it = _cols.find(id);
	if(it!=_cols.end()) {
		it->second._width = w;
		OnColumnSizeChanged();
		Repaint();
	}
}

void ListWnd::OnColumnSizeChanged() {
}

LRESULT ListWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_MOUSEWHEEL) {
		int delta = GET_WHEEL_DELTA_WPARAM(wp);
		Area a = GetClientArea();
		
		if(a.GetHeight()<(1+GetItemCount())*GetItemHeightInPixels()) {
			if(delta<0) {
				
				SetVerticalPos(min(int(GetVerticalPos())+10, (1+GetItemCount())*GetItemHeightInPixels()-a.GetHeight()));
			}
			else {
				SetVerticalPos(max(int(GetVerticalPos())-10, 0));
			}
		}
		OnScroll(ScrollDirectionVertical);
	}
	else if(msg==WM_KEYDOWN) {
		if(LOWORD(wp)==VK_UP && _selected>0) {
			_selected--;
			Repaint();
		}
		else if(LOWORD(wp)==VK_DOWN && _selected<(GetItemCount()-1)) {
			_selected++;
			Repaint();
		}
	}
	return ChildWnd::Message(msg,wp,lp);
}

void ListWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown||ev==MouseEventRDown||ev==MouseEventLDouble) {
		Pixels ch = GetHeaderHeightInPixels();

		if(y<ch && ev!=MouseEventLDouble) {
			_dragStartX = x;
			Area r = GetClientArea();
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			int cx = 0;
			std::map<int, Column>::iterator it = _cols.begin();
			_draggingCol = -1;
			while(it!=_cols.end()) {
				Column& col = it->second;
				cx += int(r.GetWidth()*col._width);
				if(x<cx) {
					_draggingCol = it->first;
					break;
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
			int cx = 0;
			while(it!=_cols.end()) {
				Column& col = it->second;
				cx += int(col._width*area.GetWidth());
				if(x<cx) {
					if(ev==MouseEventRDown) {
						OnRightClickItem(idx, it->first);
					}
					else if(ev==MouseEventLDouble) {
						OnDoubleClickItem(idx, it->first);
					}
					else {
						OnClickItem(idx, it->first);
					}
					break;
				}
				++it;
			}
		}
	}
	else if(ev==MouseEventMove) {
		Pixels ch = GetHeaderHeightInPixels();

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
		_draggingCol = -1;
		ReleaseCapture();
		SetCursor(LoadCursor(NULL, IDC_ARROW));		
	}
}

int ListWnd::GetRowIDByHeight(int y) {
	int ih = GetItemHeightInPixels();
	if(ih==0) ih = 1;
	return (y - GetHeaderHeightInPixels() + int(GetVerticalPos()))/ih;
}

void ListWnd::OnClickItem(int id, int col) {
}

void ListWnd::OnRightClickItem(int id, int col) {
}

void ListWnd::OnDoubleClickItem(int id, int col) {
}

int ListWnd::GetItemHeightInPixels() {
	ref<Theme> theme = ThemeManager::GetTheme();
	return theme->GetMeasureInPixels(Theme::MeasureListItemHeight);
}

Area ListWnd::GetRowArea(int rid) {
	Area client = GetClientArea();
	int h = -int(GetVerticalPos())+GetHeaderHeightInPixels();
	int ih = GetItemHeightInPixels();

	h += (rid*ih);
	if(h>0 && h<client.GetBottom()) {
		return Area(client.GetLeft(), client.GetTop()+h, client.GetWidth(), ih);
	}
	return Area(0,0,0,0);
}

void ListWnd::AddColumn(std::wstring name, int id, float w) {
	_cols[id] = Column(name);
	if(w>0.0f) {
		_cols[id]._width = w;
	}
}

// Column
ListWnd::Column::Column(std::wstring title) {
	_title = title;
	_width = 0.2f;
}