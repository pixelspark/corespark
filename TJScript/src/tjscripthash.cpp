#include "../include/internal/tjscript.h"
#include "../include/types/tjstatictypes.h"
using namespace tj::shared;
using namespace tj::script;

ref<Scriptable> ScriptHashType::Construct(tj::shared::ref<ParameterList> p) {
	static const Parameter<std::wstring> PData(L"data", 0);
	return GC::Hold(new ScriptHash(PData.Require(p,L"")));
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

void ScriptHash::Initialize() {
	Bind(L"toString", &ScriptHash::ToString);
	Bind(L"toInt", &ScriptHash::ToInt);
}

ref<Scriptable> ScriptHash::ToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(Stringify(_hash)));
}

ref<Scriptable> ScriptHash::ToInt(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_hash));
}