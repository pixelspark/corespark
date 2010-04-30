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
 
 #ifndef _TJScriptMap_H
#define _TJScriptMap_H

#pragma warning (push)
#pragma warning (disable: 4275 4251)

namespace tj {
	namespace script {

		class ScriptMapType: public ScriptType {
			public:
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p);
				virtual ~ScriptMapType();
		};

		class SCRIPT_EXPORTED ScriptMap: public ScriptObject<ScriptMap> {
			friend class ScriptMapIterator;

			public:	
				ScriptMap();
				virtual ~ScriptMap();
				static void Initialize();
				virtual bool Set(Field field, tj::shared::ref<Scriptable> value);

				std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator GetBegin();
				std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator GetEnd();
				
				// Script functions
				virtual tj::shared::ref<Scriptable> Size(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Get(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Keys(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> Values(tj::shared::ref<ParameterList> p);
				virtual tj::shared::ref<Scriptable> ToString(tj::shared::ref<ParameterList> p);

			protected:
				std::map<std::wstring, tj::shared::ref<Scriptable> > _array;
				tj::shared::CriticalSection _lock;
		};
	}
}

#pragma warning(pop)

#endif
