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
 
 #include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ScriptThread::ScriptThread(ref<ScriptContext> ctx) {
	_context = ctx; // weak
	_clean = false;
}

ScriptThread::~ScriptThread() {
	Log::Write(L"TJScript/ScriptThread", L"Destroy");
}

void ScriptThread::SetCleanupAfterRun(bool c) {
	_clean = c;
}

void ScriptThread::Run() {
	Timestamp start(true);

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

	Timestamp end(true);
	Log::Write(L"TJScript/ScriptThread", L"Script ended; t="+Stringify(end.Difference(start).ToMilliSeconds())+L"ms");

	if(_clean) {
		_script = 0;
	}
}

void ScriptThread::SetScript(ref<CompiledScript> x) {
	_script = x;
}