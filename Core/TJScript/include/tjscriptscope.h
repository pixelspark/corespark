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
 
 #ifndef _TJSCRIPTSCOPE_H
#define _TJSCRIPTSCOPE_H

#pragma warning(push)
#pragma warning(disable: 4251)

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptScope: public Scriptable {
			public:
				ScriptScope();
				virtual ~ScriptScope();
				tj::shared::ref<Scriptable> GetPrevious();
				void SetPrevious(tj::shared::ref<Scriptable> r);
				
				virtual tj::shared::ref<Scriptable> Execute(Command command, tj::shared::ref<ParameterList> params);
				virtual bool Set(Field field, tj::shared::ref<Scriptable> var);
				tj::shared::ref<Scriptable> Get(const std::wstring& key);
				bool Exists(const std::wstring& key);

				std::map< std::wstring, tj::shared::ref<Scriptable> > _vars;
				
			protected:
				tj::shared::ref<Scriptable> _previous;
		};

	}
}

#pragma warning(pop)
#endif