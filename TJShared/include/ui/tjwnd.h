#ifndef _TJWND_H
#define _TJWND_H

namespace tj {
	namespace shared {
		enum ScrollDirection {
			ScrollDirectionNone = 0,
			ScrollDirectionHorizontal = 1,
			ScrollDirectionVertical,
		};

		enum WheelDirection {
			WheelDirectionNone = 0,
			WheelDirectionUp,
			WheelDirectionDown,
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
			KeyDelete,
			KeyInsert,
			KeyHome,
			KeyEnd,
			KeyBackspace,
		};

		enum LayoutFlags {
			LayoutHide = 0,
			LayoutTop,
			LayoutBottom,
			LayoutLeft,
			LayoutRight,
			LayoutFill,
		};
		
		class Element;

		class EXPORTED Element: public virtual Object {
			public:
				virtual ~Element();
				virtual Area GetClientArea() const;
				virtual void Fill(LayoutFlags flags, Area& rect, bool direct = true);
				virtual void SetSize(Pixels w, Pixels h);
				virtual void Move(Pixels x, Pixels y, Pixels w, Pixels h);
				virtual void Show(bool t);
				virtual bool IsShown() const;
				virtual void Update();

				virtual void Paint(graphics::Graphics& g, strong<Theme> theme) = 0;

				struct EXPORTED ShowNotification {
					ShowNotification(bool shown);
					bool _shown;
				};

				struct EXPORTED SizeNotification {};
				struct EXPORTED UpdateNotification {};

				Listenable<ShowNotification> OnShow;
				Listenable<SizeNotification> OnSize;
				Listenable<UpdateNotification> OnUpdate;

			protected:
				Element();

			private:
				Area _client;
				bool _shown;
		};
		
		class EXPORTED Elements {
		public:
			template<class T> static ref<T> GetElementAt(std::vector< ref<T> >& elements, Pixels x, Pixels y) {
				typename std::vector< ref<T> >::iterator it = elements.begin();
				typename std::vector< ref<T> >::iterator end = elements.end();
				
				while(it!=end) {
					ref<T> element = *it;
					if(element) {
						//if(element.IsCastableTo<class Element>()) {
							ref<Element> elementCasted = element;
							if(elementCasted && elementCasted->IsShown()) {
								Area client = elementCasted->GetClientArea();
								if(client.IsInside(x,y)) {
									return element;
								}
							}
						//}
					}
					++it;
				}
				
				return null;
			}
		};

		class EXPORTED Wnd: public virtual Object {
			friend class FloatingPane;

			public:
				Wnd(ref<Wnd> parent = null, bool useDoubleBuffering = true, bool hasDropShadow = false);
				virtual ~Wnd();
				
				// Painting the window
				virtual void Show(bool s);
				bool IsShown() const;
				void Repaint();
				virtual void Update();

				// Scrolling
				void SetHorizontallyScrollable(bool s);
				void SetVerticallyScrollable(bool s);
				int GetHorizontalPos();
				int GetVerticalPos();
				void SetVerticalPos(int p);
				void SetHorizontalPos(int p);
				void SetHorizontalScrollInfo(Range<int> rng, int pageSize);
				void SetVerticalScrollInfo(Range<int> rng, int pageSize);

				// Window title or text
				virtual String GetText();
				virtual void SetText(const String& text);
				virtual void SetText(const wchar_t* t);
				virtual String GetTabTitle() const;		// return an empty string if you don't want to override Pane's title
				virtual ref<Icon> GetTabIcon() const;	// should return 0 when you don't want to override the tab icon set in Pane

				// Layout
				virtual void Layout();
				void SetSize(Pixels w, Pixels h);
				virtual Area GetClientArea() const;
				virtual Area GetWindowArea();
				virtual void Fill(LayoutFlags flags, Area& rect, bool direct = true);
				virtual void Fill();
				virtual void Move(Pixels x, Pixels y, Pixels w, Pixels h);
				Wnd* GetParent();
				Wnd* GetRootWindow();
				virtual void Focus();
				virtual bool HasFocus(bool childrenToo = false) const;

				// Input handling
				void SetWantMouseLeave(bool t);
				bool GetWantMouseLeave() const;
				bool IsMouseOver();
				static bool IsKeyDown(Key k);
				virtual void BringToFront();
				void SetDropTarget(bool d);

				// Settings functions
				void SetSettings(ref<Settings> st);
				ref<Settings> GetSettings();
				virtual void Add(ref<Wnd> child, bool visible = true);
			
				// Platform stuff
				#ifdef TJ_OS_WIN
					virtual LRESULT PreMessage(UINT msg, WPARAM wp, LPARAM lp);
					void SetStyle(DWORD style);
					void SetStyleEx(DWORD style);
					void UnsetStyle(DWORD style);
					void UnsetStyleEx(DWORD style);
					HWND GetWindow();
				#endif

			protected:
				#ifdef TJ_OS_WIN
					virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
					static void TranslateKeyCodes(int vk, Key& key, wchar_t& ch);
				#endif
			
				// Messages
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme) = 0;
				virtual void OnSize(const Area& newSize);
				virtual void OnMove(const Area& newArea);
				virtual void OnScroll(ScrollDirection dir);
				virtual void OnMouseWheelMove(WheelDirection dir);
				virtual void OnActivate(bool activate);
				virtual void OnFocus(bool focus);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSettingsChanged();
				virtual void OnDropFiles(const std::vector<String>& files);
				virtual void OnTimer(unsigned int id);
				virtual void OnKey(Key k, wchar_t t, bool down, bool isAccelerator);
				virtual void OnCharacter(wchar_t t);
				virtual void OnContextMenu(Pixels x, Pixels y);
				virtual void OnCopy();
				virtual void OnPaste();
				virtual void OnCut();

				// Timer
				virtual void StartTimer(Time interval, unsigned int id);
				virtual void StopTimer(unsigned int id);
			
			private:
				static void RegisterClasses();

				#ifdef TJ_OS_WIN
					HWND _wnd;
				#endif
			
				graphics::Bitmap* _buffer;
				bool _doubleBuffered;
				bool _wantsMouseLeave;
				int _dirty;
				int _horizontalPos;
				int _verticalPos;
				int _horizontalPageSize;
				int _verticalPageSize;
				ref<Settings> _settings;

				static bool _classesRegistered;
		};

		class EXPORTED TopWnd: public Wnd {
			public:
				TopWnd(const String& title, ref<Wnd> parent = null,  bool useDoubleBuffering = true, bool hasDropShadow = false);
				virtual ~TopWnd();
				virtual void SetQuitOnClose(bool t);
			
				#ifdef TJ_OS_WIN
					virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				#endif
			
				virtual void GetMinimumSize(Pixels& w, Pixels& h);

				// Fullscreen support
				virtual void SetFullScreen(bool f);
				virtual void SetFullScreen(bool f, int display);
				bool IsFullScreen();

			protected:
				virtual void OnSize(const Area& ns);
				virtual void OnSettingsChanged();

			private:
				bool _quitOnClose;
				bool _fullScreen;
				long _oldStyle, _oldStyleEx;
		};
	}
}

#endif
