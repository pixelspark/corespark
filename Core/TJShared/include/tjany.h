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
 
 #ifndef _TJANY_H
#define _TJANY_H

#include "tjsharedinternal.h"		
#include "tjserializable.h"

namespace tj {
	namespace shared {
		class Property;
		class Tuple;
		class Inspectable;

		class EXPORTED Any: public Serializable {
			public:
				enum Type {
					TypeNull = 0,
					TypeInteger = 1,
					TypeDouble = 2,
					TypeBool = 5,
					TypeString = 8,
					TypeObject = 16,
					TypeTuple = 32,
				};

				Any();
				Any(double d);
				Any(bool b);
				Any(int i);
				Any(const String& s);
				Any(strong<Tuple> tuple);
				Any(ref<Object> object);
				Any(Type t, const String& s);
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

				operator bool() const;
				operator double() const;
				operator int() const;
				operator float() const;
				operator String() const;

				Any operator%(const Any& o) const;
				Any operator-() const;
				Any operator!() const;
				Any Abs() const;
				Any Force(Type t) const;

				String ToString() const;
				Type GetType() const;
				ref<Object> GetContainedObject();

				virtual void Save(TiXmlElement* you);
				virtual void Load(TiXmlElement* you);

				///static ref<Property> CreateTypeProperty(const String& name, ref<Inspectable> holder, Type* type);
				static String GetTypeName(Type t);
				static Type TypeFromString(const String& tn);
				static String StringFromType(Type t);

			private:
				union {
					double _doubleValue;
					bool _boolValue;
					int _intValue;
				};
				String _stringValue;
				ref<Object> _object;
				Type _type;
		};

		class EXPORTED Tuple: public virtual Object {
			public:
				Tuple(unsigned int size);
				virtual ~Tuple();
				Any& Get(unsigned int i);
				const Any& Get(unsigned int i) const;
				unsigned int GetLength() const;
				void Set(unsigned int i, const Any& a);
				String ToString() const;

			protected:
				Any* _data;
				unsigned int _length;
		};
	}
}

#endif