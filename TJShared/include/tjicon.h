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

			protected:
				graphics::Image* _bitmap;
		};
	}
}

#endif