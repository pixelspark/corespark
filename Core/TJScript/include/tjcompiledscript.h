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
 
 #ifndef _TJCOMPILEDSCRIPT_H
#define _TJCOMPILEDSCRIPT_H

namespace tj {
	namespace script {
		class Scriptlet;

		enum ScriptletType {
			ScriptletAny=0,
			ScriptletFunction,
			ScriptletLoop,
		};

		class ScriptContext;

		class SCRIPT_EXPORTED CompiledScript: public virtual tj::shared::Object {
			friend class ScriptContext;

			public:
				// If creatingContext == 0, it cannot be executed by any context (only as delegate)
				// if creatingContext != 0, it can only be executed by the creatingContext
				CompiledScript(ScriptContext* creatingContext);
				virtual ~CompiledScript();
				void Optimize();
				
				// for internal use
				tj::shared::ref<Scriptlet> CreateScriptlet(ScriptletType type);
				tj::shared::ref<Scriptlet> GetScriptlet(int i);
				int GetScriptletIndex(tj::shared::ref<Scriptlet> s);
				tj::shared::ref<Scriptlet> GetMainScriptlet();
				int GetScriptletCount() const;
				
			protected:
				std::vector< tj::shared::ref<Scriptlet> > _scriptlets;
				ScriptContext* _creatingContext;
		};
	}
}

#endif