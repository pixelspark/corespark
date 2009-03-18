#ifndef _TJVECTOR_H
#define _TJVECTOR_H

namespace tj {
	namespace shared {
		class EXPORTED Vector: public Serializable {
			public:
				Vector(float fx, float fy, float fz);
				Vector(const Vector& other);
				~Vector();
				Vector Dot(const Vector& other) const;
				Vector Scale(float n) const;
				float GetLength() const;
				String ToString() const;

				virtual void Save(TiXmlElement* parent);
				virtual void Load(TiXmlElement* you);

				inline Vector operator+(const Vector& o) {
					return Vector(x+o.x, y+o.y, z+o.z);
				}

			public:
				float x,y,z;
		};
	}
}

#endif