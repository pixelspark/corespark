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