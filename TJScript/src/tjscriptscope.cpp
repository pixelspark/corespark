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
	return it==_vars.end()?0:it->second;
}

ref<Scriptable> ScriptScope::Execute(Command command, ref<ParameterList> params) {
	if(command==L"delete") {
		RequiredParameter<std::wstring> name(params, L"var", L"", 0);
		std::map<std::wstring, ref<Scriptable> >::iterator it = _vars.find(name);
		if(it!=_vars.end()) {
			_vars.erase(it);
		}
		else if(_previous) {
			ref<Scriptable> sc = _previous->Execute(command, params);
			if(!sc) {
				return ScriptConstants::Null;
			}
		}
		else {
			return ScriptConstants::Null;
		}
	}
	else if(command==L"exists") {
		RequiredParameter<std::wstring> name(params, L"var", L"", 0);
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
	else if(_vars.find(command)==_vars.end()) {
		if(_previous) {
			return _previous->Execute(command, params);
		}
		else {
			return ScriptConstants::Null;
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
