#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ref<Scriptable> ScriptRangeType::Construct(tj::shared::ref<ParameterList> p) {
	static const Parameter<int> PFrom(L"from",0);
	static const Parameter<int> PTo(L"to",1);	

	int from = PFrom.Require(p,0);
	int to = PTo.Require(p,0);

	if(to<from) {
		return 0;
	}

	return GC::Hold(new ScriptRange(from,to));
}

ScriptRangeType::~ScriptRangeType() {
}

ScriptRange::~ScriptRange() {
}

ScriptRange::ScriptRange(int a, int b): _a(a), _b(b) {
}

void ScriptRange::Initialize() {
	Bind(L"next", &Next);
	Bind(L"toString", &ToString);
	Bind(L"isInside", &IsInside);
	Bind(L"size", &Size);
	Bind(L"from", &From);
	Bind(L"to", &To);
}

ref<Scriptable> ScriptRange::Size(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_b-_a+1));
}

ref<Scriptable> ScriptRange::From(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_a));
}

ref<Scriptable> ScriptRange::To(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_b));
}

ref<Scriptable> ScriptRange::IsInside(ref<ParameterList> p) {
	static const Parameter<int> PValue(L"value", 0);

	int value = PValue.Require(p,0);
	return GC::Hold(new ScriptBool(value>=_a && value <= _b));
}

ref<Scriptable> ScriptRange::ToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[ScriptRange]"));
}

ref<Scriptable> ScriptRange::Next(ref<ParameterList> p) {
	int ret = _a;

	if(_a>_b) {
		return ScriptConstants::Null;
	}

	_a++;
	return GC::Hold(new ScriptInt(ret));
}