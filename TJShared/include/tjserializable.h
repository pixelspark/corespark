#ifndef _TJSERIALIZABLE_H
#define _TJSERIALIZABLE_H

class TiXmlElement;

namespace tj {
	namespace shared {
		class Property;
		template<typename T> class GenericListProperty;

		template<typename T> class Enumeration {
			public:
				Enumeration() {
					InitializeMapping();
				}

				virtual ~Enumeration() {
				}

				std::wstring Serialize(const T& et) {
					std::map<T, ValueInfo>::const_iterator it = _mapping.find(et);
					while(it!=_mapping.end()) {
						return it->second._serializedForm;
					}
					return L"";
				}

				T Unserialize(const std::wstring& ident) {
					std::map<T, ValueInfo>::const_iterator it = _mapping.begin();
					while(it!=_mapping.end()) {
						if(it->second._serializedForm==ident) {
							return it->first;
						}
						++it;
					}
					Throw(L"Could not unserialize enumeration value", ExceptionTypeError);
				}
				
				T Unserialize(const std::wstring& ident, const T& defaultValue) {
					std::map<T, ValueInfo>::const_iterator it = _mapping.begin();
					while(it!=_mapping.end()) {
						if(it->second._serializedForm==ident) {
							return it->first;
						}
						++it;
					}
					return defaultValue;
				}

				std::wstring GetFriendlyName(const T& value) {
					std::map<T, ValueInfo>::const_iterator it = _mapping.find(value);
					if(it!=_mapping.end()) {
						return Language::Get(it->second._friendlyForm);
					}
					return L"";
				}

				void Add(const T& value, const std::wstring& serialized, const std::wstring& friendly) {
					ValueInfo vi;
					vi._serializedForm = serialized;
					vi._friendlyForm = friendly;
					_mapping[value] = vi;
				}

				ref<Property> CreateSelectionProperty(const std::wstring& title, T* value) {
					ref< GenericListProperty<T> > gp = GC::Hold(new GenericListProperty<T>(title, value, 0, *value));
					std::map<T, ValueInfo>::const_iterator it = _mapping.begin();
					while(it!=_mapping.end()) {
						const ValueInfo& vi = it->second;
						gp->AddOption(Language::Get(vi._friendlyForm), it->first);
						++it;
					}
					return gp;
				}

				static Enumeration<T> Instance;

			private:
				struct ValueInfo {
					std::wstring _serializedForm;
					std::wstring _friendlyForm;
				};

				std::map<T, ValueInfo> _mapping;

				// This should be implemented for each enumeration
				void InitializeMapping();
		};

		class EXPORTED Serializable {
			public:
				virtual ~Serializable();
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

		class EXPORTED XML {
			public:
				static void GetElementHash(const TiXmlNode* node, SecureHash& sh);
		};

		class EXPORTED FileWriter: public virtual Object {
			public:
				FileWriter(const std::string& rootElementName);
				virtual ~FileWriter();

				void Save(const std::string& filename);
				strong<TiXmlElement> GetRoot();
				strong<TiXmlDocument> GetDocument();

				void Add(Serializable* ser);
				void Add(ref<Serializable> ser);

			protected:
				strong<TiXmlDocument> _document;
				strong<TiXmlElement> _root;
		};

		class EXPORTED FileReader: public virtual Object {
			public:
				FileReader();
				virtual ~FileReader();
				void Read(const std::string& filename, ref<Serializable> model);
		};
	}
}


#endif