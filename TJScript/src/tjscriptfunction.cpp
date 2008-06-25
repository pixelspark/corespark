#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ScriptFunction::ScriptFunction(ref<Scriptlet> s) {
	_scriptlet = s;
}

ref<Scriptable> ScriptFunction::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[function]"));
}

ScriptFunction::~ScriptFunction() {
}

void ScriptFunction::Initialize() {
	Bind(L"toString", &SToString);
}

ref<Scriptable> ScriptDelegate::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[delegate]"));
}

void ScriptDelegate::Initialize() {
	Bind(L"toString", &SToString);
}