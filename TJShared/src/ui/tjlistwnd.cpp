#include "../../include/tjshared.h"
#include <windowsx.h>
using namespace Gdiplus;
using namespace tj::shared;

const float ListWnd::KMinimumColumnWidth = 0.075f;

ListWnd::ListWnd(HWND parent): ChildWnd(L"", parent, true, true) {
	_draggingCol = -1;
	_dragStartX = 0;
	_selected = -1;
}

ListWnd::~ListWnd() {
}

void ListWnd::Paint(Gdiplus::Graphics &g) {
	Area area = GetClientArea();
	ref<Theme> theme = ThemeManager::GetTheme();

	SolidBrush back(theme->GetBackgroundColor());
	g.FillRectangle(&back, area);
	Pen border(theme->GetActiveStartColor(), 1.0f);
	Pen lineBorder(theme->GetActiveEndColor(), 1.0f);

	// draw items, if they fit
	int h = -int(GetVerticalPos());
	int n = GetItemCount();
	int itemHeight = GetItemHeight();

	SolidBrush colorEven(theme->GetTimeBackgroundColor());
	LinearGradientBrush colorSelected(PointF(0.0f, float(h)), PointF(0.0f, float(h+itemHeight)), theme->GetTimeSelectionColorStart(), theme->GetTimeSelectionColorEnd());

	for(int a=0;a<n;a++) {
		if(h>area.GetHeight()) {
			break;
		}
		else if(h>=-2*itemHeight) {
			Area rowArea(area.GetLeft(), area.GetTop()+KColumnHeaderHeight+h+1, area.GetWidth(), itemHeight);
			// draw a background if odd
			if(a==_selected) {
				g.FillRectangle(&colorSelected, rowArea);
			}
			else if(a%2==1) {
				g.FillRectangle(&colorEven, rowArea);
			}
			
			// draw item
			PaintItem(a, g, rowArea);
			g.DrawLine(&lineBorder, 0, h+2*itemHeight, area.GetRight(), h+2*itemHeight);
		}
		h += itemHeight;
	}

	// draw columns
	theme->DrawToolbarBackground(g, (float)area.GetLeft(), (float)area.GetTop(), (float)area.GetWidth(), (float)KColumnHeaderHeight);
	g.DrawLine(&border, 0, KColumnHeaderHeight, area.GetWidth(), KColumnHeaderHeight);

	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);
	sf.SetLineAlignment(StringAlignmentCenter);
	sf.SetTrimming(StringTrimmingEllipsisCharacter);
	//SolidBrush colBr(theme->GetTextColor());
	LinearGradientBrush colBr(PointF(0.0f, 4.0f), PointF(0.0f, float(KColumnHeaderHeight-8)), theme->GetActiveStartColor(), theme->GetActiveEndColor());
	Pen separator(theme->GetActiveEndColor());

	float x = float(area.GetLeft());
	std::map<int,Column>::iterator it = _cols.begin();
	while(it!=_cols.end()) {
		Column& col = it->second;
		float w = col._width*area.GetWidth();
		g.DrawString(col._title.c_str(), (int)col._title.length(), theme->GetGUIFontBold(), RectF(x+3.0f, (float)area.GetTop()+4.0f, w, KColumnHeaderHeight-8.0f), &sf, &colBr);
		g.DrawLine(&separator, x+w, area.GetTop()+2.0f, x+w, area.GetTop()+KColumnHeaderHeight-4.0f);
		x += w;
		it++;
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
		it++;
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
		Repaint();
	}
}

LRESULT ListWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_LBUTTONDOWN||msg==WM_RBUTTONDOWN||msg==WM_LBUTTONDBLCLK) {
		int y = GET_Y_LPARAM(lp);
		int x = GET_X_LPARAM(lp);

		if(y<KColumnHeaderHeight && msg!=WM_LBUTTONDBLCLK) {
			_dragStartX = GET_X_LPARAM(lp);
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
				it++;
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
					if(msg==WM_RBUTTONDOWN) {
						OnRightClickItem(idx, it->first);
					}
					else if(msg==WM_LBUTTONDBLCLK) {
						OnDoubleClickItem(idx, it->first);
					}
					else {
						OnClickItem(idx, it->first);
					}
					break;
				}
				it++;
			}
		}
	}
	else if(msg==WM_MOUSEMOVE) {
		int y = GET_Y_LPARAM(lp);
		int x = GET_X_LPARAM(lp);

		if(y<KColumnHeaderHeight) {
			if(_draggingCol>=0) {
				int dx = GET_X_LPARAM(lp) - _dragStartX;
				_dragStartX = GET_X_LPARAM(lp);
				Area r = GetClientArea();
				if(r.GetWidth()!=0) {
					float colWidthChange = float(dx)/float(r.GetWidth());
					_cols[_draggingCol]._width += colWidthChange; ///
					if(_cols[_draggingCol]._width<=KMinimumColumnWidth) {
						_cols[_draggingCol]._width = KMinimumColumnWidth;
					}
				}
			}
			Repaint();
		}
	}	
	else if(msg==WM_LBUTTONUP) {
		_draggingCol = -1;
		ReleaseCapture();
		SetCursor(LoadCursor(NULL, IDC_ARROW));		
		return 0;
	}
	else if(msg==WM_MOUSEWHEEL) {
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

int ListWnd::GetRowIDByHeight(int y) {
	int ih = GetItemHeight();
	if(ih==0) ih = 1;
	return (y - KColumnHeaderHeight + int(GetVerticalPos()))/ih;
}

void ListWnd::OnClickItem(int id, int col) {
}

void ListWnd::OnRightClickItem(int id, int col) {
}

void ListWnd::OnDoubleClickItem(int id, int col) {
}

int ListWnd::GetItemHeight() {
	return KColumnHeaderHeight;
}

void ListWnd::AddColumn(std::wstring name, int id) {
	_cols[id] = Column(name);
}

// Column
ListWnd::Column::Column(std::wstring title) {
	_title = title;
	_width = 0.2f;
}