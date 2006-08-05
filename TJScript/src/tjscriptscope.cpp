#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ScriptScope::ScriptScope(ref<ScriptParameterList> p) {
	if(p) {
		ParameterList::iterator it = p->_params->begin();
		while(it!=p->_params->end()) {
			std::pair< const std::wstring, tj::shared::ref<Scriptable> > data = *it;
			_vars[data.first] = data.second;
			it++;
		}
	}
}

ScriptScope::~ScriptScope() {
}

void ScriptScope::SetPrevious(ref<Scriptable> p) {
	_previous = p;
}

ref<Scriptable> ScriptScope::GetPrevious() {
	return _previous;
}

ref<Scriptable> ScriptScope::Execute(Command command, ref<ParameterList> params) {
	if(_vars.find(command)==_vars.end()) {
		if(_previous) {
			return _previous->Execute(command, params);
		}
		else {
			return 0;
		}
	}
	else {
		return _vars[command];
	}
}

void ScriptScope::Set(std::wstring key, ref<Scriptable> value) {
	/** If the variable is already defined in the outer scope, update it there */
	if(_previous && _previous.IsCastableTo<ScriptScope>() && _previous->Execute(key,0)) {
		ref<ScriptScope>(_previous)->Set(key,value);
	}
	else {
		_vars[key] = value;
	}
}
