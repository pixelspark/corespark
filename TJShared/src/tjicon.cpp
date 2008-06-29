#include "../include/tjshared.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

Icon::Icon(std::wstring rid) {
	std::wstring path = ResourceManager::Instance()->Get(rid, true);
	_bitmap = Image::FromFile(path.c_str(), TRUE);

	if(_bitmap==0) {
		Log::Write(L"TJShared/Icon", L"Could not load Icon: " + rid);
	}
}

Icon::Icon(graphics::Image* bmp) {
	_bitmap = bmp;
}

Icon::~Icon() {
	delete _bitmap;
}

bool Icon::IsLoaded() const {
	return _bitmap!=0;
}

graphics::Image* Icon::GetBitmap() {
	return _bitmap;
}

Icon::operator graphics::Image*() {
	return _bitmap;
}

void Icon::Paint(graphics::Graphics& g, const Area& rc, bool enabled) {
	if(enabled) {
		g.DrawImage(_bitmap, rc);
	}
	else {
		graphics::ColorMatrix cm = {
			0.299f, 0.299f, 0.299f, 0.0f, 0.0f,
			0.587f, 0.587f, 0.587f, 0.0f, 0.0f,
			0.114f, 0.114f, 0.114f, 0.0f, 0.0f,
			0.0f,	0.0f,	0.0f,	1.0f, 0.0f,
			0.0f,	0.0f,	0.0f,	0.0f, 1.0f,
		};

		Gdiplus::ImageAttributes attr;
		attr.SetColorMatrix(&cm);

		g.DrawImage(_bitmap, rc, 0.0f, 0.0f, (float)_bitmap->GetWidth(), (float)_bitmap->GetHeight(), graphics::UnitPixel, &attr);
	}
}