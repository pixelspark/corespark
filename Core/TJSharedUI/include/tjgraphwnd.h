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
 
 #ifndef _TJGRAPHWND_H
#define _TJGRAPHWND_H

namespace tj {
	namespace shared {
		class GraphItem;
		struct EXPORTED GraphArrow {
			GraphArrow();

			enum Direction {
				None = 0,
				In,
				Out,
				OutLight,
			};

			Direction _direction;
			weak<GraphItem> _to;
			String _text;
		};

		class EXPORTED GraphItem: public virtual Object {
			friend class GraphWnd;

			public:
				GraphItem();
				virtual ~GraphItem();
				bool IsSelected() const;
				virtual void SetSelected(bool t);
				virtual void Hide(bool h);
				bool IsHidden() const;
				virtual String GetText() = 0;

				virtual Area& GetArea();
				virtual void SetPosition(Pixels x, Pixels y);
				virtual void SetSize(Pixels x, Pixels y);
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme) = 0;
				virtual void AddArrow(ref<GraphItem> to, GraphArrow::Direction dir, const String& text);
				virtual void OnMouse(MouseEvent e, Pixels x, Pixels y, ref<GraphWnd> gw);
				virtual void OnContextMenu(Pixels x, Pixels y, ref<GraphWnd> gw);
				virtual void OnKey(Key k, wchar_t key, bool down, bool isAccelerator, ref<GraphWnd> gw);

			protected:
				Area _area;

			private:
				bool _selected;
				bool _hidden;
				std::vector<GraphArrow> _arrows;

		};

		class EXPORTED SimpleGraphItem: public GraphItem {
			public:
				SimpleGraphItem();
				virtual ~SimpleGraphItem();
				virtual void SetText(const String& t);
				virtual void SetColor(const graphics::Color& c);
				virtual String GetText() const;
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);

			protected:
				String _text;
				graphics::Color _color;
		};

		class EXPORTED GraphWnd: public ChildWnd {
			public:
				GraphWnd();
				virtual ~GraphWnd();
				virtual void OnSize(const Area& ns);
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void AddItem(ref<GraphItem> item);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual ref<GraphItem> GetItemAt(Pixels x, Pixels y);
				virtual std::pair<Pixels, Pixels> GetEdge(const Area& a, const Area& b);
				virtual void Clear();
				virtual void HideAll();
				virtual std::vector< ref<GraphItem> >& GetItems();
				virtual void Update();

			protected:
				virtual void OnFocus(bool focus);
				virtual void OnContextMenu(Pixels x, Pixels y);
				virtual void OnKey(Key k, wchar_t key, bool down, bool isAccelerator);

				std::vector< ref<GraphItem> > _items;
				ref<GraphItem> _dragging;
				ref<GraphItem> _focus;
				Pixels _dragStartX, _dragStartY;
				Pixels _dragBeginX, _dragBeginY;
		};

	}
}

#endif