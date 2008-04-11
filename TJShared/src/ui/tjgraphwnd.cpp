#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

// TODO: move to some general class in TJShared
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j) {
      if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

GraphWnd::GraphWnd(): ChildWnd(L"", true, true) {
}

GraphWnd::~GraphWnd() {
}

ref<GraphItem> GraphWnd::GetItemAt(Pixels x, Pixels y) {
	std::vector< ref<GraphItem> >::iterator it = _items.begin();
	while(it!=_items.end()) {
		ref<GraphItem> gi = *it;
		if(!gi->IsHidden() && gi->GetArea().IsInside(x,y)) {
			return gi;
		}
		++it;
	}
	return 0;
}

void GraphWnd::SaveImage(const std::wstring& path) {
	ref<Theme> theme = ThemeManager::GetTheme();

	if(path.length()>4) {
		std::wstring ext = path.substr(path.length()-3, 3);
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

		if(ext==L"emf") {
			HDC windowDC = GetDC(GetWindow());

			{
				Gdiplus::Metafile mf(path.c_str(), windowDC);
				Gdiplus::Graphics g(&mf);
				Paint(g, theme);
			}
			ReleaseDC(GetWindow(), windowDC);					
		}
		else {
			if(ext==L"jpg") ext = L"jpeg";
			std::wstring mime = std::wstring(L"image/")+ext;
			Area rc = GetClientArea();
			Bitmap* bmp = new Bitmap(rc.GetWidth(), rc.GetHeight());
			Gdiplus::Graphics g(bmp);
			Paint(g, theme);
			CLSID pngClsid;
			GetEncoderClsid(mime.c_str(), &pngClsid);
			bmp->Save(path.c_str(),&pngClsid, NULL);
			delete bmp;
		}
	}
}

void GraphWnd::OnSize(const Area& ns) {
	Repaint();
}

void GraphWnd::Clear() {
	_items.clear();
	Repaint();
}

void GraphWnd::HideAll() {
	std::vector< ref<GraphItem> >::iterator it = _items.begin();
	while(it!=_items.end()) {
		ref<GraphItem> gi = *it;
		if(gi) {
			gi->Hide(true);
		}
		++it;
	}
	Repaint();
}

void GraphWnd::OnFocus(bool f) {
	Repaint();
}

void GraphWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		Focus();
		_dragging = GetItemAt(x,y);
		_focus = _dragging;

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
		gi->OnMouse(ev, x, y, this);
	}
	else if(ev==MouseEventRDown) {
		ContextMenu cm;

		std::vector< ref<GraphItem> > inMenu;
		std::vector< ref<GraphItem> >::iterator it = _items.begin();
		while(it!=_items.end()) {
			ref<GraphItem> item = *it;
			if(item->IsHidden()) {
				// add to the list
				inMenu.push_back(item);
				cm.AddItem(item->GetText(), (int)inMenu.size(), false, false);
			}
			++it;
		}

		int r = cm.DoContextMenu(this);
		if(r>0 && r<=int(inMenu.size())) {
			ref<GraphItem> gi = inMenu.at(r-1);
			if(gi) {
				gi->SetPosition(x,y);
				gi->Hide(false);
				Update();
			}
		}
	}
}

void GraphWnd::Update() {
	Repaint();
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

void GraphWnd::Paint(Graphics& g, ref<Theme> theme) {
	Area rc = GetClientArea();

	SolidBrush backBrush(theme->GetBackgroundColor());
	g.FillRectangle(&backBrush, rc);
	g.SetSmoothingMode(SmoothingModeHighQuality);
	SolidBrush textBrush(theme->GetTextColor());

	std::vector< ref<GraphItem> >::iterator it = _items.begin();
	while(it!=_items.end()) {
		ref<GraphItem> item = *it;
		if(!item->IsHidden()) {
			item->Paint(g, theme);
			if(HasFocus() && _focus==item) {
				theme->DrawFocusRectangle(g, item->GetArea());
			}

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

				if(other && !other->IsHidden()) {
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
		}

		++it;
	}
}

void GraphWnd::OnContextMenu(Pixels x, Pixels y) {
	if(_focus) {
		_focus->OnContextMenu(x,y, this);
	}
}

void GraphWnd::OnKey(Key k, wchar_t key, bool down, bool isAccelerator) {
	if(_focus) {
		Area rc = _focus->GetArea();
		if(k==KeyLeft) {
			_focus->SetPosition(max(0,rc.GetLeft()-10), rc.GetTop());
		}
		else if(k==KeyRight) {
			_focus->SetPosition(max(0,rc.GetLeft()+10), rc.GetTop());
		}
		else if(k==KeyUp) {
			_focus->SetPosition(max(0,rc.GetLeft()), max(0,rc.GetTop()-10));
		}
		else if(k==KeyDown) {
			_focus->SetPosition(rc.GetLeft(), rc.GetTop()+10);
		}
		else if(k==KeyCharacter && key==VK_DELETE) {
			_focus->Hide(true);
		}
		else {
			_focus->OnKey(k,key,down,isAccelerator, this);
		}
	}

	Repaint();
}

GraphItem::GraphItem() {
	_selected = false;
	_hidden = false;
}

GraphItem::~GraphItem() {
}

bool GraphItem::IsSelected() const {
	return _selected;
}

void GraphItem::SetSelected(bool t) {
	_selected = t;
}

bool GraphItem::IsHidden() const {
	return _hidden;
}

void GraphItem::Hide(bool h) {
	_hidden = h;
}

void GraphItem::OnContextMenu(Pixels x, Pixels y, ref<GraphWnd> gw) {
	ref<ContextMenu> cm = GC::Hold(new ContextMenu());
	enum {KCHide=1};

	cm->AddItem(TL(graph_item_hide), KCHide, false, _hidden);

	int r = cm->DoContextMenu(gw, x, y);
	if(r==KCHide) {
		Hide(true);
	}

	gw->Update();
	
}
void GraphItem::OnKey(Key k, wchar_t key, bool down, bool isAccelerator, ref<GraphWnd> gw) {
}

Area& GraphItem::GetArea() {
	return _area;
}

GraphArrow::GraphArrow() {
}

void GraphItem::OnMouse(MouseEvent me, Pixels x, Pixels y, ref<GraphWnd> gw) {
	if(me==MouseEventRDown) {
		OnContextMenu(x,y, gw);
	}
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