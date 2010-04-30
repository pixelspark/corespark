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