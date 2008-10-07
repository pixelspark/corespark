#ifndef _TJFILEWRITER_H
#define _TJFILEWRITER_H

namespace tj {
	namespace shared {
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