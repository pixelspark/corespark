#include "../include/tjshared.h"
#include <math.h>
using namespace tj::shared;
using namespace Gdiplus;

Vector::Vector(float fx, float fy, float fz): x(fx), y(fy), z(fz) {
}

Vector::Vector(const Vector& other) {
	x = other.x;
	y = other.y;
	z = other.z;
}

Vector::~Vector() {
}

std::wstring Vector::ToString() const {
	std::wostringstream wos;
	wos << L'[' << x << L',' << y << L',' << z << L']';
	return wos.str();
}

Vector Vector::Dot(const Vector& other) const {
	return Vector(x*other.x, y*other.y, z*other.z);
}

float Vector::GetLength() const {
	return sqrt(x*x + y*y + z*z);
}

void Vector::Save(TiXmlElement* parent) {
	SaveAttribute(parent, "x", x);
	SaveAttribute(parent, "y", y);
	SaveAttribute(parent, "z", z);
}

void Vector::Load(TiXmlElement* you) {
	x = LoadAttribute(you, "x", x);
	y = LoadAttribute(you, "y", y);
	z = LoadAttribute(you, "z", z);
}

namespace tj {
	namespace shared {
		class VectorPropertyWnd: public ChildWnd {
			public:
				VectorPropertyWnd(HWND parent, Vector* vec): ChildWnd(L"", parent, false, false), _vec(vec) {
					SetStyleEx(WS_EX_CONTROLPARENT);
					_x = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL, 0, 0, 0, 0, _wnd, (HMENU)1, GetModuleHandle(NULL), 0L);
					_y = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL, 0, 0, 0, 0, _wnd, (HMENU)2, GetModuleHandle(NULL), 0L);
					_z = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL, 0, 0, 0, 0, _wnd, (HMENU)3, GetModuleHandle(NULL), 0L);
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

				virtual void Paint(Gdiplus::Graphics& g) {
				}

				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp) {
					if(msg==WM_SIZE) {
						Layout();
					}
					else if(msg==WM_NOTIFY||msg==WM_COMMAND) {
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
					tj::shared::Rectangle rc = GetClientRectangle();
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
				Vector* _vec;
				HWND _x, _y, _z;
				HFONT _font;
		};
	}
}

/* VectorProperty */
VectorProperty::VectorProperty(std::wstring name, Vector* vec): Property(name), _vec(vec) {
	_wnd = 0;
}

VectorProperty::~VectorProperty() {
	if(_wnd!=0) {
		delete _wnd;
	}
}

std::wstring VectorProperty::GetValue() {
	return _vec->ToString();
}

HWND VectorProperty::GetWindow() {
	if(_wnd==0) {
		return 0;
	}
	return _wnd->GetWindow();
}

HWND VectorProperty::Create(HWND parent) {
	if(_wnd==0) {
		_wnd = new VectorPropertyWnd(parent, _vec);
	}
	return _wnd->GetWindow();
}

void VectorProperty::Changed() {
}

void VectorProperty::Update() {
	if(_wnd!=0) {
		_wnd->Update();
	}
}