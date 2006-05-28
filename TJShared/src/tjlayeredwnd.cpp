#include "../include/tjshared.h"
using namespace tj::shared;

LayeredWnd::LayeredWnd(std::wstring title, HWND parent): Wnd(title.c_str(), parent, TJ_DEFAULT_CLASS_NAME, true, WS_EX_LAYERED) {
	SetTransparency(1.0f);	
}

LayeredWnd::~LayeredWnd() {
}

void LayeredWnd::SetTransparency(float f) {
	SetLayeredWindowAttributes(_wnd, 0, int(f*255.0f), LWA_ALPHA);
}
