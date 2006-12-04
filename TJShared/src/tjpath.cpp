#include "../include/tjshared.h"
#include <windowsx.h>
using namespace tj::shared;
using namespace Gdiplus;

/* Path */
Path::Path() {
}

Path::~Path() {
}

void Path::Add(std::wstring text, std::wstring icon, ref<Inspectable> is) {
	_crumbs.push_back(GC::Hold(new Crumb(text,icon,is)));
}

void Path::Add(ref<Crumb> r) {
	_crumbs.push_back(r);
}

Path::Crumb::Crumb(std::wstring text, std::wstring icon, ref<Inspectable> subject) {
	_text = text;
	std::wstring icfn = ResourceManager::Instance()->Get(icon);
	_icon = Bitmap::FromFile(icfn.c_str(), TRUE);
	_subject = subject;
}

Path::Crumb::~Crumb() {
	delete _icon;
}

/* PathWnd*/
PathWnd::PathWnd(HWND parent): ChildWnd(L"", parent, true, true) {
	std::wstring spfn = ResourceManager::Instance()->Get(L"icons/path_separator.png");
	_separator = Bitmap::FromFile(spfn.c_str(), TRUE);
	SetWantMouseLeave(true);
}

PathWnd::~PathWnd() {
	delete _separator;
}

void PathWnd::Update() {
	Repaint();
}

ref<Path::Crumb> PathWnd::GetCrumbAt(int x, int* left) {
	if(_path) {
		ref<Theme> theme = ThemeManager::GetTheme();
		HDC dc = GetDC(_wnd);
		{
			Graphics g(dc);
			std::vector< ref<Path::Crumb> >::iterator it = _path->_crumbs.begin();
			int rx = 1;
			while(it!=_path->_crumbs.end()) {
				ref<Path::Crumb> crumb = *it;
				RectF textrc;
				g.MeasureString(crumb->_text.c_str(), int(crumb->_text.length()), theme->GetGUIFont(), PointF(0.0f,0.0f), &textrc);
				int totalWidth = int(textrc.Width)+KMarginLeft+KMarginRight+KIconWidth;
				rx += totalWidth;

				if(x<rx) {
					if(left!=0) {
						*left = rx-totalWidth;
					}
					return crumb;
				}
				it++;
			}
		}
		ReleaseDC(_wnd, dc);
	}
	
	return 0;
}

