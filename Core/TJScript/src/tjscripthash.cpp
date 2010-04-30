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

ref<Scriptable> ScriptHashType::Construct(tj::shared::ref<ParameterList> p) {
	static const Parameter<std::wstring> PData(L"data", 0);
	return GC::Hold(new ScriptHash(PData.Require(p,L"")));
}

ScriptHashType::~ScriptHashType() {
}

ScriptHash::~ScriptHash() {
}

ScriptHash::ScriptHash(std::wstring w) {
	Hash h;
	_hash = h.Calculate(w);
}

ScriptHash::ScriptHash(int x) {
	_hash = x;
}

void ScriptHash::Initialize() {
	Bind(L"toString", &ScriptHash::ToString);
	Bind(L"toInt", &ScriptHash::ToInt);
}

ref<Scriptable> ScriptHash::ToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(Stringify(_hash)));
}

ref<Scriptable> ScriptHash::ToInt(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_hash));
}