#ifndef _TJSCRIPTSTACK_H
#define _TJSCRIPTSTACK_H

namespace tj {
	namespace script {
		
		class ScriptStack {
			friend class VM;

			public:	
				inline ScriptStack(int stackLimit=512) {
					_stack = new tj::shared::ref<Scriptable>[stackLimit];
					_limit = stackLimit-2;
					_sp = -1;
				}

				virtual ~ScriptStack();

				inline void Push(tj::shared::ref<Scriptable> sc) {
					_sp++;
					if(_sp>_limit) {
						throw ScriptException(L"Stack overflow!");
					}
					_stack[_sp] = sc;
				}

				inline bool IsEmpty() const {
					return _sp==-1;
				}

				inline unsigned int GetSize() const {
					return (unsigned int)_sp+1;
				}

				inline tj::shared::ref<Scriptable> Top() {
					if(_sp<0) {
						throw ScriptException(L"Stack underflow!");
					}
					return _stack[_sp];
				}

				void Pop(int size);
				void Clear();
				tj::shared::ref<Scriptable> Pop();
				std::wstring Dump();

			protected:
				tj::shared::ref<Scriptable>* _stack;
				int _limit;
				int _sp;

			private:
				// disallow copy
				ScriptStack(ScriptStack& other) {
				}
		};

	}
}
#endif