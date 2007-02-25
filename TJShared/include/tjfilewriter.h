#ifndef _TJFILEWRITER_H
#define _TJFILEWRITER_H

namespace tj {
	namespace shared {
		class EXPORTED FileWriter: public virtual Object {
			public:
				FileWriter(std::string root);
				virtual ~FileWriter();

				void Save(std::string filename);
				TiXmlElement* GetRoot();
				TiXmlDocument* GetDocument();

				void Add(Serializable* ser);
				void Add(ref<Serializable> ser);

			protected:
				TiXmlDocument _document;
				TiXmlElement _root;
		};

		class EXPORTED FileReader: public virtual Object {
			public:
				FileReader();
				virtual ~FileReader();
				void Read(std::string filename, ref<Serializable> model);
		};
	}
}

#endif