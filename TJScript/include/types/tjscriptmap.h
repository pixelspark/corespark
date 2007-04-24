#ifndef _TJScriptMap_H
#define _TJScriptMap_H

namespace tj {
	namespace script {

		class ScriptMapType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptMapType();
		};

		class SCRIPT_EXPORTED ScriptMap: public ScriptObject<ScriptMap> {
			friend class ScriptMapIterator;

			public:	
				ScriptMap();
				virtual ~ScriptMap();
				static void Initialize();

				std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator GetBegin();
				std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator GetEnd();
				
				// Script functions
				virtual tj::shared::ref<Scriptable> Size(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Set(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Get(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Keys(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Values(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> ToString(tj::shared::ref<ParameterList> p);

			protected:
				std::map<std::wstring, tj::shared::ref<Scriptable> > _array;
				tj::shared::CriticalSection _lock;
		};
	}
}

#endif