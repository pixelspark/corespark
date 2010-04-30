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
 
 #ifndef _SCRIPTITERATOR_H
#define _SCRIPTITERATOR_H

namespace tj {
	namespace script {

		template<typename T> class ScriptIterator: public Scriptable {
			public:
				virtual ~ScriptIterator() {
				}

				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p) {
					if(c==L"toString") {
						return tj::shared::GC::Hold(new ScriptString(L"[Iterator]"));
					}
					else if(c==L"next") {
						return Next();
					}
					return 0;
				}

				virtual tj::shared::ref<Scriptable> Next() = 0;
		};
	}
}

#endif