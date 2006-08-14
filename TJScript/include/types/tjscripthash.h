#ifndef _TJSCRIPTHASH_H
#define _TJSCRIPTHASH_H

namespace tj {
	namespace script {

		class ScriptHashType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptHashType();
		};

		class SCRIPT_EXPORTED ScriptHash: public virtual tj::shared::Object, public Scriptable {
			public:	
				ScriptHash(std::wstring x);
				ScriptHash(int h);
				virtual ~ScriptHash();
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p);
				int GetHash() const;
			protected:
				int _hash;
		};
	}
}

#endif