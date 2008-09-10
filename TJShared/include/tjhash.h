#ifndef _TJHASH_H
#define _TJHASH_H

namespace tj {
	namespace shared {
		class EXPORTED Hash: public virtual Object {
			public:
				Hash();
				virtual ~Hash();
				int Calculate(const std::wstring& data);
		};

		class EXPORTED Hasheable {
			public:
				virtual ~Hasheable();
				virtual int GetHash() const = 0;
		};
	}
}

#endif