#include "../include/internal/tjscript.h"
#include "../include/types/tjstatictypes.h"
using namespace tj::shared;
using namespace tj::script;

ref<ScriptPackage> ScriptPackage::_instance;

// register static types here
strong<ScriptPackage> ScriptPackage::DefaultInstance() {
	if(!_instance) {
		_instance = GC::Hold(new ScriptPackage());
		_instance->AddType(L"Map", ref<ScriptType>(GC::Hold(new ScriptMapType())));
		_instance->AddType(L"Range", ref<ScriptType>(GC::Hold(new ScriptRangeType())));
		_instance->AddType(L"Hash", ref<ScriptType>(GC::Hold(new ScriptHashType())));
		_instance->AddType(L"Math", ref<ScriptType>(GC::Hold(new ScriptMathType())));
		_instance->AddType(L"Array", ref<ScriptType>(GC::Hold(new ScriptArrayType())));
	}
	return _instance;
}