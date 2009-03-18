#ifndef _TJSPLASH_H
#define _TJSPLASH_H

namespace tj {
	namespace shared {
		class EXPORTED SplashWnd: public PopupWnd {
			public:
				SplashWnd(String path, Pixels w, Pixels h);
				virtual ~SplashWnd();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void Layout();

			protected:
				graphics::Image* _image;
				ref<ProgressWnd> _progress;
		};

		class EXPORTED SplashThread: public Thread {
			public:
				SplashThread(String path, int w, int h);
				virtual ~SplashThread();
				virtual void Run();
				void Hide();
			protected:
				int _w, _h;
				String _path;
				ref<SplashWnd> _wnd;
				Event _closeEvent;
		};
	}
}

#endif
