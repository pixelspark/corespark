#ifndef _TJSCRIPTCONTEXT_H
#define _TJSCRIPTCONTEXT_H

using tj::shared::ref;

namespace tj {
	namespace script {
		class ScriptScope;
		class VM;
		class ScriptThread;

		class SCRIPT_EXPORTED ScriptContext: public virtual tj::shared::Object {
			friend class ScriptThread;

			public:
				ScriptContext(ref<Scriptable> global);
				virtual ~ScriptContext();
				virtual ref<CompiledScript> Compile(std::wstring source);
				virtual ref<CompiledScript> CompileFile(std::wstring file);
				virtual ref<Scriptable> Execute(ref<CompiledScript> scr, ref<ScriptScope> scope=0);
				virtual ref<ScriptThread> CreateExecutionThread(ref<CompiledScript> scr);
				virtual ref<Scriptable> GetGlobal();
				void SetDebug(bool d);
				void SetOptimize(bool o);
				virtual void SetDispatcher(ref<tj::shared::Dispatcher> d);
				virtual tj::shared::strong<tj::shared::Dispatcher> GetDispatcher();

				template<typename T> static inline T GetValue(ref<Scriptable> s, T defaultValue);
				static tj::shared::Any GetValue(ref<Scriptable> s);
			
				// Types
				void AddType(const std::wstring& type, ref<ScriptType> stype);
				ref<ScriptType> GetType(const std::wstring& type);

			protected:
				ref<VM> _vm;
				ref<tj::shared::Dispatcher> _dispatcher;
				ref<ScriptScope> _global;
				std::map< std::wstring, ref<ScriptType> > _types;
				bool _optimize;
				tj::shared::CriticalSection _running;
		};

		// Converting to some other type, either the object we want to convert is of the type desired
		// or we have to use the slow string stuff.
		template<typename T> T ScriptContext::GetValue(ref<Scriptable> s, T defaultValue) {
			if(s.IsCastableTo< ScriptAny >()) {
				ref<ScriptAny> value = s;
				return (T)(value->Unbox());
			}
			else {
				ref<Scriptable> stringRep = s->Execute(L"toString", tj::shared::null);
				if(stringRep.IsCastableTo<ScriptAny>()) {
					return (T)(ref<ScriptAny>(stringRep)->Unbox());
				}
			}
			return defaultValue;
		}
	}
}

#endif