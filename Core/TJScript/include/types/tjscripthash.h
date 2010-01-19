#ifndef _TJSCRIPTHASH_H
#define _TJSCRIPTHASH_H

#pragma warning(push)
#pragma warning(disable: 4251 4275)

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

#pragma warning(pop)

#endif