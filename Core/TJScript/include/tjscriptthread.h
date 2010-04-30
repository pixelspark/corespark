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
 
 #ifndef _TJSCRIPTTHREAD_H
#define _TJSCRIPTTHREAD_H

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptThread: public tj::shared::Thread {
			public:
				ScriptThread(tj::shared::ref<ScriptContext> ctx);
				virtual ~ScriptThread();
				virtual void Run();
				void SetScript(tj::shared::ref<CompiledScript> scr);

				// Set if you want to dereference _context and _script after execution
				void SetCleanupAfterRun(bool c);

			protected:
				tj::shared::weak<ScriptContext> _context;
				tj::shared::ref<CompiledScript> _script;
				bool _clean;
		};
	}
}

#endif