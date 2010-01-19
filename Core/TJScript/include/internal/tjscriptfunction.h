#ifndef _TJSCRIPTFUNCTION_H
#define _TJSCRIPTFUNCTION_H

namespace tj {
	namespace script {
		class ScriptFunction: public ScriptObject<ScriptFunction> {
			public:
				ScriptFunction(tj::shared::ref<Scriptlet> s);
				virtual ~ScriptFunction();
				virtual tj::shared::ref<Scriptable> SToString(tj::shared::ref<ParameterList> p);
				static void Initialize();

				tj::shared::ref<Scriptlet> _scriptlet;
		};
	}
}

#endif