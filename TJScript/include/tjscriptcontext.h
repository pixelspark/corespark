#ifndef _TJSCRIPTCONTEXT_H
#define _TJSCRIPTCONTEXT_H

using tj::shared::ref;

namespace tj {
	namespace script {
		class ScriptScope;
		class VM;

		class SCRIPT_EXPORTED ScriptContext: public virtual tj::shared::Object {
			friend class ScriptThread;

			public:
				ScriptContext(ref<Scriptable> global);
				virtual ~ScriptContext();
				virtual ref<CompiledScript> Compile(std::wstring source);
				virtual ref<CompiledScript> CompileFile(std::wstring file);
				virtual ref<Scriptable> Execute(ref<CompiledScript> scr, ref<ScriptScope> scope=0);
				virtual ref<ScriptThread> CreateExecutionThread(ref<CompiledScript> scr);
				void SetDebug(bool d);
				void SetOptimize(bool o);

				template<typename T> static inline T GetValue(ref<Scriptable> s, T defaultValue);
				template<typename T> static inline T GetValueByString(ref<Scriptable> s, T defaultValue);

				// Types
				static void AddStaticType(const std::wstring& type, ref<ScriptType> stype);
				void AddType(const std::wstring& type, ref<ScriptType> stype);
				ref<ScriptType> GetType(const std::wstring& type);

			protected:
				ref<VM> _vm;
				ref<ScriptScope> _global;
				static std::map< std::wstring,ref<ScriptType> > _staticTypes;
				std::map< std::wstring, ref<ScriptType> > _types;
				bool _optimize;
				tj::shared::CriticalSection _running;
		};

		// Converting to some other type, either the object we want to convert is of the type desired
		// or we have to use the slow string stuff.
		template<typename T> T ScriptContext::GetValue(ref<Scriptable> s, T defaultValue) {
			if(s.IsCastableTo< ScriptValue<T> >()) {
				ref< ScriptValue<T> > value = s;
				return value->GetValue();
			}
			else {
				return GetValueByString<T>(s, defaultValue);
			}
		}

		template<> double SCRIPT_EXPORTED ScriptContext::GetValue(ref<Scriptable> s, double defaultValue);
		template<> float SCRIPT_EXPORTED ScriptContext::GetValue(ref<Scriptable> s, float defaultValue);
		template<> int SCRIPT_EXPORTED ScriptContext::GetValue(ref<Scriptable> s, int defaultValue);
		template<> bool SCRIPT_EXPORTED ScriptContext::GetValue(ref<Scriptable> s, bool defaultValue);

		// The 'always works, but slow'-method
		template<typename T> T ScriptContext::GetValueByString(ref<Scriptable> s, T defaultValue) {
			try {
				ref<Scriptable> str = s->Execute(L"toString", 0);
				if(str && str.IsCastableTo< ScriptValue<std::wstring> >()) {
					ref<ScriptValue<std::wstring> > value = str;
					return StringTo<T>(value->GetValue(), defaultValue);
				}
				return defaultValue;
			}
			catch(...) {
				return defaultValue;
			}
		}

		class ScriptDelegate: public Scriptable {
			public:
				inline ScriptDelegate(tj::shared::ref<CompiledScript> sc, tj::shared::ref<ScriptContext> ctx) {
					assert(sc);
					_cs = sc;
					_context = ctx;
				}

				virtual ~ScriptDelegate() {
				}

				inline tj::shared::ref<CompiledScript> GetScript() {
					return _cs;
				}

				inline tj::shared::ref<ScriptContext> GetContext() {
					return _context;
				}

				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> plist);

			protected:
				tj::shared::ref<CompiledScript> _cs;
				tj::shared::ref<ScriptContext> _context;
		};
	}
}

#endif