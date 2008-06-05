#ifndef _TJSPLASH_H
#define _TJSPLASH_H

namespace tj {
	namespace shared {
		class EXPORTED SplashWnd: public PopupWnd {
			public:
				SplashWnd(std::wstring path, Pixels w, Pixels h);
				virtual ~SplashWnd();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void Layout();

			protected:
				graphics::Image* _image;
				ref<ProgressWnd> _progress;
		};

		class EXPORTED SplashThread: public Thread {
			public:
				SplashThread(std::wstring path, int w, int h);
				virtual ~SplashThread();
				virtual void Run();
				void Hide();
			protected:
				int _w, _h;
				std::wstring _path;
				ref<SplashWnd> _wnd;
				Event _closeEvent;
		};
	}
}

#endif
