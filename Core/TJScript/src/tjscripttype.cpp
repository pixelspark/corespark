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

ScriptType::~ScriptType() {
}

ref<Scriptable> ScriptType::Execute(Command c, ref<ParameterList> p) {
	return 0;
}

/** ScriptPackage **/
// DefaultInstance is defined in tjstatictypes.cpp
ScriptPackage::~ScriptPackage() {
}

ScriptPackage::ScriptPackage() {
}

void ScriptPackage::AddType(const std::wstring& identifier, strong<ScriptType> st) {
	ThreadLock lock(&_lock);
	_types[identifier] = st;
}

ref<ScriptType> ScriptPackage::GetType(const std::wstring& type) {
	ThreadLock lock(&_lock);
	std::map<std::wstring, ref<ScriptType> >::iterator it = _types.find(type);
	if(it!=_types.end()) {
		return it->second;
	}
	return null;
}