#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ScriptContext::ScriptContext(ref<Scriptable> global) {
	_vm = GC::Hold(new VM());
	_vm->SetGlobal(global);
	_optimize = true;
}

ScriptContext::~ScriptContext() {
}

void ScriptContext::SetOptimize(bool o) {
	_optimize = o;
}

void ScriptContext::Execute(ref<CompiledScript> scr) {
	ThreadLock lock(&_running);
	assert(scr);
	_vm->Execute(This<ScriptContext>(), scr);
}

ref<ScriptScope> ScriptContext::GetGlobal() {
	return _vm->GetGlobal();
}

ref<ScriptThread> ScriptContext::CreateExecutionThread(ref<CompiledScript> scr) {
	ref<ScriptThread> thread = GC::Hold(new ScriptThread(This<ScriptContext>()));
	thread->SetScript(scr);
	return thread;
}

void ScriptContext::SetDebug(bool d) {
	_vm->SetDebug(d);
}

std::map<std::wstring, ref<ScriptType> > ScriptContext::_staticTypes;

void ScriptContext::AddStaticType(std::wstring type, ref<ScriptType> stype) {
	_staticTypes[type] = stype;
}

void ScriptContext::AddType(std::wstring type, ref<ScriptType> stype) {
	_types[type] = stype;
}

ref<ScriptType> ScriptContext::GetType(std::wstring type) {
	if(_types.find(type)!=_types.end()) {
		return _types[type];
	}
	else if(_staticTypes.find(type)!=_staticTypes.end()) {
		return _staticTypes[type];
	}

	throw ScriptException(std::wstring(L"The type ")+type+L" does not exist.");
}
