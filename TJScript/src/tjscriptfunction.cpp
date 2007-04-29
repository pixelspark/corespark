#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ScriptFunction::ScriptFunction(ref<Scriptlet> s) {
	_scriptlet = s;
}

ref<Scriptable> ScriptFunction::Execute(Command c, ref<ParameterList> plist) {
	if(c==L"toString") {
		return GC::Hold(new ScriptString(L"[function]"));
	}

	return 0;
}

ScriptFunction::~ScriptFunction() {
}

ref<Scriptable> ScriptDelegate::Execute(Command c, ref<ParameterList> p) {
	if(c==L"toString") {
		return GC::Hold(new ScriptString(L"[delegate]"));
	}
	return 0;
}