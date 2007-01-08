#ifndef _TJSCRIPTCONTEXT_H
#define _TJSCRIPTCONTEXT_H

namespace tj {
	namespace script {
		class ScriptScope;
		class VM;

		class SCRIPT_EXPORTED ScriptContext: public virtual tj::shared::Object {
			friend class ScriptThread;

			public:
				ScriptContext(tj::shared::ref<Scriptable> global);
				virtual ~ScriptContext();
				virtual tj::shared::ref<CompiledScript> Compile(std::wstring source);
				virtual tj::shared::ref<CompiledScript> CompileFile(std::wstring file);
				virtual void Execute(tj::shared::ref<CompiledScript> scr, tj::shared::ref<ScriptScope> scope=0);
				virtual tj::shared::ref<ScriptThread> CreateExecutionThread(tj::shared::ref<CompiledScript> scr);
				void SetDebug(bool d);
				void SetOptimize(bool o);

				template<typename T> static T GetValue(tj::shared::ref<Scriptable> s, T defaultValue) {
					if(s.IsCastableTo< ScriptValue<T> >()) {
						tj::shared::ref< ScriptValue<T> > value = s;
						return value->GetValue();
					}
					else {
						try {
							tj::shared::ref<Scriptable> str = s->Execute(L"toString", 0);
							if(str && str.IsCastableTo< ScriptValue<std::wstring> >()) {
								tj::shared::ref<ScriptValue<std::wstring> > value = str;
								return StringTo<T>(value->GetValue(), defaultValue);
							}
							return defaultValue;
						}
						catch(...) {
							return defaultValue;
						}
					}
				}

				// Types
				static void AddStaticType(std::wstring type, tj::shared::ref<ScriptType> stype);
				void AddType(std::wstring type, tj::shared::ref<ScriptType> stype);
				tj::shared::ref<ScriptType> GetType(std::wstring type);

			protected:
				tj::shared::ref<VM> _vm;
				tj::shared::ref<ScriptScope> _global;
				static std::map< std::wstring, tj::shared::ref<ScriptType> > _staticTypes;
				std::map< std::wstring, tj::shared::ref<ScriptType> > _types;
				bool _optimize;
				tj::shared::CriticalSection _running;
		};
	}
}

#endif