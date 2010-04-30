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
 
 #ifndef _TJBROWSERWND_H
#define _TJBROWSERWND_H

namespace tj {
	namespace shared {
		class BrowserSink;

		class EXPORTED BrowserWnd: public ChildWnd {
			friend class BrowserSink;

			public:
				BrowserWnd(const String& title);
				virtual ~BrowserWnd();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				void Hide(bool t);
				void Navigate(const String& url);
				virtual void Layout();
				virtual void SetShowToolbar(bool t);
				virtual void Back();
				virtual void Forward();
				virtual void Stop();
				virtual void Reload();

			protected:
				virtual void OnSize(const Area& newSize);

				void* _ax;
				ref<ToolbarWnd> _tools;
				BrowserSink* _sink;
		};
	}
}

#endif