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
 
 #ifndef _TJSCRIPTTYPE_H
#define _TJSCRIPTTYPE_H

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptType: public Scriptable {
			public:
				virtual ~ScriptType();
				virtual tj::shared::ref<Scriptable> Construct(tj::shared::ref<ParameterList> p) = 0;
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p); // for static methods of this type
		};
		
		class SCRIPT_EXPORTED ScriptPackage: public virtual tj::shared::Object {
			public:
				ScriptPackage();
				virtual ~ScriptPackage();
				static tj::shared::strong<ScriptPackage> DefaultInstance();
				virtual void AddType(const std::wstring& ident, tj::shared::strong<ScriptType> st);
				virtual tj::shared::ref<ScriptType> GetType(const std::wstring& type);
			
			protected:
				tj::shared::CriticalSection _lock;
				std::map< std::wstring, tj::shared::ref<ScriptType> > _types;
				static tj::shared::ref<ScriptPackage> _instance;
			
		};
	}
}

#endif