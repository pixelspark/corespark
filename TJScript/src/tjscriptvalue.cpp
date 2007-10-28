#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ScriptNull::ScriptNull() {
}

ScriptNull::~ScriptNull() {
}

ref<Scriptable> ScriptNull::Execute(Command c, ref<ParameterList> p) {
	if(c==L"toString") {
		return GC::Hold(new ScriptString(L"null"));
	}
	return 0;
}

template<> ref<Scriptable> ScriptValue<std::wstring>::TypeSpecificExecute(Command command, ref<ParameterList> params) {
	if(command==L"length") {
		return GC::Hold(new ScriptValue<int>((int)_value.length()));
	}
	else if(command==L"toUpper") {
		std::wstring temp = _value;
		std::transform(temp.begin(), temp.end(), temp.begin(), toupper);
		return GC::Hold(new ScriptValue<std::wstring>(temp));
	}
	else if(command==L"toLower") {
		std::wstring temp = _value;
		std::transform(temp.begin(), temp.end(), temp.begin(), tolower);
		return GC::Hold(new ScriptValue<std::wstring>(temp));
	}
	else if(command==L"explode") {
		static const Parameter<std::wstring> PSeparator(L"separator", 0);

		std::wstring separator = PSeparator.Require(params, L"");
		ref<ScriptArray> result = GC::Hold(new ScriptArray());
		std::wstring::size_type start = 0;
		std::wstring::size_type end = 0;

		while((end = _value.find(separator, start)) != std::wstring::npos) {
			result->Push(GC::Hold(new ScriptString(_value.substr (start, end-start))));
			start = end + separator.size();
		}
		result->Push(GC::Hold(new ScriptString(_value.substr(start))));
	
		return result;
	}
	return 0;
}