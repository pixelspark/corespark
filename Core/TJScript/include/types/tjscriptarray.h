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
 
 #ifndef _TJSCRIPTARRAY_H
#define _TJSCRIPTARRAY_H

#pragma warning(push)
#pragma warning(disable: 4251 4275)

namespace tj {
	namespace script {

		class ScriptArrayType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptArrayType();
		};

		class SCRIPT_EXPORTED ScriptArray: public ScriptObject<ScriptArray> {
			friend class ScriptArrayIterator;

			public:
				ScriptArray();
				virtual ~ScriptArray();
				static void Initialize();
				void Push(ref<Scriptable> st);

				// Script functions
				virtual tj::shared::ref<Scriptable> SToString(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SPush(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SPop(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SGet(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SCount(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SImplode(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> SValues(tj::shared::ref<ParameterList> p);

			protected:
				std::vector< tj::shared::ref<Scriptable> > _array;
				tj::shared::CriticalSection _lock;
		};
	}
}

#pragma warning(pop)

#endif