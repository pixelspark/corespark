#include "../include/internal/tjscript.h"
#include <math.h>
using namespace tj::script;
using namespace tj::shared;

const static double PI = 3.14159265358979323846;

ref<Scriptable> ScriptMathType::Construct(ref<ParameterList> p) {
	return GC::Hold(new ScriptMath());
}

ScriptMathType::~ScriptMathType() {
}

ScriptMath::~ScriptMath() {
}

ScriptMath::ScriptMath() {
}

ref<Scriptable> ScriptMath::Execute(Command c, ref<ParameterList> p) {
	if(c==L"random") {
		RequiredParameter<int> from(p,L"from", 0,0);
		RequiredParameter<int> to(p, L"to", 0,1);

		if(to.Get()-from.Get()<=0) {
			throw ScriptException(L"Invalid argument values for from and to");
		}

		int r = (rand()%(to-from+1))+from;
		return GC::Hold(new ScriptInt(r));
	}
	else if(c==L"sin") {
		RequiredParameter<double> angle(p, L"angle", 0.0, 0);
		return GC::Hold(new ScriptDouble(sin(angle)));
	}
	else if(c==L"cos") {
		RequiredParameter<double> angle(p, L"angle", 0.0, 0);
		return GC::Hold(new ScriptDouble(cos(angle)));
	}
	else if(c==L"tan") {
		RequiredParameter<double> angle(p, L"angle", 0.0, 0);
		return GC::Hold(new ScriptDouble(tan(angle)));
	}
	else if(c==L"atan2") {
		RequiredParameter<double> x(p, L"x", 0.0, 0);
		RequiredParameter<double> y(p, L"y", 0.0, 1);
		return GC::Hold(new ScriptDouble(atan2(y,x)));
	}
	else if(c==L"atan") {
		RequiredParameter<double> angle(p, L"angle", 0.0, 0);
		return GC::Hold(new ScriptDouble(atan(angle)));
	}
	else if(c==L"acos") {
		RequiredParameter<double> angle(p, L"angle", 0.0, 0);
		return GC::Hold(new ScriptDouble(acos(angle)));
	}
	else if(c==L"asin") {
		RequiredParameter<double> angle(p, L"angle", 0.0, 0);
		return GC::Hold(new ScriptDouble(asin(angle)));
	}
	else if(c==L"fmod") {
		RequiredParameter<double> a(p, L"a", 0.0, 0);
		RequiredParameter<double> b(p, L"b", 0.0, 1);
		return GC::Hold(new ScriptDouble(fmod(a,b)));
	}
	else if(c==L"pow") {
		RequiredParameter<double> a(p, L"a", 0.0, 0);
		RequiredParameter<double> b(p, L"b", 0.0, 1);
		return GC::Hold(new ScriptDouble(pow(a,b)));
	}
	else if(c==L"pi") {
		return GC::Hold(new ScriptDouble(PI));
	}
	else if(c==L"toString") {
		return GC::Hold(new ScriptString(L"[ScriptMath]"));
	}

	return 0;
}