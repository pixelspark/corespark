#ifndef _TJSCRIPT_DELEGATE_H
#define _TJSCRIPT_DELEGATE_H

namespace tj {
	namespace script {
		using tj::shared::ref;

		class SCRIPT_EXPORTED ScriptDelegate: public ScriptObject<ScriptDelegate> {
			public:
				ScriptDelegate(tj::shared::ref<CompiledScript> sc, tj::shared::ref<ScriptContext> ctx);
				virtual ~ScriptDelegate();
				static void Initialize();
				tj::shared::ref<CompiledScript> GetScript();
				tj::shared::ref<ScriptContext> GetContext();
				tj::shared::ref<Scriptable> SToString(tj::shared::ref<ParameterList> p);

			protected:
				tj::shared::ref<CompiledScript> _cs;
				tj::shared::ref<ScriptContext> _context;
		};
	}
}

#endif