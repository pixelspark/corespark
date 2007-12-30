#include "../include/internal/tjscript.h"
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
	Bind(L"toString", &SToString);
	Bind(L"push", &SPush);
	Bind(L"pop", &SPop);
	Bind(L"get", &SGet);
	Bind(L"count", &SCount);
	Bind(L"implode", &SImplode);
	Bind(L"values", &SValues);
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