#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ref<Scriptable> ScriptConstants::Null;
ref<Scriptable> ScriptConstants::True;
ref<Scriptable> ScriptConstants::False;
ScriptConstantsInitializer ScriptConstants::_init;

ScriptConstantsInitializer::ScriptConstantsInitializer() {
	ScriptConstants::Null = GC::Hold(new ScriptNull());
	ScriptConstants::True = GC::Hold(new ScriptBool(true));
	ScriptConstants::False = GC::Hold(new ScriptBool(false));
}

Scriptable::~Scriptable() {
}

bool Scriptable::Set(Field field, tj::shared::ref<Scriptable> value) {
	return false;
}