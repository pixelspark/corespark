#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

GraphWnd::GraphWnd(): ChildWnd(L"", true, true) {
}

GraphWnd::~GraphWnd() {
}

ref<GraphItem> GraphWnd::GetItemAt(Pixels x, Pixels y) {
	std::vector< ref<GraphItem> >::iterator it = _items.begin();
	while(it!=_items.end()) {
		ref<GraphItem> gi = *it;
		if(gi->GetArea().IsInside(x,y)) {
			return gi;
		}
		++it;
	}
	return 0;
}

void GraphWnd::OnSize(const Area& ns) {
	Repaint();
}

void GraphWnd::Clear() {
	_items.clear();
	Repaint();
}

void GraphWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		_dragging = GetItemAt(x,y);
		if(_dragging) {
			_dragBeginX = x-_dragging->GetArea().GetX();
			_dragBeginY = y-_dragging->GetArea().GetY();
			Repaint();
		}
	}
	else if(ev==MouseEventMove) {
		if(_dragging) {
			Pixels nx = x-_dragBeginX;
			Pixels ny = y-_dragBeginY;
			_dragging->SetPosition(nx, ny);
			Repaint();
		}
	}
	else if(ev==MouseEventLUp) {
		_dragging = 0;
	}

	ref<GraphItem> gi = GetItemAt(x,y);
	if(gi) {
		gi->OnMouse(ev,x,y, This<GraphWnd>());
	}
}

void GraphWnd::AddItem(ref<GraphItem> gi) {
	gi->SetSelected(false);
	_items.push_back(gi);
	Repaint();
}

std::vector< ref<GraphItem> >& GraphWnd::GetItems() {
	return _items;
}

std::pair<Pixels,Pixels> GraphWnd::GetEdge(const Area& from, const Area& to) {
	Pixels mx = from.GetLeft()+(from.GetWidth()/2);
	Pixels my = from.GetTop()+(from.GetHeight()/2);
	Pixels ox = to.GetLeft()+(to.GetWidth()/2);
	Pixels oy = to.GetTop()+(to.GetHeight()/2);

	Pixels dx = abs(ox - mx);
	Pixels dy = abs(oy - my);

	if(dx>dy) {
		// use a left or right edge
		if(mx>ox) {
			// my left edge
			return std::pair<Pixels, Pixels>(from.GetLeft(), my);
		}
		else {
			// my right edge
			return std::pair<Pixels, Pixels>(from.GetRight(), my);
		}
	}
	else {
		// use a top or bottom edge
		if(my>oy) {
			// my top edge
			return std::pair<Pixels, Pixels>(mx, from.GetTop());
		}
		else {
			// my bottom edge
			return std::pair<Pixels, Pixels>(mx, from.GetBottom());
		}
	}
}

void GraphWnd::Paint(Graphics& g) {
	ref<Theme> theme = ThemeManager::GetTheme();
	Area rc = GetClientArea();

	SolidBrush backBrush(theme->GetBackgroundColor());
	g.FillRectangle(&backBrush, rc);
	g.SetSmoothingMode(SmoothingModeHighQuality);
	SolidBrush textBrush(theme->GetTextColor());

	std::vector< ref<GraphItem> >::iterator it = _items.begin();
	while(it!=_items.end()) {
		ref<GraphItem> item = *it;
		item->Paint(g, theme);

		// Draw arrows
		Pen arrowOut(theme->GetLineColor(), 2.0f);
		Pen arrowIn(theme->GetLineColor(), 2.0f);
		Pen normal(theme->GetLineColor(), 1.0f);
		Pen outLight(theme->GetLineColor(), 1.0f);
		AdjustableArrowCap aac(5.0f, 5.0f);
		arrowOut.SetCustomEndCap(&aac);
		arrowIn.SetCustomStartCap(&aac);
		outLight.SetCustomEndCap(&aac);

		std::vector<GraphArrow>::iterator ait = item->_arrows.begin();
		while(ait!=item->_arrows.end()) {
			GraphArrow& ga = *ait;
			ref<GraphItem> other = ga._to;

			if(other) {
				// get areas
				Area& otherArea = other->GetArea();
				Area& myArea = item->GetArea();

				// get edges
				std::pair< Pixels, Pixels> fromEdge = GetEdge(myArea, otherArea);
				std::pair<Pixels,Pixels> toEdge = GetEdge(otherArea, myArea);

				Pen* pn = &normal;
				if(ga._direction==GraphArrow::Out) {
					pn = &arrowOut;
				}
				else if(ga._direction==GraphArrow::In) {
					pn = &arrowIn;
				}

				g.DrawLine(pn, fromEdge.first, fromEdge.second, toEdge.first, toEdge.second);

				// draw text
				Pixels tx = (fromEdge.first+toEdge.first)/2 + 4;
				Pixels ty = (fromEdge.second+toEdge.second)/2 + 4;
				g.DrawString(ga._text.c_str(), (int)ga._text.length(), theme->GetGUIFontSmall(), PointF(float(tx),float(ty)), &textBrush);
			}
			++ait;
		}

		++it;
	}
}

GraphItem::GraphItem() {
}

GraphItem::~GraphItem() {
}

bool GraphItem::IsSelected() const {
	return _selected;
}

void GraphItem::SetSelected(bool t) {
	_selected = t;
}

Area& GraphItem::GetArea() {
	return _area;
}

void GraphItem::OnMouse(MouseEvent me, Pixels x, Pixels y, ref<GraphWnd> gw) {
}

void GraphItem::AddArrow(ref<GraphItem> to, GraphArrow::Direction dir, const std::wstring& text) {
	GraphArrow ga;
	ga._direction = dir;
	ga._to = to;
	ga._text = text;
	_arrows.push_back(ga);
}

void GraphItem::SetPosition(Pixels x, Pixels y) {
	_area.SetX(x);
	_area.SetY(y);
}

void GraphItem::SetSize(Pixels w, Pixels h) {
	_area.SetWidth(w);
	_area.SetHeight(h);
}

SimpleGraphItem::SimpleGraphItem() {
}

SimpleGraphItem::~SimpleGraphItem() {
}

void SimpleGraphItem::SetText(const std::wstring& t) {
	_text = t;
}

void SimpleGraphItem::SetColor(const Gdiplus::Color& c) {
	_color = c;
}

std::wstring SimpleGraphItem::GetText() const {
	return _text;
}

void SimpleGraphItem::Paint(Gdiplus::Graphics& g, ref<Theme> theme) {
	LinearGradientBrush brush(PointF(0.0f, float(_area.GetTop())), PointF(0.0f, float(_area.GetBottom())), Theme::ChangeAlpha(_color, 200), _color);
	Pen border(_color, 1.0f);
	g.FillRectangle(&brush, _area);
	g.DrawRectangle(&border, _area);
	
	SolidBrush textBrush(theme->GetTextColor());
	StringFormat sf;
	sf.SetAlignment(StringAlignmentCenter);
	sf.SetLineAlignment(StringAlignmentCenter);
	g.DrawString(_text.c_str(), (int)_text.length(), theme->GetGUIFontBold(), _area, &sf, &textBrush);
}