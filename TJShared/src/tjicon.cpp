#include "../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

Icon::Icon(std::wstring rid) {
	std::wstring path = ResourceManager::Instance()->Get(rid, true);
	_bitmap = Bitmap::FromFile(path.c_str(), TRUE);

	if(_bitmap==0) {
		Log::Write(L"TJShared/Icon", L"Could not load Icon: " + rid);
	}
}

Icon::Icon(Gdiplus::Bitmap* bmp) {
	_bitmap = bmp;
}

Icon::~Icon() {
	delete _bitmap;
}

bool Icon::IsLoaded() const {
	return _bitmap!=0;
}

Gdiplus::Bitmap* Icon::GetBitmap() {
	return _bitmap;
}

Icon::operator Gdiplus::Bitmap*() {
	return _bitmap;
}