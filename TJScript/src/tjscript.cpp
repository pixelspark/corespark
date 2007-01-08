#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

std::wostringstream& operator <<(std::wostringstream& i, const _Null& n) {
	i << L"null";
	return i;
}

ref<Scriptable> ScriptConstants::_null;
ref<Scriptable> ScriptConstants::_true;
ref<Scriptable> ScriptConstants::_false;

ref<Scriptable> ScriptConstants::Null() {
	if(!_null) {
		_null = GC::Hold(new ScriptNull());
	}

	return _null;
}

ref<Scriptable> ScriptConstants::True() {
	if(!_true) {
		_true = GC::Hold(new ScriptBool(true));
	}

	return _true;
}

ref<Scriptable> ScriptConstants::False() {
	if(!_false) {
		_false = GC::Hold(new ScriptBool(false));
	}

	return _false;
}
