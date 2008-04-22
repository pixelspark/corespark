#ifndef _TJANY_H
#define _TJANY_H

namespace tj {
	namespace shared {
		class Property;

		class EXPORTED Any: public Serializable {
			public:
				enum Type {
					TypeNull = 0,
					TypeInteger = 1,
					TypeDouble = 2,
					TypeBool = 5,
					TypeString = 8,
					TypeObject = 16,
				};

				Any();
				Any(double d);
				Any(bool b);
				Any(int i);
				Any(const std::wstring& s);
				Any(ref<Object> object);
				Any(Type t, const std::wstring& s);
				Any(Type t);
				virtual ~Any();

				Any operator+(const Any& o) const;
				Any operator-(const Any& o) const;
				Any operator*(const Any& o) const;
				Any operator/(const Any& o) const;
				bool operator>(const Any& o) const;
				bool operator<(const Any& o) const;
				bool operator==(const Any& o) const;
				bool operator!=(const Any& o) const;
				bool operator||(const Any& o) const;
				bool operator&&(const Any& o) const;
				Any operator%(const Any& o) const;
				Any operator-() const;
				Any operator!() const;
				Any Abs() const;

				std::wstring ToString() const;
				Type GetType() const;

				virtual void Save(TiXmlElement* you);
				virtual void Load(TiXmlElement* you);

				static ref<Property> CreateTypeProperty(const std::wstring& name, Type* type);
				static std::wstring GetTypeName(Type t);

			private:
				union {
					double _doubleValue;
					bool _boolValue;
					int _intValue;
				};
				std::wstring _stringValue;
				ref<Object> _object;
				Type _type;
		};
	}
}

#endif