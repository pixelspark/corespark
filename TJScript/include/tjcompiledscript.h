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

		class SCRIPT_EXPORTED CompiledScript: public virtual tj::shared::Object {
			public:
				CompiledScript();
				virtual ~CompiledScript();
				void Optimize();
				
				// for internal use
				tj::shared::ref<Scriptlet> CreateScriptlet(ScriptletType type);
				tj::shared::ref<Scriptlet> GetScriptlet(int i);
				int GetScriptletIndex(tj::shared::ref<Scriptlet> s);
				tj::shared::ref<Scriptlet> GetMainScriptlet();
				
			protected:
				std::vector< tj::shared::ref<Scriptlet> > _scriptlets;
		};
	}
}

#endif