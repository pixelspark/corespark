#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

// register static types here
ScriptTypeRegistration<ScriptMapType> mapRegistration(L"Map");
ScriptTypeRegistration<ScriptRangeType> rangeRegistration(L"Range");
ScriptTypeRegistration<ScriptHashType> hashRegistration(L"Hash");
ScriptTypeRegistration<ScriptMathType> mathRegistration(L"Math");
ScriptTypeRegistration<ScriptArrayType> arrayRegistration(L"Array");