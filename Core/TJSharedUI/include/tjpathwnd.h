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
 
 #ifndef _TJPATH_H
#define _TJPATH_H

namespace tj {
	namespace shared {
		class PropertyGridWnd;

		class EXPORTED Crumb: public virtual Object {
			friend class PathWnd;

			public:
				Crumb(const String& text, const ResourceIdentifier& icon);
				virtual ~Crumb();
				String GetText() const;
				String GetTextTrimmed() const;
				graphics::Image* GetIcon();
				virtual void GetChildren(std::vector< ref<Crumb> >& crs);
				virtual ref<Inspectable> GetSubject();
				virtual void SetText(const String& x);

			protected:
				String _text;
				Icon _icon;
				static const int KMaxTextLength = 15;
		};

		class EXPORTED BasicCrumb: public Crumb {
			public:
				BasicCrumb(const String& text, const ResourceIdentifier& icon, ref<Inspectable> subject);
				virtual ~BasicCrumb();
				virtual ref<Inspectable> GetSubject();
				virtual void GetChildren(std::vector< ref<Crumb> >& crs);

			protected:
				ref<Inspectable> _subject;
		};

		class EXPORTED Path: public virtual Object {
			friend class PathWnd;

			public:
				Path();
				virtual ~Path();
				virtual void Add(String text, String icon, ref<Inspectable> subject);
				virtual void Add(ref<Crumb> r);
				virtual ref<Crumb> GetHead();

			protected:
				std::vector< ref<Crumb> > _crumbs;
		};

		class PathWnd: public ChildWnd {
			public:
				PathWnd(PropertyGridWnd* grid);
				virtual ~PathWnd();
				virtual void Update();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void SetPath(ref<Path> p);
				virtual ref<Crumb> GetCrumbAt(Pixels x, int* left=0);

			protected:
				virtual void OnSize(const Area& ns);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void DoCrumbMenu(ref<Crumb> r, int x);

				const static int KMarginLeft = 3;
				const static int KMarginRight = 12;
				const static int KIconWidth = 16;

				ref<Path> _path;
				PropertyGridWnd* _propertyGrid;
				Icon _separator;
				ref<Crumb> _over;
		};
	}
}

#endif