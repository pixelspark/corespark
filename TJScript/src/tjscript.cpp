#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

std::wostringstream& operator <<(std::wostringstream& i, const _Null& n) {
	i << L"null";
	return i;
}

ref<Scriptable> ScriptConstants::_null;

ref<Scriptable> ScriptConstants::Null() {
	if(!_null) {
		_null = GC::Hold(new ScriptNull());
	}

	return _null;
}
