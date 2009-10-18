#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

ScriptScope::ScriptScope() {
}

ScriptScope::~ScriptScope() {
}

void ScriptScope::SetPrevious(ref<Scriptable> p) {
	_previous = p;
}

ref<Scriptable> ScriptScope::GetPrevious() {
	return _previous;
}

bool ScriptScope::Exists(const std::wstring& key) {
	return _vars.find(key)!=_vars.end();
}

ref<Scriptable> ScriptScope::Get(const std::wstring& key) {
	std::map<std::wstring, ref<Scriptable> >::iterator it = _vars.find(key);
	if(it!=_vars.end()) {
		return it->second;
	}
	return 0;
}

ref<Scriptable> ScriptScope::Execute(Command command, ref<ParameterList> params) {
	static const Parameter<std::wstring> PVar(L"var");

	if(command==L"exists") {
		std::wstring name = PVar.Require(params,L"");

		bool exists = _vars.find(name)!=_vars.end();
		if(!exists) {
			if(_previous) {
				ref<Scriptable> ret = _previous->Execute(command, params);
				if(!ret || !ret.IsCastableTo<ScriptBool>()) {
					return ScriptConstants::False;
				}
				return ret;
			}
			else {
				return ScriptConstants::False;
			}
		}
		else {
			return ScriptConstants::True;
		}
	}
	else {
		ref<Scriptable> val = Get(command);
		if(val) {
			return val;
		}
		else if(_previous) {
			return _previous->Execute(command, params);
		}
		else {
			return ScriptConstants::Null;
		}

	}
}

bool ScriptScope::Set(Field key, ref<Scriptable> value) {
	/** If the variable is already defined in the outer scope, update it there */
	if(_previous && _previous.IsCastableTo<ScriptScope>() && _previous->Execute(key,0)) {
		ref<ScriptScope>(_previous)->Set(key,value);
	}
	else {
		_vars[key] = value;
	}
	return true;
}
