#ifndef _TJHASH_H
#define _TJHASH_H

class EXPORTED Hash: public virtual Object {
	public:
		Hash();
		virtual ~Hash();
		int Calculate(std::wstring data);
};

#endif