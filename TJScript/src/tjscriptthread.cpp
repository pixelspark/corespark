#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ScriptThread::ScriptThread(ref<ScriptContext> ctx) {
	_context = ctx; // weak
	_clean = false;
}

ScriptThread::~ScriptThread() {
}

void ScriptThread::SetCleanupAfterRun(bool c) {
	_clean = c;
}

void ScriptThread::Run() {
	int start = GetTickCount();

	{
		ref<ScriptContext> ctx = _context;
		if(!ctx) {
			return;
		}

		ThreadLock lock(&(ctx->_running));
		try {
			ctx->Execute(_script);
		}
		catch(ScriptException& e) {
			Log::Write(L"TJScript/ScriptThread", L"Script execution stopped: "+e.GetMsg());
		}
		catch(...) {
			Log::Write(L"TJScript/ScriptThread", L"Script execution failed with an unknown exception");
		}
	}

	Log::Write(L"TJScript/ScriptThread", L"Script ended; t="+Stringify(GetTickCount()-start)+L"ms");

	if(_clean) {
		_script = 0;
	}
}

void ScriptThread::SetScript(ref<CompiledScript> x) {
	_script = x;
}