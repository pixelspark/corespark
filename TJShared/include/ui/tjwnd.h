#ifndef _TJWND_H
#define _TJWND_H
#include "../tjplatform.h"

namespace tj {
	namespace shared {

		/** Class for initializing GDI+ **/
		class EXPORTED GraphicsInit {
			public:
				GraphicsInit();
				virtual ~GraphicsInit();
		};

		class EXPORTED Displays: public virtual Object {
			public:
				Displays();
				virtual ~Displays();
				void AddDisplay(HMONITOR hm);
				RECT GetDisplayRectangle(int idx);
			protected:
				std::vector<HMONITOR> _displays;
		};

		typedef int Pixels;

		enum ScrollDirection {
			ScrollDirectionNone = 0,
			ScrollDirectionHorizontal = 1,
			ScrollDirectionVertical,
		};

		class EXPORTED Wnd: public virtual Object {
			friend class FloatingPane;

			public:
				Wnd(const wchar_t* title, HWND parent=0, const wchar_t* className=TJ_DEFAULT_CLASS_NAME,  bool useDoubleBuffering=true, int exStyle=0L);
				virtual ~Wnd();
				
				virtual void Show(bool s);
				bool IsShown() const;
				void Repaint();
				void SetQuitOnClose(bool q);
				virtual void Layout();
				virtual void Update();
				virtual LRESULT PreMessage(UINT msg, WPARAM wp, LPARAM lp);
				virtual void SetText(const wchar_t* t);
				void SetStyle(DWORD style);
				void SetStyleEx(DWORD style);
				void UnsetStyle(DWORD style);
				void UnsetStyleEx(DWORD style);
				bool IsMouseOver();
				virtual void SetFullScreen(bool f);
				virtual void SetFullScreen(bool f, int display);
				bool IsFullScreen();
				bool HasFocus();

				// Scrolling
				void SetHorizontallyScrollable(bool s);
				void SetVerticallyScrollable(bool s);
				unsigned int GetHorizontalPos();
				unsigned int GetVerticalPos();
				void SetVerticalPos(unsigned int p);
				void SetHorizontalPos(unsigned int p);
				void SetHorizontalScrollInfo(Range<unsigned int> rng, unsigned int pageSize);
				void SetVerticalScrollInfo(Range<unsigned int> rng, unsigned int pageSize);

				virtual void Move(int x, int y, int w, int h);
				virtual bool IsSplitter();

				virtual std::wstring GetText();
				virtual void SetText(std::wstring text);
				void SetSize(int w, int h);
				Area GetClientArea();
				Area GetWindowArea();
				virtual void Fill(LayoutFlags flags, Area& rect);
				void Fill();

				Wnd* GetParent();
				Wnd* GetRootWindow();

				void SetWantMouseLeave(bool t);
				bool GetWantMouseLeave() const;

				HWND GetWindow();
				virtual std::wstring GetTabTitle() const;		// return an empty string if you don't want to override Pane's title
				virtual Gdiplus::Bitmap* GetTabIcon() const;	// should return 0 when you don't want to override the tab icon set in Pane
				virtual void Focus();

			protected:
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);

				// Messages
				virtual void Paint(Gdiplus::Graphics& g) = 0;
				virtual void OnSize(const Area& newSize);
				virtual void OnScroll(ScrollDirection dir);
				virtual void OnActivate(bool activate);
			
			private:
				static void RegisterClasses();

				Gdiplus::Bitmap* _buffer;
				bool _doubleBuffered;
				bool _quitOnClose;
				unsigned int _horizontalPos;
				unsigned int _verticalPos;
				unsigned int _horizontalPageSize;
				unsigned int _verticalPageSize;
				static bool _classesRegistered;
				long _oldStyle, _oldStyleEx;
				HWND _wnd;
				bool _fullScreen;
				bool _wantsMouseLeave;
		};

		/*class ChildEnumerator {
			public:
				ChildEnumerator(HWND parent, bool recursive=false);
				void Add(HWND wnd);
			
				std::vector<Wnd*> _children;
				bool _recursive;
				HWND _for;
		};*/
	}
}

#endif