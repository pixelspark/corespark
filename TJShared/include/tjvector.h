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
				float GetLength() const;
				std::wstring ToString() const;

				virtual void Save(TiXmlElement* parent);
				virtual void Load(TiXmlElement* you);

			public:
				float x,y,z;
		};
	}
}

#endif