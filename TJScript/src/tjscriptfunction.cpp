#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ref<Scriptable> ScriptFunction::Execute(Command c, ref<ParameterList> plist) {
	if(c==L"toString") {
		return GC::Hold(new ScriptString(L"[function]"));
	}

	return 0;
}

ScriptFunction::~ScriptFunction() {
}