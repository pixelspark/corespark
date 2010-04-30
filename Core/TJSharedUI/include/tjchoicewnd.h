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
 
 #ifndef _TJCHOICEWND_H
#define _TJCHOICEWND_H

namespace tj {
	namespace shared {
		class EXPORTED Choice {
			public:
				Choice(const String& title, const String& text, const String& image);
				virtual ~Choice();
				const String& GetText() const;
				const String& GetTitle() const;
				Icon& GetImage();
				Pixels GetWidth() const;
				Pixels GetHeight() const;
				void SetSize(Pixels w, Pixels h);

			protected:
				String _title, _text;
				Icon _image;
				Pixels _w, _h;
		};

		class EXPORTED ChoiceListWnd: public ListWnd {
			public:
				ChoiceListWnd();
				virtual ~ChoiceListWnd();
				virtual int GetItemCount();
				virtual void PaintItem(int id, graphics::Graphics& g, Area& row, const ColumnInfo& ci);
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
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void Layout();
				virtual void OnSize(const Area& ns);
				virtual void AddChoice(ref<Choice> trick);
				virtual void SetTitle(const String& title);
				virtual bool HasHeader() const;

			protected:
				String _title;
				ref<ChoiceListWnd> _choices;
		};
	}
}

#endif
