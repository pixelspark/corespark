#ifndef _TJPROPERTY_H
#define _TJPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED Property: public virtual Object {
			public:
				Property(const std::wstring& name, bool expandable=false);
				virtual ~Property();

				virtual std::wstring GetName();
				virtual Pixels GetHeight();
				virtual const std::wstring& GetHint() const;
				virtual void SetHint(const std::wstring& h);
				virtual bool IsExpandable() const;
				virtual void SetExpanded(bool t);
				
				virtual bool IsExpanded();
				virtual ref<Wnd> GetWindow() = 0;
				virtual void Update() = 0;
				
			protected:
				virtual void SetExpandable(bool t);

				bool _expandable;
				bool _expanded;
				std::wstring _name;
				std::wstring _hint;
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

		class EXPORTED Inspectable {
			public:
				virtual ~Inspectable();
				virtual ref<PropertySet> GetProperties()=0;
		};
	}
}

#endif