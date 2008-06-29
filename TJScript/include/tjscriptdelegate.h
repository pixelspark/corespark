#ifndef _TJSCRIPT_DELEGATE_H
#define _TJSCRIPT_DELEGATE_H

namespace tj {
	namespace script {
		using tj::shared::ref;

		class ScriptDelegate: public ScriptObject<ScriptDelegate> {
			public:
				inline ScriptDelegate(tj::shared::ref<CompiledScript> sc, tj::shared::ref<ScriptContext> ctx) {
					assert(sc);
					_cs = sc;
					_context = ctx;
				}

				virtual ~ScriptDelegate() {
				}

				static void Initialize();

				inline tj::shared::ref<CompiledScript> GetScript() {
					return _cs;
				}

				inline tj::shared::ref<ScriptContext> GetContext() {
					return _context;
				}

				tj::shared::ref<Scriptable> SToString(tj::shared::ref<ParameterList> p);

			protected:
				tj::shared::ref<CompiledScript> _cs;
				tj::shared::ref<ScriptContext> _context;
		};
	}
}

#endif