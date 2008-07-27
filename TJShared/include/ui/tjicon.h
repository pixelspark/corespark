#ifndef _TJICON_H
#define _TJICON_H

namespace tj {
	namespace shared {
		class EXPORTED Icon {
			public:
				Icon(std::wstring rid);
				virtual ~Icon();
				Icon(graphics::Image* bmp); // Icon will own and delete bmp!
				operator graphics::Image*();
				graphics::Image* GetBitmap();
				bool IsLoaded() const;
				void Paint(graphics::Graphics& g, const Area& rc, bool enabled = true);
				void Paint(graphics::Graphics& g, const Area& rc, float alpha);
				void Paint(graphics::Graphics& g, const Area& rc, bool enabled, float alpha);

			protected:
				graphics::Image* _bitmap;
		};
	}
}

#endif