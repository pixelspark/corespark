#ifndef _TJSCRIPTPARAMETERLIST_H
#define _TJSCRIPTPARAMETERLIST_H

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptParameterList: public ScriptScope {
			public:
				ScriptParameterList();
				virtual ~ScriptParameterList();
				void AddNamelessParameter(tj::shared::ref<Scriptable> t);

			protected:
				int _namelessCount;
		};
	}
}

#endif