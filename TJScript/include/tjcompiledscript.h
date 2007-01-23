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
				int GetScriptletCount() const;
				
			protected:
				std::vector< tj::shared::ref<Scriptlet> > _scriptlets;
		};

		class ScriptDelegate: public Scriptable {
			public:
				inline ScriptDelegate(tj::shared::ref<CompiledScript> sc) {
					assert(sc);
					_cs = sc;
				}

				virtual ~ScriptDelegate() {
				}

				inline tj::shared::ref<CompiledScript> GetScript() {
					return _cs;
				}

				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> plist);

			protected:
				tj::shared::ref<CompiledScript> _cs;
		};
	}
}

#endif