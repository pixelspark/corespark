#ifndef _TJGRAPHWND_H
#define _TJGRAPHWND_H

namespace tj {
	namespace shared {
	class GraphItem;
		struct EXPORTED GraphArrow {
			enum Direction {
				None = 0,
				In,
				Out,
				OutLight,
			};

			Direction _direction;
			weak<GraphItem> _to;
			std::wstring _text;
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
				virtual std::wstring GetText() = 0;

				virtual Area& GetArea();
				virtual void SetPosition(Pixels x, Pixels y);
				virtual void SetSize(Pixels x, Pixels y);
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme) = 0;
				virtual void AddArrow(ref<GraphItem> to, GraphArrow::Direction dir, const std::wstring& text);
				virtual void OnMouse(MouseEvent e, Pixels x, Pixels y, ref<GraphWnd> gw);

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
				virtual void SetText(const std::wstring& t);
				virtual void SetColor(const Gdiplus::Color& c);
				virtual std::wstring GetText() const;
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);

			protected:
				std::wstring _text;
				Gdiplus::Color _color;
		};

		class EXPORTED GraphWnd: public ChildWnd {
			public:
				GraphWnd();
				virtual ~GraphWnd();
				virtual void OnSize(const Area& ns);
				virtual void Paint(Gdiplus::Graphics& g);
				virtual void AddItem(ref<GraphItem> item);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual ref<GraphItem> GetItemAt(Pixels x, Pixels y);
				virtual std::pair<Pixels, Pixels> GetEdge(const Area& a, const Area& b);
				virtual void Clear();
				virtual void HideAll();
				virtual void SaveImage(const std::wstring& path);
				virtual std::vector< ref<GraphItem> >& GetItems();
				virtual void Update();

			protected:
				std::vector< ref<GraphItem> > _items;
				ref<GraphItem> _dragging;
				Pixels _dragStartX, _dragStartY;
				Pixels _dragBeginX, _dragBeginY;
		};

	}
}

#endif