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

ref<Scriptable> ScriptRangeType::Construct(tj::shared::ref<ParameterList> p) {
	static const Parameter<int> PFrom(L"from",0);
	static const Parameter<int> PTo(L"to",1);	

	int from = PFrom.Require(p,0);
	int to = PTo.Require(p,0);

	if(to<from) {
		return 0;
	}

	return GC::Hold(new ScriptRange(from,to));
}

ScriptRangeType::~ScriptRangeType() {
}

ScriptRange::~ScriptRange() {
}

ScriptRange::ScriptRange(int a, int b): _a(a), _b(b) {
}

void ScriptRange::Initialize() {
	Bind(L"next", &ScriptRange::Next);
	Bind(L"toString", &ScriptRange::ToString);
	Bind(L"isInside", &ScriptRange::IsInside);
	Bind(L"size", &ScriptRange::Size);
	Bind(L"from", &ScriptRange::From);
	Bind(L"to", &ScriptRange::To);
}

ref<Scriptable> ScriptRange::Size(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_b-_a+1));
}

ref<Scriptable> ScriptRange::From(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_a));
}

ref<Scriptable> ScriptRange::To(ref<ParameterList> p) {
	return GC::Hold(new ScriptInt(_b));
}

ref<Scriptable> ScriptRange::IsInside(ref<ParameterList> p) {
	static const Parameter<int> PValue(L"value", 0);

	int value = PValue.Require(p,0);
	return GC::Hold(new ScriptBool(value>=_a && value <= _b));
}

ref<Scriptable> ScriptRange::ToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[ScriptRange]"));
}

ref<Scriptable> ScriptRange::Next(ref<ParameterList> p) {
	int ret = _a;

	if(_a>_b) {
		return ScriptConstants::Null;
	}

	_a++;
	return GC::Hold(new ScriptInt(ret));
}