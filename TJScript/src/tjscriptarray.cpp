#include "../include/internal/tjscript.h"
using namespace tj::shared;
using namespace tj::script;

namespace tj {
	namespace script {
		class ScriptArrayIterator: public Scriptable {
			public:
				ScriptArrayIterator(ref<ScriptArray> x, bool iterateKey) {
					_array = x;
					_it = _array->_array.begin();
					_key = iterateKey;
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
							if(_key) {
								std::wstring key = (*_it).first;
								++_it;
								return GC::Hold(new ScriptString(key));
							}
							else {
								ref<Scriptable> val = (*_it).second;
								++_it;
								return val;
							}
						}
						return ScriptConstants::Null; // no more elements
					}
					return 0;
				};

				ref<ScriptArray> _array;
				std::map< std::wstring, ref<Scriptable> >::iterator _it;
				bool _key;
		};
	}
}

ScriptArrayType::~ScriptArrayType() {
}

ref<Scriptable> ScriptArrayType::Construct(ref<ParameterList> p) {
	ref<ScriptArray> ar = GC::Hold(new ScriptArray());
	if(p) {
		ar->Set(p);
	}

	return ar;
}

ScriptArray::ScriptArray() {
}

ScriptArray::~ScriptArray() {
}

std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator ScriptArray::GetBegin() {
	return _array.begin();
}

std::map<std::wstring, tj::shared::ref<Scriptable> >::iterator ScriptArray::GetEnd() {
	return _array.end();
}

ref<Scriptable> ScriptArray::Execute(Command c, ref<ParameterList> p) {
	ThreadLock lock(&_lock);

	if(c==L"size") {
		return GC::Hold(new ScriptInt((int)_array.size()));
	}
	else if(c==L"set") {
		if(p) {
			Set(p);
		}
		return ScriptConstants::Null;
	}
	else if(c==L"get") {
		RequiredParameter<std::wstring> key(p, L"key", L"");
		if(_array.find(key.Get())!=_array.end()) {
			return _array[key.Get()];
		}

		return ScriptConstants::Null;
	}
	else if(c==L"toString") {
		return GC::Hold(new ScriptString(L"[ScriptArray]"));
	}
	else if(c==L"keys") {
		return GC::Hold(new ScriptArrayIterator(This<ScriptArray>(), true));
	}
	else if(c==L"values") {
		return GC::Hold(new ScriptArrayIterator(This<ScriptArray>(), false));
	}
	else if(_array.find(c)!=_array.end()) {
		return _array[c];
	}
	else {
		return ScriptConstants::Null;
	}
}

void ScriptArray::Set(ref<ParameterList> p) {
	ParameterList::iterator it = p->begin();
	while(it!=p->end()) {
		std::pair< const std::wstring, ref<Scriptable> > data = *it;
		_array[data.first] = data.second;
		++it;
	}
}