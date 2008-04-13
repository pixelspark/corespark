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

		enum ScrollDirection {
			ScrollDirectionNone = 0,
			ScrollDirectionHorizontal = 1,
			ScrollDirectionVertical,
		};

		enum MouseEvent {
			MouseEventNone = 0,
			MouseEventLUp,
			MouseEventRUp,
			MouseEventLDown,
			MouseEventRDown,
			MouseEventMove,
			MouseEventLDouble,
			MouseEventRDouble,
			MouseEventLeave,
			MouseEventMDown,
			MouseEventMUp,
		};

		enum Key {
			KeyNone = 0,
			KeyMouseLeft = 1,
			KeyMouseRight,
			KeyMouseMiddle,
			KeyControl,
			KeyShift,
			KeyUp,
			KeyDown,
			KeyLeft,
			KeyRight,
			KeyPageUp,
			KeyPageDown,
			KeyCharacter,
			KeyBrowseBack,
			KeyBrowseForward,
			KeyAlt,
		};

		class EXPORTED Wnd: public virtual Object {
			friend class FloatingPane;

			public:
				Wnd(const wchar_t* title, HWND parent=0, const wchar_t* className=TJ_DEFAULT_CLASS_NAME,  bool useDoubleBuffering=true, int exStyle=0L);
				virtual ~Wnd();
				
				virtual void Show(bool s);
				bool IsShown() const;
				void Repaint();
				virtual void Layout();
				virtual void Update();
				virtual LRESULT PreMessage(UINT msg, WPARAM wp, LPARAM lp);
				virtual void SetText(const wchar_t* t);
				void SetStyle(DWORD style);
				void SetStyleEx(DWORD style);
				void UnsetStyle(DWORD style);
				void UnsetStyleEx(DWORD style);
				
				virtual void SetFullScreen(bool f);
				virtual void SetFullScreen(bool f, int display);
				bool IsFullScreen();

				// Scrolling
				void SetHorizontallyScrollable(bool s);
				void SetVerticallyScrollable(bool s);
				int GetHorizontalPos();
				int GetVerticalPos();
				void SetVerticalPos(int p);
				void SetHorizontalPos(int p);
				void SetHorizontalScrollInfo(Range<int> rng, int pageSize);
				void SetVerticalScrollInfo(Range<int> rng, int pageSize);

				virtual void Move(Pixels x, Pixels y, Pixels w, Pixels h);

				virtual std::wstring GetText();
				virtual void SetText(std::wstring text);
				void SetSize(Pixels w, Pixels h);
				virtual Area GetClientArea() const;
				Area GetWindowArea();
				virtual void Fill(LayoutFlags flags, Area& rect, bool direct = true);
				void Fill();

				Wnd* GetParent();
				Wnd* GetRootWindow();

				void SetWantMouseLeave(bool t);
				bool GetWantMouseLeave() const;
				bool IsMouseOver();

				HWND GetWindow();
				virtual std::wstring GetTabTitle() const;		// return an empty string if you don't want to override Pane's title
				virtual ref<Icon> GetTabIcon() const;		// should return 0 when you don't want to override the tab icon set in Pane
				virtual void Focus();
				bool HasFocus(bool childrenToo = false) const;
				static bool IsKeyDown(Key k);
				virtual void BringToFront();

				/* Drag & Drop - Sleur & Pleur */
				void SetDropTarget(bool d);

				/* Settings API */
				void SetSettings(ref<Settings> st);
				ref<Settings> GetSettings();
				virtual void Add(ref<Wnd> child, bool visible = true);

			protected:
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);

				// Messages
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme) = 0;
				virtual void OnSize(const Area& newSize);
				virtual void OnScroll(ScrollDirection dir);
				virtual void OnActivate(bool activate);
				virtual void OnFocus(bool focus);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSettingsChanged();
				virtual void OnDropFiles(const std::vector<std::wstring>& files);
				virtual void OnTimer(unsigned int id);
				virtual void OnKey(Key k, wchar_t t, bool down, bool isAccelerator);
				virtual void OnContextMenu(Pixels x, Pixels y);

				// Timer
				virtual void StartTimer(Time interval, unsigned int id);
				virtual void StopTimer(unsigned int id);

				// Keys
				static void TranslateKeyCodes(int vk, Key& key, wchar_t& ch);
			
			private:
				static void RegisterClasses();
				Gdiplus::Bitmap* _buffer;
				bool _doubleBuffered;
				int _horizontalPos;
				int _verticalPos;
				int _horizontalPageSize;
				int _verticalPageSize;
				static bool _classesRegistered;
				long _oldStyle, _oldStyleEx;
				HWND _wnd;
				bool _fullScreen;
				bool _wantsMouseLeave;
				ref<Settings> _settings;
		};

		class EXPORTED TopWnd: public Wnd {
			public:
				TopWnd(const wchar_t* title, HWND parent=0, const wchar_t* className=TJ_DEFAULT_CLASS_NAME,  bool useDoubleBuffering=true, int exStyle=0L);
				virtual ~TopWnd();
				virtual void SetQuitOnClose(bool t);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void GetMinimumSize(Pixels& w, Pixels& h);

			protected:
				virtual void OnSize(const Area& ns);
				virtual void OnSettingsChanged();

			private:
				bool _quitOnClose;
		};
	}
}

#endif
