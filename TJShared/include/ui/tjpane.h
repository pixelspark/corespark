#ifndef _TJPANE_H
#define _TJPANE_H

namespace tj {
	namespace shared {
		class RootWnd;

		struct EXPORTED Placement {
			enum Type { 
				Orphan = 0,
				Tab,
				Floating,
				_Last,
			};

			Type _type;
			std::wstring _container;

			Placement(Type t = Orphan, const std::wstring& container = L"");
			~Placement();
			static std::wstring TypeToString(Type t);
			static Type TypeFromString(const std::wstring& s);
		};

		class EXPORTED Pane: public virtual Object {
			friend class TabWnd;
			friend class RootWnd;
			friend class FloatingPane;

			public:
				Pane(const std::wstring& title, ref<Wnd> wnd, bool detached, bool closable, ref<Settings> st, const Placement& defaultPlacement = Placement(Placement::Orphan), std::wstring icon=L"");
				virtual ~Pane();
				ref<Wnd> GetWindow();
				const ref<Wnd> GetWindow() const;
				ref<Icon> GetIcon() const;
				std::wstring GetTitle() const;
				bool IsClosable() const;
				virtual void SetTitle(std::wstring c);
				bool HasIcon() const;

			protected:
				virtual void OnMove(int x, int y, int w, int h); // Called by FloatingPane
				virtual void OnPlacementChange(Placement& p);
				virtual Placement GetPreferredPlacement();
				virtual RECT GetPreferredPosition(); // For FloatingPane

				std::wstring _title;
				ref<Wnd> _wnd;
				ref<Settings> _settings;
				Placement _defaultPlacement;
				bool _detached;
				bool _fullscreen;
				bool _closable;
				ref<Icon> _icon;
		};

		class FloatingPane: public Wnd {
			friend class RootWnd;

			public:
				FloatingPane(RootWnd* rw, ref<Pane> pane);
				virtual ~FloatingPane();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void Layout();
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);

			protected:
				ref<Pane> _pane;
				RootWnd* _root;
				bool _dragging;
		};
	}
}

#endif