#ifndef _TJSCRIPTSCOPE_H
#define _TJSCRIPTSCOPE_H

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptScope: public Scriptable {
			public:
				ScriptScope();
				virtual ~ScriptScope();
				tj::shared::ref<Scriptable> GetPrevious();
				void SetPrevious(tj::shared::ref<Scriptable> r);
				
				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params);
				virtual void Set(std::wstring, tj::shared::ref<Scriptable> var);
				ref<Scriptable> Get(const std::wstring& key);
				bool Exists(const std::wstring& key);

				std::map< std::wstring, tj::shared::ref<Scriptable> > _vars;

			protected:
				tj::shared::ref<Scriptable> _previous;
		};

	}
}

#endif