void PathWnd::Paint(Gdiplus::Graphics& g) {
	tj::shared::Rectangle rc = GetClientRectangle();
	ref<Theme> theme = ThemeManager::GetTheme();
	
	SolidBrush zwart(theme->GetBackgroundColor());
	g.FillRectangle(&zwart, rc);

	LinearGradientBrush br(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetToolbarColorStart(), theme->GetToolbarColorEnd());
	SolidBrush dbr(theme->GetDisabledOverlayColor());
	g.FillRectangle(&br, rc);
	g.FillRectangle(&dbr, rc);

	LinearGradientBrush glas(PointF(0.0f,0.0f), PointF(0.0f,float(rc.GetHeight())/2.0f), theme->GetGlassColorStart(), theme->GetGlassColorEnd());
	g.FillRectangle(&glas, RectF(0.0f, 0.0f, float(rc.GetWidth()), float(rc.GetHeight())/2.0f));

	Pen pn(theme->GetActiveEndColor(), 1.0f);
	g.DrawLine(&pn, PointF(0.0f, float(rc.GetHeight()-1.0f)), PointF(float(rc.GetWidth()), float(rc.GetHeight()-1.0f)));
	
	// knopjes!
	if(_path) {
		SolidBrush tbr(theme->GetActiveEndColor());
		SolidBrush atbr(theme->GetTextColor());

		std::vector< ref<Path::Crumb> >::iterator it = _path->_crumbs.begin();
		int rx = 1;
		while(it!=_path->_crumbs.end()) {
			ref<Path::Crumb> crumb = *it;
			/*

			| margin_l | icon-size | text-size | margin_r

			*/
			RectF textrc;
			g.MeasureString(crumb->_text.c_str(), int(crumb->_text.length()), theme->GetGUIFont(), PointF(0.0f,0.0f), &textrc);
			RectF rtext(float(rx+KMarginLeft+KIconWidth), 4.0f, float(textrc.Width+1), float(15.0f));

			StringFormat sf;
			sf.SetAlignment(StringAlignmentFar);

			// active item draws differently
			if(it+1 == _path->_crumbs.end()) {
				LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetActiveStartColor(), theme->GetActiveEndColor());
				g.FillRectangle(&lbr,RectF(float(rx), 1.0f, float(textrc.Width+KMarginLeft+KMarginRight+KIconWidth), float(rc.GetHeight()-3)));
				
				if(_over!=crumb) {
					SolidBrush dbr(theme->GetDisabledOverlayColor());
					g.FillRectangle(&dbr,RectF(float(rx), 1.0f, float(textrc.Width+KMarginLeft+KMarginRight+KIconWidth), float(rc.GetHeight()-3)));
				}
				
				g.DrawString(crumb->_text.c_str(), int(crumb->_text.length()), theme->GetGUIFont(), rtext, &sf, &atbr);
			}
			else { 
				if(_over==crumb) {
					LinearGradientBrush lbr(PointF(0.0f, 0.0f), PointF(0.0f, float(rc.GetHeight())), theme->GetActiveStartColor(), theme->GetActiveEndColor());
					g.FillRectangle(&lbr,RectF(float(rx), 1.0f, float(textrc.Width+KMarginLeft+KMarginRight+KIconWidth), float(rc.GetHeight()-3)));
					g.DrawString(crumb->_text.c_str(), int(crumb->_text.length()), theme->GetGUIFont(), rtext, &sf, &atbr);
				}
				else {
					g.DrawString(crumb->_text.c_str(), int(crumb->_text.length()), theme->GetGUIFont(), rtext, &sf, &tbr);
				}

				// draw separator after
				if(_separator!=0) {
					g.DrawImage(_separator, RectF(float(rx)+textrc.Width+KMarginLeft+KIconWidth,(rc.GetHeight()-KIconWidth)/2.0f, float(KIconWidth), float(KIconWidth)));
				}
			}

			// draw icon
			if(crumb->_icon!=0) {
				g.DrawImage(crumb->_icon, RectF(float(rx+KMarginLeft/2), (rc.GetHeight()-KIconWidth)/2.0f, float(KIconWidth), float(KIconWidth)));
			}

			rx += int(textrc.Width)+KMarginLeft+KMarginRight+KIconWidth;
			it++;
		}
	}
}

wchar_t PathWnd::GetPreferredHotkey() {
	return L'P';
}

LRESULT PathWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_LBUTTONDOWN) {
		int x = GET_X_LPARAM(lp);
		int left = 0;
		ref<Path::Crumb> cr = GetCrumbAt(x,&left);
		if(cr) {
			DoCrumbMenu(cr, left);
		}
		return 0;
	}
	else if(msg==WM_MOUSEMOVE) {
		int x = GET_X_LPARAM(lp);
		_over = GetCrumbAt(x);
		Repaint();
	}
	else if(msg==WM_MOUSELEAVE) {
		_over = 0;
		Repaint();
	}
	return ChildWnd::Message(msg,wp,lp);
}

void PathWnd::SetPath(ref<Path> p) {
	_path = p;
	Update();
}

void PathWnd::DoCrumbMenu(ref<Path::Crumb> crumb, int x) {
	tj::shared::Rectangle rc = GetClientRectangle();

	ContextMenu cm;
	cm.AddItem(crumb->_text, 0, true, false);
	int command = cm.DoContextMenu(_wnd, x, rc.GetHeight()-1, true);

}