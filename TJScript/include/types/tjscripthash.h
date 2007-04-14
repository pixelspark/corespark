#ifndef _TJSCRIPTHASH_H
#define _TJSCRIPTHASH_H

namespace tj {
	namespace script {

		class ScriptHashType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptHashType();
		};

		class SCRIPT_EXPORTED ScriptHash: public ScriptObject<ScriptHash> {
			public:	
				ScriptHash(std::wstring x);
				ScriptHash(int h);
				virtual ~ScriptHash();
				int GetHash() const;

				static void Initialize();
			protected:
				virtual ref<Scriptable> ToString(ref<ParameterList> p);
				virtual ref<Scriptable> ToInt(ref<ParameterList> p);
				int _hash;
		};
	}
}

#endif