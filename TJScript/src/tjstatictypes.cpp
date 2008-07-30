#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

template<class T> struct ScriptTypeRegistration {
	inline ScriptTypeRegistration(const std::wstring& n) {
		ScriptContext::AddStaticType(n, GC::Hold(new T()));
	}
};

// register static types here
ScriptTypeRegistration<ScriptMapType> mapRegistration(L"Map");
ScriptTypeRegistration<ScriptRangeType> rangeRegistration(L"Range");
ScriptTypeRegistration<ScriptHashType> hashRegistration(L"Hash");
ScriptTypeRegistration<ScriptMathType> mathRegistration(L"Math");
ScriptTypeRegistration<ScriptArrayType> arrayRegistration(L"Array");