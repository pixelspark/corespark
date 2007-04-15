#ifndef _TJICON_H
#define _TJICON_H

namespace tj {
	namespace shared {
		class EXPORTED Icon {
			public:
				Icon(std::wstring rid);
				virtual ~Icon();
				Icon(Gdiplus::Image* bmp); // Icon will own and delete bmp!
				operator Gdiplus::Image*();
				Gdiplus::Image* GetBitmap();
				bool IsLoaded() const;

			protected:
				Gdiplus::Image* _bitmap;
		};
	}
}

#endif