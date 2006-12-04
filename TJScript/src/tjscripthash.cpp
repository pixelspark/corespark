#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ref<Scriptable> ScriptHashType::Construct(tj::shared::ref<ParameterList> p) {
	if(!p) {
		return 0;
	}

	RequiredParameter<std::wstring> data(p, L"data", L"",0);
	return GC::Hold(new ScriptHash(data.Get()));
}

ScriptHashType::~ScriptHashType() {
}

ScriptHash::~ScriptHash() {
}

ScriptHash::ScriptHash(std::wstring w) {
	Hash h;
	_hash = h.Calculate(w);
}

ScriptHash::ScriptHash(int x) {
	_hash = x;
}

ref<Scriptable> ScriptHash::Execute(Command c, ref<ParameterList> p) {
	if(c==L"toString") {
		return GC::Hold(new ScriptString(Stringify(_hash)));
	}
	else if(c==L"toInt") {
		return GC::Hold(new ScriptInt(_hash));
	}

	return 0;
}