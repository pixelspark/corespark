#ifndef _TJSERIALIZABLE_H
#define _TJSERIALIZABLE_H

class Serializable {
	public:
		virtual void Save(TiXmlElement* parent) = 0;
		virtual void Load(TiXmlElement* you) = 0;
};

template<typename T> inline void SaveAttribute(TiXmlElement* parent, const char* name, const T& val) {
	std::string cs = StringifyMbs(val);
	TiXmlElement element(name);
	element.InsertEndChild(TiXmlText(cs.c_str()));
	parent->InsertEndChild(element);
}

template<typename T> inline T LoadAttribute(TiXmlElement* you, const char* name, const T& defaultValue) {
	TiXmlElement* elem = you->FirstChildElement(name);
	if(elem==0) {
		return defaultValue;
	}

	TiXmlNode* nd = elem->FirstChild();
	if(nd==0) return defaultValue;
	return StringTo<T>(nd->Value(), defaultValue);
}


#endif