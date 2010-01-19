#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ScriptType::~ScriptType() {
}

ref<Scriptable> ScriptType::Execute(Command c, ref<ParameterList> p) {
	return 0;
}

/** ScriptPackage **/
// DefaultInstance is defined in tjstatictypes.cpp
ScriptPackage::~ScriptPackage() {
}

ScriptPackage::ScriptPackage() {
}

void ScriptPackage::AddType(const std::wstring& identifier, strong<ScriptType> st) {
	ThreadLock lock(&_lock);
	_types[identifier] = st;
}

ref<ScriptType> ScriptPackage::GetType(const std::wstring& type) {
	ThreadLock lock(&_lock);
	std::map<std::wstring, ref<ScriptType> >::iterator it = _types.find(type);
	if(it!=_types.end()) {
		return it->second;
	}
	return null;
}