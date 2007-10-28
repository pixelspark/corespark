#ifndef _TJGRAPHICSINIT_H
#define _TJGRAPHICSINIT_H

namespace tj {
	namespace shared {
		/* GDI+ Init */
		struct GraphicsInit {
			GraphicsInit() {
				Gdiplus::GdiplusStartupInput gdiplusStartupInput;
				ULONG_PTR gdiplusToken;
				Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

				INITCOMMONCONTROLSEX sex;
				sex.dwSize = sizeof(INITCOMMONCONTROLSEX);
				sex.dwICC = ICC_STANDARD_CLASSES|ICC_TAB_CLASSES|ICC_PROGRESS_CLASS|ICC_UPDOWN_CLASS|ICC_USEREX_CLASSES|ICC_WIN95_CLASSES;
				InitCommonControlsEx(&sex);
			}

			~GraphicsInit() {
			}
		}
	}
}

#endif