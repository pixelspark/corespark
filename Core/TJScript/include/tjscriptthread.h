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