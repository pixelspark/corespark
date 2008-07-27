#ifndef _TJFILEWRITER_H
#define _TJFILEWRITER_H

namespace tj {
	namespace shared {
		class EXPORTED FileWriter: public virtual Object {
			public:
				FileWriter(const std::string& rootElementName);
				virtual ~FileWriter();

				void Save(const std::string& filename);
				ref<TiXmlElement> GetRoot();
				ref<TiXmlDocument> GetDocument();

				void Add(Serializable* ser);
				void Add(ref<Serializable> ser);

			protected:
				ref<TiXmlDocument> _document;
				ref<TiXmlElement> _root;
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