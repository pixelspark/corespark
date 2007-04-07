#ifndef _TJICON_H
#define _TJICON_H

namespace tj {
	namespace shared {
		class EXPORTED Icon {
			public:
				Icon(std::wstring rid);
				virtual ~Icon();
				operator Gdiplus::Bitmap*();
				Gdiplus::Bitmap* GetBitmap();
				bool IsLoaded() const;

			protected:
				Gdiplus::Bitmap* _bitmap;
		};
	}
}

#endif