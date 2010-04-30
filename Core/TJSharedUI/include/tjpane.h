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
			String _container;

			Placement(Type t = Orphan, const String& container = L"");
			~Placement();
			static String TypeToString(Type t);
			static Type TypeFromString(const String& s);
		};

		class EXPORTED Pane: public virtual Object {
			friend class TabWnd;
			friend class RootWnd;
			friend class FloatingPane;

			public:
				Pane(const String& title, ref<Wnd> wnd, bool detached, bool closable, ref<Settings> st, const Placement& defaultPlacement = Placement(Placement::Orphan), String icon=L"");
				virtual ~Pane();
				ref<Wnd> GetWindow();
				const ref<Wnd> GetWindow() const;
				ref<Icon> GetIcon() const;
				String GetTitle() const;
				bool IsClosable() const;
				virtual void SetTitle(String c);
				bool HasIcon() const;

			protected:
				virtual void OnMove(const Area& rc); // Called by FloatingPane
				virtual void OnPlacementChange(Placement& p);
				virtual Placement GetPreferredPlacement();
				virtual Area GetPreferredPosition(); // For FloatingPane

				String _title;
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
				virtual void Layout();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
		
			protected:
				#ifdef TJ_OS_WIN
					virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				#endif
			
				virtual void OnMove(const Area& ns);
				virtual void OnSize(const Area& ns);
				
				MouseCapture _capture;
				ref<Pane> _pane;
				RootWnd* _root;
				bool _dragging;
		};
	}
}

#endif