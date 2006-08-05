#ifndef _TJSCRIPTVM_H
#define _TJSCRIPTVM_H

namespace tj {
	namespace script {
		class CompiledScript;
		class Op;
		class ScriptStack;
		class ScriptScope;
		class ScriptContext;

		class Scriptlet: public virtual tj::shared::Object {
			friend class VM;

			public:
				enum ScriptletType {
					ScriptletAny=0,
					ScriptletFunction,
					ScriptletLoop,
				};

				Scriptlet(ScriptletType type);
				virtual ~Scriptlet();
				void Add(tj::shared::ref<Op> op);
				inline bool IsFunction() const {
					return _type==ScriptletFunction;
				}

				inline bool IsLoop() const {
					return _type==ScriptletLoop;
				}

			protected:
				std::vector< tj::shared::ref<Op> > _code;
				ScriptletType _type;

		};

		struct StackFrame {
			public:
				inline StackFrame(tj::shared::ref<Scriptlet> s, unsigned int pc) {
					_scriptlet = s;
					_pc = pc;
					_stackSize = 0;
				}
				
				unsigned int _pc;
				tj::shared::ref<Scriptlet> _scriptlet;
				unsigned int _stackSize;
		};

		class VM: public virtual tj::shared::Object {
			public:
				VM(int stackLimit=512);
				virtual ~VM();
				void Execute(tj::shared::ref<ScriptContext> c, tj::shared::ref<CompiledScript> script);
				tj::shared::ref<ScriptStack> GetStack();
				tj::shared::ref<CompiledScript> GetScript();
				void SetGlobal(tj::shared::ref<Scriptable> global);
				tj::shared::ref<ScriptScope> GetGlobal();
				tj::shared::ref<ScriptContext> GetContext();
				tj::shared::ref<StackFrame> GetStackFrame();
				void SetDebug(bool d);

				// to be called by ops or vm during execution
				void Call(tj::shared::ref<Scriptlet> s, tj::shared::ref<ScriptParameterList> sc=0);
				void Call(int n);
				void Return();
				void Break();
				
			protected:
				tj::shared::ref<ScriptStack> _stack;
				tj::shared::ref<ScriptScope> _global;
				tj::shared::ref<CompiledScript> _script;
				tj::shared::weak<ScriptContext> _context;
				std::deque< tj::shared::ref<StackFrame> > _call;
				bool _debug;
		};
	}
}

#endif