#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

template<class T> struct ScriptTypeRegistration {
	inline ScriptTypeRegistration(std::wstring n) {
		ScriptContext::AddStaticType(n, GC::Hold(new T()));
	}
};

// register static types here
ScriptTypeRegistration<ScriptArrayType> arrayRegistration(L"Array");
ScriptTypeRegistration<ScriptRangeType> rangeRegistration(L"Range");