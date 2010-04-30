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
using namespace tj::script;
using namespace tj::shared;

namespace tj {
	namespace script {
		class ScriptArrayIterator: public Scriptable {
			public:
				ScriptArrayIterator(ref<ScriptArray> x) {
					_array = x;
					_it = _array->_array.begin();
				}

				virtual ~ScriptArrayIterator() {
				}

				virtual ref<Scriptable> Execute(Command c, ref<ParameterList> p) {
					ThreadLock lock(&(_array->_lock));
					if(c==L"toString") {
						return GC::Hold(new ScriptString(L"[ScriptArrayIterator]"));
					}
					else if(c==L"next") {
						if(_it!=_array->_array.end()) {
							ref<Scriptable> val = *_it;
							++_it;
							return val;
						}
						return ScriptConstants::Null; // no more elements
					}
					return 0;
				};

				ref<ScriptArray> _array;
				std::vector< ref<Scriptable> >::iterator _it;
		};
	}
}

ScriptArrayType::~ScriptArrayType() {
}

ref<Scriptable> ScriptArrayType::Construct(ref<ParameterList> p) {
	return GC::Hold(new ScriptArray());
}

ScriptArray::ScriptArray() {
}

ScriptArray::~ScriptArray() {
}

void ScriptArray::Initialize() {
	Bind(L"toString", &ScriptArray::SToString);
	Bind(L"push", &ScriptArray::SPush);
	Bind(L"pop", &ScriptArray::SPop);
	Bind(L"get", &ScriptArray::SGet);
	Bind(L"count", &ScriptArray::SCount);
	Bind(L"implode", &ScriptArray::SImplode);
	Bind(L"values", &ScriptArray::SValues);
}

ref<Scriptable> ScriptArray::SValues(ref<ParameterList> p) {
	return GC::Hold(new ScriptArrayIterator(this));
}

ref<Scriptable> ScriptArray::SToString(ref<ParameterList> p) {
	return GC::Hold(new ScriptString(L"[ScriptArray]")); // TODO stringify the whole array
}

ref<Scriptable> ScriptArray::SImplode(ref<ParameterList> p) {
	static const Parameter<std::wstring> PGlue(L"glue", 0);
	static const Parameter<std::wstring> PNestBegin(L"nestBegin", 1);
	static const Parameter<std::wstring> PNestEnd(L"nestEnd", 2);

	std::wstring glue = PGlue.Get(p, L", ");
	std::wstring nb = PNestBegin.Get(p, L"[");
	std::wstring ne = PNestEnd.Get(p, L"]");

	std::wostringstream os;
	ThreadLock lock(&_lock);
	std::vector< ref<Scriptable> >::iterator it = _array.begin();
	while(it!=_array.end()) {
		ref<Scriptable> object = *it;
		if(object) {
			if(object.IsCastableTo<ScriptArray>()) {
				ref<ScriptArray> sa = object;
				ref<ScriptString> imploded = sa->SImplode(p);
				os << nb << imploded->GetValue() << ne;
			}
			else {
				os << ScriptContext::GetValue<std::wstring>(object, L"");
			}
		}
		++it;
		if(it!=_array.end()) {
			os << glue;
		}
	}

	return GC::Hold(new ScriptString(os.str()));
}

void ScriptArray::Push(ref<Scriptable> st) {
	ThreadLock lock(&_lock);
	_array.push_back(st);
}

ref<Scriptable> ScriptArray::SPush(ref<ParameterList> p) {
	static Parameter<int> object(L"object",0);

	ref<Scriptable> sc = object.Get(p);
	if(sc) {
		ThreadLock lock(&_lock);
		_array.push_back(sc);
	}
	return ScriptConstants::Null;
}

ref<Scriptable> ScriptArray::SPop(ref<ParameterList> p) {
	if(_array.size()>0) {
		ThreadLock lock(&_lock);
		ref<Scriptable> object = *(_array.rbegin());
		_array.pop_back();
		return object;
	}
	throw ScriptException(L"Array is empty");
}

ref<Scriptable> ScriptArray::SCount(ref<ParameterList> p) {
	ThreadLock lock(&_lock);
	return GC::Hold(new ScriptInt((int)_array.size()));
}

ref<Scriptable> ScriptArray::SGet(ref<ParameterList> p) {
	const static Parameter<int> index(L"key", 0);
	ThreadLock lock(&_lock);
	ref<Scriptable> t = _array.at(index.Require(p,-1));
	return t;
}