#include "../include/internal/tjscript.h"
#include <limits>
using namespace tj::shared;
using namespace tj::script;

ScriptContext::ScriptContext(ref<Scriptable> global) {
	_vm = GC::Hold(new VM());
	_global = GC::Hold(new ScriptScope());
	_global->SetPrevious(global);
	_optimize = true;
}

ScriptContext::~ScriptContext() {
}

Any ScriptContext::GetValue(ref<Scriptable> s) {
	if(!s) return Any();
	
	if(s.IsCastableTo<ScriptAny>()) {
		return ref<ScriptAny>(s)->Unbox();
	}
	else {
		ref<Scriptable> stringRepresentation = s->Execute(L"toString", null);
		if(stringRepresentation.IsCastableTo<ScriptAny>()) {
			return ref<ScriptAny>(stringRepresentation)->Unbox();
		}
	}
	return Any();
}

void ScriptContext::SetOptimize(bool o) {
	_optimize = o;
}

ref<Scriptable> ScriptContext::Execute(ref<CompiledScript> scr, ref<ScriptScope> scope) {
	ThreadLock lock(&_running);
	assert(scr);

	if(scr->_creatingContext!=0 && scr->_creatingContext!=this) {
		throw ScriptException(L"Script cannot be executed, because it was not created by this context");
	}

	if(scope) {
		scope->SetPrevious(_global);
		ref<Scriptable> val = _vm->Execute(this, scr, scope);
		scope->SetPrevious(0);
		return val;
	}
	else {
		return _vm->Execute(this, scr, _global);
	}
}

ref<ScriptThread> ScriptContext::CreateExecutionThread(ref<CompiledScript> scr) {
	if(scr->_creatingContext!=this) {
		throw ScriptException(L"Cannot create execution thread for this script, because it was not created by this context");
	}
	ref<ScriptThread> thread = GC::Hold(new ScriptThread(this));
	thread->SetScript(scr);
	return thread;
}

void ScriptContext::SetDebug(bool d) {
	_vm->SetDebug(d);
}

std::map<std::wstring, ref<ScriptType> > ScriptContext::_staticTypes;

void ScriptContext::AddStaticType(const std::wstring& type, ref<ScriptType> stype) {
	_staticTypes[type] = stype;
}

void ScriptContext::AddType(const std::wstring& type, ref<ScriptType> stype) {
	_types[type] = stype;
}

ref<ScriptType> ScriptContext::GetType(const std::wstring& type) {
	if(_types.find(type)!=_types.end()) {
		return _types[type];
	}
	else if(_staticTypes.find(type)!=_staticTypes.end()) {
		return _staticTypes[type];
	}

	throw ScriptException(std::wstring(L"The type ")+type+L" does not exist.");
}