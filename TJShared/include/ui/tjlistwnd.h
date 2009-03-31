#ifndef _TJLISTWND_H
#define _TJLISTWND_H

namespace tj {
	namespace shared {
		class EXPORTED ColumnInfo {
			public:
				virtual ~ColumnInfo();
				virtual float GetColumnX(int id) = 0;
				virtual float GetColumnWidth(int id) = 0;
				virtual bool IsColumnVisible(int id) const = 0;
		};

		class EXPORTED GridWnd: public ChildWnd, public ColumnInfo {
			public:
				struct Column {
					Column(String title=L"");
					String _title;
					float _width;
					bool _visible;
				};
				
				GridWnd();
				virtual ~GridWnd();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual Area GetClientArea() const;

				// col stuff
				virtual void AddColumn(String name, int id, float w =-1.0f, bool visible=true);
				virtual void SetColumnWidth(int id, float w);
				virtual float GetColumnX(int id);
				virtual float GetColumnWidth(int id);
				virtual void SetColumnVisible(int id, bool v);
				virtual bool IsColumnVisible(int id) const;
				virtual void SetShowHeader(bool t);

				virtual void SetAllowColumnResizing(bool r);
				virtual bool IsColumnResizingAllowed() const;

				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnColumnSizeChanged();
				virtual void OnSettingsChanged();
				virtual void OnContextMenu(Pixels x, Pixels y);

			protected:
				virtual void DoContextMenu(Pixels x, Pixels y);
				virtual Pixels GetHeaderHeight() const;
				const static float KMinimumColumnWidth;

				std::map<int,Column> _cols;	
				int _draggingCol;
				int _dragStartX;
				bool _showHeader;
				bool _allowResize;
		};

		class EXPORTED ListWnd: public GridWnd {
			public:
				ListWnd();
				virtual ~ListWnd();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void SetSelectedRow(int r);
				virtual int GetSelectedRow() const;

				/* The 'empty text' is the text shown when the list is empty (could be a hint for the user
				on how to fill the list */
				virtual void SetEmptyText(const String& txt); 
				String GetEmptyText() const;

			protected:
				// to be implemented by child
				virtual int GetItemCount() = 0;
				virtual void PaintItem(int id, graphics::Graphics& g, Area& row, const ColumnInfo& ci) = 0;
				virtual Pixels GetItemHeight();
				virtual void OnClickItem(int id, int col, Pixels x, Pixels y);
				virtual void OnRightClickItem(int id, int col);
				virtual void OnDoubleClickItem(int id, int col);
				virtual void OnColumnSizeChanged();
				
				// Overridden from GridWnd
				virtual void OnSettingsChanged();
				virtual void OnFocus(bool f);
				virtual void OnSize(const Area& ns);
				virtual void OnKey(Key k, wchar_t t, bool down, bool isAccel);
				virtual void OnScroll(ScrollDirection dir);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnMouseWheelMove(WheelDirection wd);

				// other handy stuff
				void DrawCellText(graphics::Graphics& g, graphics::StringFormat* sf, graphics::SolidBrush* br, graphics::Font* font, int col, Area row, const String& str);
				void DrawCellDownArrow(graphics::Graphics& g, int col, const Area& row);
				void DrawCellIcon(graphics::Graphics& g, int col, Area row, Icon& icon);

				int GetRowIDByHeight(int h);
				virtual Area GetRowArea(int rid);

			private:
				String _emptyText;
				int _selected;
		};

		class EXPORTED EditableListWnd: public ListWnd {
			public:
				EditableListWnd();
				virtual ~EditableListWnd();
				virtual void SetSelectedRowAndEdit(int r);
				virtual void SetSelectedRow(int r);
				virtual void EndEditing();
				virtual void Layout();

				// To be implemented by child; returns 0 when item is not editable
				virtual ref<Property> GetPropertyForItem(int id, int col) = 0;
				virtual void OnEditingStarted(int row);
				virtual void OnEditingDone(int row);

			protected:
				virtual void OnColumnSizeChanged();
				bool IsEditing() const;
				int GetEditingRow() const;

			private:
				int _rowEditing;
				std::map< int, ref<Property> > _editorProperties;
		};
	}
}

#endif