/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
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