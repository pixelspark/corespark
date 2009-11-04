#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ScriptParameterList::ScriptParameterList() {
	_namelessCount = 0;
}

ScriptParameterList::~ScriptParameterList() {
}

void ScriptParameterList::AddNamelessParameter(ref<Scriptable> t) {
	std::wstring parameterName = Stringify(_namelessCount);
	Set(parameterName, t);
	_namelessCount++;
}
