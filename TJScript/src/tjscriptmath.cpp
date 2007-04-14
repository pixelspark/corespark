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

void ScriptMath::Initialize() {
	Bind(L"sin", &Sin);
	Bind(L"cos", &Cos);
	Bind(L"tan", &Tan);
	Bind(L"atan", &Atan);
	Bind(L"atan2", &Atan2);
	Bind(L"acos", &Acos);
	Bind(L"asin", &Asin);
	Bind(L"random", &Random);
	Bind(L"pi", &Pi);
	Bind(L"pow", &Pow);
	Bind(L"fmod", &Fmod);
}

ref<Scriptable> ScriptMath::Sin(ref<ParameterList> p) {
	RequiredParameter<double> angle(p, L"angle", 0.0, 0);
	return GC::Hold(new ScriptDouble(sin(angle)));
}


ref<Scriptable> ScriptMath::Cos(ref<ParameterList> p) {
	RequiredParameter<double> angle(p, L"angle", 0.0, 0);
	return GC::Hold(new ScriptDouble(cos(angle)));
}


ref<Scriptable> ScriptMath::Tan(ref<ParameterList> p) {
	RequiredParameter<double> angle(p, L"angle", 0.0, 0);
	return GC::Hold(new ScriptDouble(tan(angle)));
}


ref<Scriptable> ScriptMath::Atan2(ref<ParameterList> p) {
	RequiredParameter<double> x(p, L"x", 0.0, 0);
	RequiredParameter<double> y(p, L"y", 0.0, 1);
	return GC::Hold(new ScriptDouble(atan2(y,x)));
}


ref<Scriptable> ScriptMath::Atan(ref<ParameterList> p) {
	RequiredParameter<double> angle(p, L"angle", 0.0, 0);
	return GC::Hold(new ScriptDouble(atan(angle)));
}

ref<Scriptable> ScriptMath::Random(ref<ParameterList> p) {
	RequiredParameter<int> from(p,L"from", 0,0);
	RequiredParameter<int> to(p, L"to", 0,1);

	if(to.Get()-from.Get()<=0) {
		throw ScriptException(L"Invalid argument values for from and to");
	}

	int r = (rand()%(to-from+1))+from;
	return GC::Hold(new ScriptInt(r));
}

ref<Scriptable> ScriptMath::Acos(ref<ParameterList> p) {
	RequiredParameter<double> angle(p, L"angle", 0.0, 0);
	return GC::Hold(new ScriptDouble(acos(angle)));
}

ref<Scriptable> ScriptMath::Asin(ref<ParameterList> p) {
	RequiredParameter<double> angle(p, L"angle", 0.0, 0);
	return GC::Hold(new ScriptDouble(asin(angle)));
}

ref<Scriptable> ScriptMath::Fmod(ref<ParameterList> p) {
	RequiredParameter<double> a(p, L"a", 0.0, 0);
	RequiredParameter<double> b(p, L"b", 0.0, 1);
	return GC::Hold(new ScriptDouble(fmod(a,b)));
}

ref<Scriptable> ScriptMath::Pow(ref<ParameterList> p) {
	RequiredParameter<double> a(p, L"a", 0.0, 0);
	RequiredParameter<double> b(p, L"b", 0.0, 1);
	return GC::Hold(new ScriptDouble(pow(a,b)));
}

ref<Scriptable> ScriptMath::Pi(ref<ParameterList> p) {
	return GC::Hold(new ScriptDouble(PI));
}