#ifndef _TJPACKAGE_H
#define _TJPACKAGE_H

namespace tj {
	namespace zip {
		class TJZIP_EXPORTED Package: public virtual tj::shared::Object {
			public:
				Package(const std::wstring& file, const char* passsword=0);
				virtual ~Package();
				virtual void Add(const std::wstring& filename, const std::wstring& realfile);
				virtual void AddData(const std::wstring& filename, const std::wstring& data);
			protected:
				HZIP _zip;
		};
	
	}
}

#endif