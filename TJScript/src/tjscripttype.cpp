#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ScriptType::~ScriptType() {
}

ref<Scriptable> ScriptType::Execute(Command c, ref<ParameterList> p) {
	return 0;
}