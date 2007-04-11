#ifndef _TJICON_H
#define _TJICON_H

namespace tj {
	namespace shared {
		class EXPORTED Icon {
			public:
				Icon(std::wstring rid);
				virtual ~Icon();
				Icon(Gdiplus::Bitmap* bmp); // Icon will own and delete bmp!
				operator Gdiplus::Bitmap*();
				Gdiplus::Bitmap* GetBitmap();
				bool IsLoaded() const;

			protected:
				Gdiplus::Bitmap* _bitmap;
		};
	}
}

#endif