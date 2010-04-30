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
 
 #include "../include/tjsharedui.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

class SlicedIcon: public Icon {
	public:
		SlicedIcon(const ResourceIdentifier& rid, int marginLeft, int marginTop, int marginRight, int marginBottom);
		virtual ~SlicedIcon();
		Area GetMinimumSize() const;
		Area GetContentAreaAtSize(const Area& size) const;
		virtual void PaintSliced(graphics::Graphics& g, const Area& rc);

	protected:
		
		int _mr, _ml, _mt, _mb;
};

SlicedIcon::SlicedIcon(const ResourceIdentifier& rid, int marginLeft, int marginTop, int marginRight, int marginBottom): Icon(rid), _ml(marginLeft), _mr(marginRight), _mt(marginTop), _mb(marginBottom) {
}

SlicedIcon::~SlicedIcon() {
}

Area SlicedIcon::GetMinimumSize() const {
	float df = ThemeManager::GetTheme()->GetDPIScaleFactor();
	return Area(0,0,int((_ml+_mr)/df), int((_mt+_mb)/df));
}

Area SlicedIcon::GetContentAreaAtSize(const Area& size) const {
	float df = ThemeManager::GetTheme()->GetDPIScaleFactor();
	
	Area ns = size;
	ns.Narrow(int(_ml/df), int(_mt/df), int(_mr/df), int(_mb/df));
	return ns;
}

void SlicedIcon::PaintSliced(graphics::Graphics& g, const Area& rc) {
	strong<Theme> theme = ThemeManager::GetTheme();
	float df = theme->GetDPIScaleFactor();
	int bitmapWidth = _bitmap->GetWidth();
	int bitmapHeight = _bitmap->GetHeight();

	// Center
	g.DrawImage(_bitmap, int(rc.GetLeft()+(_ml/df)), int(rc.GetTop()+(_mt/df)), (int)_ml, (int)_mt, bitmapWidth-_ml-_mr, bitmapHeight-_mb-_mt);

	// Edges
	g.DrawImage(_bitmap, int(rc.GetLeft()), int(rc.GetTop()+(_mt/df)), 0, _mt, _ml, int(rc.GetHeight()-(_mb/df)-(_mt/df))); // left
	g.DrawImage(_bitmap, int(rc.GetRight()-(_mr/df)), int(rc.GetTop()+(_mt/df)), bitmapWidth-_mr, _mt, _mr, int(rc.GetHeight()-(_mb/df)-(_mt/df))); // right
	g.DrawImage(_bitmap, int(rc.GetLeft()+(_ml/df)), int(rc.GetTop()), _ml, 0, int(rc.GetWidth()-(_mr/df)-(_ml/df)), _mt); // top
	g.DrawImage(_bitmap, int(rc.GetLeft()+(_ml/df)), int(rc.GetBottom()-(_mb/df)), _ml, bitmapHeight-_mb, int(rc.GetWidth()-(_mr/df)-(_ml/df)), _mb); // bottom

	// Corners
	g.DrawImage(_bitmap, int(rc.GetLeft()), int(rc.GetTop()), 0, 0, _ml, _mt); // Top-left slice
	g.DrawImage(_bitmap, int(rc.GetRight()-(_mr/df)), int(rc.GetTop()), bitmapWidth-_mr, 0, _mr, _mt); // Top-right slice
	g.DrawImage(_bitmap, int(rc.GetLeft()), int(rc.GetBottom()-(_mb/df)), 0, bitmapHeight-_mb, _ml, _mb); // bottom left slice
	g.DrawImage(_bitmap, int(rc.GetRight()-(_mr/df)), int(rc.GetBottom()-(_mb/df)), bitmapWidth-_mr, bitmapHeight-_mb, _mr, _mb);
}

Icon::Icon(const ResourceIdentifier& rid): _bitmap(0) {
	if(rid!=L"") {
		std::wstring path;
		if(ResourceManager::Instance()->GetPathToLocalResource(rid, path)) {
			_bitmap = Image::FromFile(path.c_str(), TRUE);
		}

		if(_bitmap==0) {
			Log::Write(L"TJShared/Icon", L"Could not load Icon: " + rid);
		}
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

// alpha
void Icon::Paint(graphics::Graphics& g, const Area& rc, float alpha) {
	if(_bitmap==0) return;

	// Set up color matrix
	ColorMatrix cm = {
		1.0f,   0.0f,   0.0f, 0.0f, 0.0f,
		0.0f,   1.0f,   0.0f, 0.0f, 0.0f,
		0.0f,   0.0f,   1.0f, 0.0f, 0.0f,
		0.0f,	0.0f,	0.0f, alpha, 0.0f,
		0.0f,	0.0f,	0.0f, 0.0f, 1.0f,
	};

	ImageAttributes attr;
	attr.SetColorMatrix(&cm);

	g.DrawImage(_bitmap, rc, &attr);
}

// disabled + alpha
void Icon::Paint(graphics::Graphics& g, const Area& rc, bool enabled, float alpha) {
	if(_bitmap==0) return;

	if(enabled) {
		Paint(g, rc, alpha);
	}
	else {
		graphics::ColorMatrix cm = {
			0.299f, 0.299f, 0.299f, 0.0f, 0.0f,
			0.587f, 0.587f, 0.587f, 0.0f, 0.0f,
			0.114f, 0.114f, 0.114f, 0.0f, 0.0f,
			0.0f,	0.0f,	0.0f,	alpha, 0.0f,
			0.0f,	0.0f,	0.0f,	0.0f, 1.0f,
		};

		ImageAttributes attr;
		attr.SetColorMatrix(&cm);

		g.DrawImage(_bitmap, rc, &attr);
	}
}

// Disabled or normal
void Icon::Paint(graphics::Graphics& g, const Area& rc, bool enabled) {
	if(_bitmap==0) return;

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

		ImageAttributes attr;
		attr.SetColorMatrix(&cm);

		g.DrawImage(_bitmap, rc, &attr);
	}
}