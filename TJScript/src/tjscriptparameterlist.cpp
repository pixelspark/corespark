#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ref<Scriptable> ScriptParameterList::Execute(Command command, tj::shared::ref<ParameterList> params) {
	if(command==L"toString") {
		std::wostringstream wos;

		ParameterList::iterator it = _params->begin();
		while(it!=_params->end()) {
			std::pair< std::wstring, tj::shared::ref<Scriptable> > p = *it;
			wos << p.first << L"; ";
			++it;
		}

		return tj::shared::GC::Hold(new ScriptValue<std::wstring>(wos.str()));
	}
	return 0;
}

ScriptParameterList::ScriptParameterList() {
	_params = GC::Hold(new ParameterList());
	_namelessCount = 0;
}

ScriptParameterList::~ScriptParameterList() {
}

void ScriptParameterList::Set(std::wstring k, ref<Scriptable> v) {
	_params->operator[](k) = v;
}

void ScriptParameterList::AddNamelessParameter(ref<Scriptable> t) {
	Set(Stringify(_namelessCount), t);
	_namelessCount++;
}