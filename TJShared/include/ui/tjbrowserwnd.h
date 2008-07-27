#ifndef _TJBROWSERWND_H
#define _TJBROWSERWND_H

namespace tj {
	namespace shared {
		class BrowserSink;

		class EXPORTED BrowserWnd: public ChildWnd {
			friend class BrowserSink;

			public:
				BrowserWnd(const std::wstring& title);
				virtual ~BrowserWnd();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				void Hide(bool t);
				void Navigate(const std::wstring& url);
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