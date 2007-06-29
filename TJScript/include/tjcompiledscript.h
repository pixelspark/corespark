#ifndef _TJCOMPILEDSCRIPT_H
#define _TJCOMPILEDSCRIPT_H

namespace tj {
	namespace script {
		class Scriptlet;

		enum ScriptletType {
			ScriptletAny=0,
			ScriptletFunction,
			ScriptletLoop,
		};

		class ScriptContext;

		class SCRIPT_EXPORTED CompiledScript: public virtual tj::shared::Object {
			friend class ScriptContext;

			public:
				// If creatingContext == 0, it cannot be executed by any context (only as delegate)
				// if creatingContext != 0, it can only be executed by the creatingContext
				CompiledScript(ScriptContext* creatingContext);
				virtual ~CompiledScript();
				void Optimize();
				
				// for internal use
				tj::shared::ref<Scriptlet> CreateScriptlet(ScriptletType type);
				tj::shared::ref<Scriptlet> GetScriptlet(int i);
				int GetScriptletIndex(tj::shared::ref<Scriptlet> s);
				tj::shared::ref<Scriptlet> GetMainScriptlet();
				int GetScriptletCount() const;
				
			protected:
				std::vector< tj::shared::ref<Scriptlet> > _scriptlets;
				ScriptContext* _creatingContext;
		};
	}
}

#endif