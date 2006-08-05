#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ref<Scriptable> ScriptRangeType::Construct(tj::shared::ref<ParameterList> p) {
	if(!p) {
		return 0;
	}

	RequiredParameter<int> from(p, L"from", 0);
	RequiredParameter<int> to(p, L"to", 0);

	if(to.Get()<from.Get()) {
		return 0;
	}

	return GC::Hold(new ScriptRange(from.Get(), to.Get()));
}

ScriptRangeType::~ScriptRangeType() {
}

ScriptRange::~ScriptRange() {
}

ScriptRange::ScriptRange(int a, int b): _a(a), _b(b) {
}

ref<Scriptable> ScriptRange::Execute(Command c, ref<ParameterList> p) {
	if(c==L"toString") {
		return GC::Hold(new ScriptString(L"[ScriptRange]"));
	}
	else if(c==L"next") {
		int ret = _a;

		if(_a>_b) {
			return ScriptConstants::Null();
		}

		_a++;
		return GC::Hold(new ScriptInt(ret));
	}

	return 0;
}