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
				virtual void OnCreated();
				virtual void OnSize(const Area& ns);
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
