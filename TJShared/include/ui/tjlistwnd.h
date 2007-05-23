#ifndef _TJLISTWND_H
#define _TJLISTWND_H

namespace tj {
	namespace shared {
		class EXPORTED ListWnd: public ChildWnd {
			public:
				struct Column {
					Column(std::wstring title=L"");
					std::wstring _title;
					float _width;
				};

				ListWnd(HWND parent);
				virtual ~ListWnd();
				virtual void Paint(Gdiplus::Graphics& g);
				virtual void OnSize(const Area& ns);
				virtual void OnScroll(ScrollDirection dir);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void SetSelectedRow(int r);
				virtual int GetSelectedRow() const;

				// col stuff
				virtual void AddColumn(std::wstring name, int id, float w =-1.0f);
				virtual void SetColumnWidth(int id, float w);
				virtual float GetColumnX(int id);
				virtual float GetColumnWidth(int id);

			protected:
				// implemented by child
				virtual int GetItemCount() = 0;
				virtual void PaintItem(int id, Gdiplus::Graphics& g, Area& row) = 0;
				virtual int GetItemHeightInPixels();
				virtual void OnClickItem(int id, int col);
				virtual void OnRightClickItem(int id, int col);
				virtual void OnDoubleClickItem(int id, int col);
				virtual void OnColumnSizeChanged();

				// other stuff
				void DrawCellText(Gdiplus::Graphics& g, Gdiplus::StringFormat* sf, Gdiplus::SolidBrush* br, Gdiplus::Font* font, int col, Area row, const std::wstring& str);
				void DrawCellIcon(Gdiplus::Graphics& g, int col, Area row, Icon& icon);

				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				int GetRowIDByHeight(int h);
				virtual Area GetRowArea(int rid);
				virtual int GetHeaderHeightInPixels() const;
				const static float KMinimumColumnWidth;
			
			private:
				std::map<int,Column> _cols;	
				int _draggingCol;
				int _dragStartX;
				int _selected;
		};
	}
}

#endif