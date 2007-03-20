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
				virtual void AddColumn(std::wstring name, int id);
				virtual void SetColumnWidth(int id, float w);
				virtual float GetColumnX(int id);
				virtual float GetColumnWidth(int id);

				// implement
				virtual int GetItemCount() = 0;
				virtual int GetItemHeight();
				virtual void PaintItem(int id, Gdiplus::Graphics& g, Area& row) = 0;
				virtual void OnClickItem(int id, int col);
				virtual void OnRightClickItem(int id, int col);
				virtual void OnDoubleClickItem(int id, int col);

			protected:
				int GetRowIDByHeight(int h);
				std::map<int,Column> _cols;
				const static int KColumnHeaderHeight = 24;
				const static float KMinimumColumnWidth;
				int _draggingCol;
				int _dragStartX;
				int _selected;
		};
	}
}

#endif