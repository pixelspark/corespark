#ifndef _TJSCRIPTFUNCTION_H
#define _TJSCRIPTFUNCTION_H

namespace tj {
	namespace script {
		class ScriptFunction: public Scriptable {
			public:
				ScriptFunction(tj::shared::ref<Scriptlet> s);
				virtual ~ScriptFunction();
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> plist);

				tj::shared::ref<Scriptlet> _scriptlet;
		};
	}
}

#endif