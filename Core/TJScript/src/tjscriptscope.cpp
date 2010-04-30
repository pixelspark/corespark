/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
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
	static const Parameter<std::wstring> PVar(L"var",0);

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
	else if(command==L"value") {
		ref<Scriptable> dlg = params->Get(L"0");
		if(!dlg) {
			throw ScriptException(L"First argument to value(..) has to be set");
		}

		if(dlg.IsCastableTo<ScriptFuture>()) {
			ref<ScriptFuture> sf = dlg;
			if(sf) {
				if(sf->WaitForCompletion()) {
					if(sf->DidFail()) {
						throw ScriptException(L"A future has failed (i.e. an exception was thrown during the execution of the delegate)");
					}
					return sf->GetReturnValue();
				}
			}
			return ScriptConstants::Null;
		}
		return dlg;
	}
	else if(command==L"future") {
		ref<ScriptDelegate> dlg = params->Get(L"0");
		if(!dlg) {
			throw ScriptException(L"First argument to future(..) has to be a delegate");
		}

		ref<ScriptFuture> sft = GC::Hold(new ScriptFuture(dlg->GetScript(), dlg->GetContext()));

		// Add variables (possibly as dependencies)
		std::map<std::wstring, ref<Scriptable> >::iterator it = params->_vars.begin();
		while(it!=params->_vars.end()) {
			ref<Scriptable> value = it->second;
			if(value && it->first!=L"0") {
				if(value.IsCastableTo<ScriptFuture>()) {
					sft->AddDependency(it->first, ref<ScriptFuture>(value));
				}
				else {
					sft->AddVariable(it->first, it->second);
				}
			}
			++it;
		}

		// Dispatch this future
		strong<Dispatcher> disp = dlg->GetContext()->GetDispatcher();
		disp->Dispatch(ref<Task>(sft));
		return sft;
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
