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

		// This is a MD5 hash
		class EXPORTED SecureHash: public virtual Object {
			public:
				SecureHash();
				virtual ~SecureHash();
				virtual void AddData(const void* data, size_t length);
				virtual std::string GetHashAsString();
				virtual void AddString(const wchar_t* data);
				virtual void AddFile(const std::wstring& path);

			protected:
				void* _data;
		};
	}
}

#endif