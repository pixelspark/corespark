#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ScriptParameterList::ScriptParameterList() {
	_namelessCount = 0;
}

ScriptParameterList::~ScriptParameterList() {
}

void ScriptParameterList::AddNamelessParameter(ref<Scriptable> t) {
	Set(Stringify(_namelessCount), t);
	_namelessCount++;
}