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
		ref<Scriptable> val = _vm->Execute(This<ScriptContext>(), scr, scope);
		scope->SetPrevious(0);
		return val;
	}
	else {
		return _vm->Execute(This<ScriptContext>(), scr, _global);
	}
}

ref<ScriptThread> ScriptContext::CreateExecutionThread(ref<CompiledScript> scr) {
	if(scr->_creatingContext!=this) {
		throw ScriptException(L"Cannot create execution thread for this script, because it was not created by this context");
	}
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

// doubles
template<> double ScriptContext::GetValue(ref<Scriptable> s, double defaultValue) {
	if(s.IsCastableTo<ScriptInt>()) {
		return double(ref<ScriptInt>(s)->GetValue());
	}
	else if(s.IsCastableTo<ScriptDouble>()) {
		return double(ref<ScriptDouble>(s)->GetValue());
	}
	else if(s.IsCastableTo<ScriptBool>()) {
		return ref<ScriptBool>(s)->GetValue()?1.0:0.0;
	}
	else if(s.IsCastableTo<ScriptNull>()) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	else {
		return GetValueByString<double>(s,defaultValue);	
	}
}

// floats are not different from doubles in the scripting engine (doubles are used everywhere) so use double
template<> float ScriptContext::GetValue(ref<Scriptable> s, float defaultValue) {
	return (float)GetValue<double>(s,(double)defaultValue);
}

// ints
template<> int ScriptContext::GetValue(ref<Scriptable> s, int defaultValue) {
	if(s.IsCastableTo<ScriptInt>()) {
		return ref<ScriptInt>(s)->GetValue();
	}
	else if(s.IsCastableTo<ScriptDouble>()) {
		return int(ref<ScriptDouble>(s)->GetValue());
	}
	else if(s.IsCastableTo<ScriptBool>()) {
		return ref<ScriptBool>(s)->GetValue()?1:0;
	}
	else if(s.IsCastableTo<ScriptNull>()) {
		return 0;
	}
	else {
		return GetValueByString<int>(s,defaultValue);	
	}
}

// booleans
template<> bool ScriptContext::GetValue(ref<Scriptable> s, bool defaultValue) {
	if(s.IsCastableTo<ScriptInt>()) {
		return ref<ScriptInt>(s)->GetValue()!=0;
	}
	else if(s.IsCastableTo<ScriptDouble>()) {
		return ref<ScriptDouble>(s)->GetValue()!=0.0;
	}
	else if(s.IsCastableTo<ScriptBool>()) {
		return ref<ScriptBool>(s)->GetValue();
	}
	else if(s.IsCastableTo<ScriptNull>()) {
		return false;
	}
	else {
		return GetValueByString<bool>(s,defaultValue);	
	}
}