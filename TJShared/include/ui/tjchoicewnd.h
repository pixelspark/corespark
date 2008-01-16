#ifndef _TJCHOICEWND_H
#define _TJCHOICEWND_H

namespace tj {
	namespace shared {
		class EXPORTED Choice {
			public:
				Choice(const std::wstring& title, const std::wstring& text, const std::wstring& image);
				virtual ~Choice();
				const std::wstring& GetText() const;
				const std::wstring& GetTitle() const;
				Icon& GetImage();
				Pixels GetWidth() const;
				Pixels GetHeight() const;
				void SetSize(Pixels w, Pixels h);

			protected:
				std::wstring _title, _text;
				Icon _image;
				Pixels _w, _h;
		};

		class EXPORTED ChoiceListWnd: public ListWnd {
			public:
				ChoiceListWnd();
				virtual ~ChoiceListWnd();
				virtual int GetItemCount();
				virtual void PaintItem(int id, Gdiplus::Graphics& g, Area& row);
				virtual int GetItemHeight();
				virtual void AddChoice(ref<Choice> choice);
				virtual void SetItemHeight(Pixels h);
				virtual void OnClickItem(int id, int col, Pixels x, Pixels y);

			protected:
				enum {
					KColTricks = 1,
				};
				std::vector< ref<Choice> > _choices;
				Pixels _itemHeight;
		};

		class EXPORTED ChoiceWnd: public PopupWnd {
			public:
				ChoiceWnd();
				virtual ~ChoiceWnd();
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual void Layout();
				virtual void OnSize(const Area& ns);
				virtual void AddChoice(ref<Choice> trick);
				virtual void SetTitle(const std::wstring& title);
				virtual bool HasHeader() const;

			protected:
				std::wstring _title;
				ref<ChoiceListWnd> _choices;
		};
	}
}

#endif
