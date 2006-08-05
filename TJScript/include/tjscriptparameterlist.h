#ifndef _TJSCRIPTPARAMETERLIST_H
#define _TJSCRIPTPARAMETERLIST_H

namespace tj {
	namespace script {

		class SCRIPT_EXPORTED ScriptParameterList: public Scriptable {
			public:
				ScriptParameterList();
				virtual ~ScriptParameterList();
				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params);
				void Set(std::wstring k, tj::shared::ref<Scriptable> v);

				tj::shared::ref<ParameterList> _params;
		};
	}
}

#endif