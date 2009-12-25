#ifndef _TJSCRIPT_DELEGATE_H
#define _TJSCRIPT_DELEGATE_H

namespace tj {
	namespace script {
		using tj::shared::ref;
		using tj::shared::strong;

		class SCRIPT_EXPORTED ScriptDelegate: public ScriptObject<ScriptDelegate> {
			public:
				ScriptDelegate(ref<CompiledScript> sc, ref<ScriptContext> ctx);
				virtual ~ScriptDelegate();
				static void Initialize();
				ref<CompiledScript> GetScript();
				ref<ScriptContext> GetContext();
				ref<Scriptable> SToString(ref<ParameterList> p);

			protected:
				ref<CompiledScript> _cs;
				ref<ScriptContext> _context;
		};

		class SCRIPT_EXPORTED ScriptFuture: public ScriptObject<ScriptFuture>, public tj::shared::Future {
			public:
				ScriptFuture(ref<CompiledScript> cs, ref<ScriptContext> originalContext);
				virtual ~ScriptFuture();
				virtual void Run();
				virtual bool IsConcrete();
				virtual ref<Scriptable> GetReturnValue();
				virtual bool WaitForCompletion();
				virtual void AddDependency(Field field, strong<ScriptFuture> fut);
				virtual void AddVariable(Field field, strong<Scriptable> sc);

				static void Initialize();
				virtual ref<Scriptable> SToString(ref<ParameterList> p);

			protected:
				strong<CompiledScript> _cs;
				strong<ScriptContext> _originalContext;
				strong<ScriptScope> _scope;
				ref<Scriptable> _returnValue;
				bool _isConcrete;
				tj::shared::Event _finished;
		};
	}
}

#endif