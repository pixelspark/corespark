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