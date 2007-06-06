#ifndef _TJSCRIPTVM_H
#define _TJSCRIPTVM_H

namespace tj {
	namespace script {
		class CompiledScript;
		class Op;
		class ScriptScope;
		class ScriptContext;

		struct StackFrame {
				inline StackFrame(tj::shared::ref<Scriptlet> s, unsigned int pc, bool createdScope=false) {
					_scriptlet = s;
					_pc = pc;
					_stackSize = 0;
					_createdScope = createdScope;
					_previous = 0;
				}

				~StackFrame() {
				}
				
				unsigned int _pc;
				tj::shared::ref<Scriptlet> _scriptlet;
				unsigned int _stackSize;
				bool _createdScope; // True if a scope was created for this frame
				StackFrame* _previous;
		};

		class VM: public virtual tj::shared::Object {
			public:
				VM(int stackLimit=512);
				virtual ~VM();
				ref<Scriptable> Execute(tj::shared::ref<ScriptContext> c, tj::shared::ref<CompiledScript> script, tj::shared::ref<ScriptScope> global);
				void SetDebug(bool d);				

				// to be called by ops or vm during execution
				void Call(tj::shared::ref<Scriptlet> s, tj::shared::ref<ScriptParameterList> sc=0);
				void Call(int n);
				void Return(bool takeLastValue);
				void Break();

				// Inlines
				inline tj::shared::ref<ScriptScope> GetCurrentScope() {
					return _scope;
				}

				tj::shared::ref<ScriptScope> GetCurrentScopeForWriting();
				
				inline tj::shared::ref<CompiledScript> GetScript() {
					return _script;
				}

				inline tj::shared::ref<ScriptContext> GetContext() {
					return _context;
				}

				inline StackFrame* GetStackFrame() {
					return _frame;
				}

				inline ScriptStack& GetStack() {
					return _stack;
				}
				
			protected:
				ScriptStack _stack;
				tj::shared::ref<ScriptScope> _scope;
				tj::shared::ref<CompiledScript> _script;
				tj::shared::weak<ScriptContext> _context;
				StackFrame* _frame;
				bool _debug;
		};
	}
}

#endif