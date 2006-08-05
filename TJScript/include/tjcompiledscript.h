#ifndef _TJCOMPILEDSCRIPT_H
#define _TJCOMPILEDSCRIPT_H

namespace tj {
	namespace script {

		class SCRIPT_EXPORTED CompiledScript: public virtual tj::shared::Object {
			public:
				CompiledScript();
				virtual ~CompiledScript();
				tj::shared::ref<Scriptlet> CreateScriptlet(Scriptlet::ScriptletType type);
				tj::shared::ref<Scriptlet> GetScriptlet(int i);
				int GetScriptletIndex(tj::shared::ref<Scriptlet> s);
				tj::shared::ref<Scriptlet> GetMainScriptlet();
				void Optimize();

			protected:
				std::vector< tj::shared::ref<Scriptlet> > _scriptlets;
		};
	}
}

#endif