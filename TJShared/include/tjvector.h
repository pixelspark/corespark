#ifndef _TJVECTOR_H
#define _TJVECTOR_H

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

class VectorPropertyWnd;

class EXPORTED VectorProperty: public Property {
	public:
		VectorProperty(std::wstring name, Vector* v);
		virtual ~VectorProperty();
		virtual std::wstring GetValue();

		virtual HWND GetWindow();
		virtual HWND Create(HWND parent);
		virtual void Changed();
		virtual void Update();

	protected:
		Vector* _vec;
		VectorPropertyWnd* _wnd;
};

#endif