#ifndef _TJSCRIPTARRAY_H
#define _TJSCRIPTARRAY_H

namespace tj {
	namespace script {

		class ScriptArrayType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptArrayType();
		};

		class SCRIPT_EXPORTED ScriptArray: public ScriptObject<ScriptArray> {
			friend class ScriptArrayIterator;

			public:
				ScriptArray();
				virtual ~ScriptArray();
				static void Initialize();
				void Push(ref<Scriptable> st);

				// Script functions
				virtual tj::shared::ref<Scriptable> SToString(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SPush(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SPop(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SGet(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SCount(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SImplode(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SValues(tj::shared::ref<ParameterList> p);

			protected:
				std::vector< tj::shared::ref<Scriptable> > _array;
				tj::shared::CriticalSection _lock;
		};
	}
}

#endif