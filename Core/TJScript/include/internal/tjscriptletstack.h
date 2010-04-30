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