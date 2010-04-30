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
 
 #ifndef _TJPROPERTY_H
#define _TJPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED Property: public virtual Object {
			public:
				Property(const String& name, bool expandable=false);
				virtual ~Property();

				virtual String GetName();
				virtual Pixels GetHeight();
				virtual const String& GetHint() const;
				virtual void SetHint(const String& h);
				virtual bool IsExpandable() const;
				virtual void SetExpanded(bool t);
				
				virtual bool IsExpanded();
				virtual ref<Wnd> GetWindow() = 0;
				virtual void Update() = 0;
				
			protected:
				virtual void SetExpandable(bool t);

				bool _expandable;
				bool _expanded;
				String _name;
				String _hint;
		};

		class EXPORTED PropertySet: public virtual Object {
			friend class PropertyGridWnd;

			public:
				PropertySet();
				virtual ~PropertySet();
				virtual void Add(ref<Property> p);
				virtual void MergeAdd(ref<PropertySet> other);

			protected:
				std::vector< ref<Property> > _properties;
		};

		class EXPORTED Inspectable: public virtual Object {
			public:
				virtual ~Inspectable();
				virtual ref<PropertySet> GetProperties();
				virtual void OnPropertyChanged(void* member);
		};
	}
}

#endif