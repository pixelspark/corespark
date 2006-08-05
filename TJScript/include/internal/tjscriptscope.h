#ifndef _TJSCRIPTSCOPE_H
#define _TJSCRIPTSCOPE_H

namespace tj {
	namespace script {

		class ScriptScope: public Scriptable {
			public:
				ScriptScope(tj::shared::ref<ScriptParameterList> p=0);
				virtual ~ScriptScope();
				tj::shared::ref<Scriptable> GetPrevious();
				void SetPrevious(tj::shared::ref<Scriptable> r);
				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params);
				virtual void Set(std::wstring, tj::shared::ref<Scriptable> var);
			protected:
				tj::shared::ref<Scriptable> _previous;
				std::map< std::wstring, tj::shared::ref<Scriptable> > _vars;
		};

	}
}

#endif