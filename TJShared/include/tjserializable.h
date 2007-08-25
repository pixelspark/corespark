#ifndef _TJSERIALIZABLE_H
#define _TJSERIALIZABLE_H

namespace tj {
	namespace shared {
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

		template<typename T> inline void SaveAttributeSmall(TiXmlElement* parent, const char* name, const T& val) {
			std::string cs = StringifyMbs(val);
			parent->SetAttribute(name, cs.c_str());
		}

		template<typename T> inline T LoadAttributeSmall(TiXmlElement* you, const char* name, const T& defaultValue) {
			const char* value = you->Attribute(name);
			if(value!=0) {
				return StringTo<T>(value, defaultValue);
			}
			return LoadAttribute<T>(you, name, defaultValue);
		}
	}
}


#endif