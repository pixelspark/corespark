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
 
 #ifndef _TJPROPERTYGRID_H
#define _TJPROPERTYGRID_H

namespace tj {
	namespace shared {
		class EXPORTED PropertyGridWnd: public ChildWnd {
			public:
				PropertyGridWnd(bool withPath=true);
				virtual ~PropertyGridWnd();
				void Update();
				void Inspect(ref<Inspectable> isp, ref<Path> p = ref<Path>());
				void Layout();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				void Clear();
				void SetNameWidth(Pixels w);
				virtual void OnScroll(ScrollDirection dir);
				virtual void OnSize(const Area& ar);
				void SetShowHints(bool h);
				bool GetShowHints() const;
				void FocusFirstProperty();

			protected:
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSettingsChanged();
				Pixels GetPathHeight() const;

				std::vector< ref<Property> > _properties;
				ref<TooltipWnd> _tw;
				ref<Inspectable> _subject;
				ref<PathWnd> _path;
				bool _isDraggingSplitter;
				bool _showHints;
				Pixels _nameWidth;
				Icon _expandIcon, _collapseIcon;
				MouseCapture _capture;

				const static Pixels KPathHeight;
				const static Pixels KMinimumNameColumnWidth;
				const static Pixels KPropertyMargin;
		};
	}
}

#endif