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
#include <math.h>
using namespace tj::shared;
using namespace tj::shared::graphics;

namespace tj {
	namespace shared {
		class VectorPropertyWnd: public ChildWnd {
			public:
				VectorPropertyWnd(Vector* vec): ChildWnd(true), _vec(vec) {
					SetStyleEx(WS_EX_CONTROLPARENT);
					_x = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL, 0, 0, 0, 0, GetWindow(), (HMENU)1, GetModuleHandle(NULL), 0L);
					_y = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL, 0, 0, 0, 0, GetWindow(), (HMENU)2, GetModuleHandle(NULL), 0L);
					_z = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL, 0, 0, 0, 0, GetWindow(), (HMENU)3, GetModuleHandle(NULL), 0L);
					Layout();
					Update();

					_font = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TL(ui_font));
					SendMessage(_x, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);
					SendMessage(_y, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);
					SendMessage(_z, WM_SETFONT, (WPARAM)(HFONT)_font, FALSE);

					ShowWindow(_x, SW_SHOW);
					ShowWindow(_y, SW_SHOW);
					ShowWindow(_z, SW_SHOW);
				}

				virtual ~VectorPropertyWnd() {
					DestroyWindow(_x);
					DestroyWindow(_y);
					DestroyWindow(_z);
					DeleteObject((HGDIOBJ)_font);
				}

				virtual void SetDimensionShown(unsigned int d, bool h) {
					HWND editWindow = 0L;
					
					switch(d) {
						case 0:
							editWindow = _x;
							break;

						case 1:
							editWindow = _y;
							break;

						case 2:
							editWindow = _z;
							break;
					}

					if(editWindow!=0L) {
						ShowWindow(editWindow, h ? SW_SHOW : SW_HIDE);
					}
				}

				virtual void Paint(graphics::Graphics& g, strong<Theme> theme) {
					SolidBrush back(theme->GetColor(Theme::ColorBackground));
					Area rc = GetClientArea();
					g.FillRectangle(&back, rc);
				}

				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp) {
					if(msg==WM_NOTIFY||msg==WM_COMMAND) {
						wchar_t buffer[34];
						GetWindowText(_x, buffer, 32);
						std::wstring tx = buffer;
						GetWindowText(_y, buffer, 32);
						std::wstring ty = buffer;
						GetWindowText(_z, buffer, 32);
						std::wstring tz = buffer;

						_vec->x = StringTo<float>(tx, 0.0f);
						_vec->y = StringTo<float>(ty, 0.0f);
						_vec->z = StringTo<float>(tz, 0.0f);
					}
					return ChildWnd::Message(msg,wp,lp);
				}

				virtual void Layout() {
					Area rc = GetClientArea();
					strong<Theme> theme = ThemeManager::GetTheme();
					rc.MultiplyCeil(theme->GetDPIScaleFactor(), theme->GetDPIScaleFactor());

					int w = rc.GetWidth()/3;
					SetWindowPos(_x, 0L, 0, 0, w, rc.GetHeight(), SWP_NOZORDER);
					SetWindowPos(_y, 0L, w-1, 0, w, rc.GetHeight(), SWP_NOZORDER);
					SetWindowPos(_z, 0L, 2*w-2, 0, w+2, rc.GetHeight(), SWP_NOZORDER);
				}

				virtual void Update() {
					std::wstring x = Stringify(_vec->x);
					std::wstring y = Stringify(_vec->y);
					std::wstring z = Stringify(_vec->z);
					SetWindowText(_x, x.c_str());
					SetWindowText(_y, y.c_str());
					SetWindowText(_z, z.c_str());
				}

				virtual wchar_t GetPreferredHotkey() {
					return L'V';
				}

			protected:
				virtual void OnSize(const Area& s) {
					Layout();
				}

				Vector* _vec;
				HWND _x, _y, _z;
				HFONT _font;
		};
	}
}

/* VectorProperty */
VectorProperty::VectorProperty(const std::wstring& name, Vector* vec): Property(name), _vec(vec) {
}

VectorProperty::~VectorProperty() {
}

ref<Wnd> VectorProperty::GetWindow() {
	if(!_wnd) {
		_wnd = GC::Hold(new VectorPropertyWnd(_vec));
	}
	return _wnd;
}

void VectorProperty::Update() {
	if(_wnd) {
		_wnd->Update();
	}
}

void VectorProperty::SetDimensionShown(unsigned int d, bool h) {
	GetWindow();
	ref<VectorPropertyWnd>(_wnd)->SetDimensionShown(d,h);
}