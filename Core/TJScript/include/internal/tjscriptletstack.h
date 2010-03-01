#ifndef _TJSCRIPTLETSTACK_H
#define _TJSCRIPTLETSTACK_H

namespace tj {
	namespace script {

		class ScriptletStack {
			public:
				inline ScriptletStack() {
				}
			
				virtual ~ScriptletStack();

				inline void Push(tj::shared::ref<Scriptlet> s, int n) {
					_stack.push_back(std::pair< tj::shared::ref<Scriptlet>,int >(s,n));
				}

				inline tj::shared::ref<Scriptlet> Top() {
					std::pair< tj::shared::ref<Scriptlet> ,int > s = *(_stack.rbegin());
					return s.first;
				}

				inline tj::shared::ref<Scriptlet> Pop() {
					std::pair< tj::shared::ref<Scriptlet> ,int > s = *(_stack.rbegin());
					_stack.pop_back();
					return s.first;
				}

				inline int GetCurrentIndex() {
					std::pair< tj::shared::ref<Scriptlet> ,int > s = *(_stack.rbegin());
					return s.second;
				}

				std::deque< std::pair< tj::shared::ref<Scriptlet>, int> > _stack;
		};
	}
}

#endif