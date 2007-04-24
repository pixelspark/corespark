#ifndef _TJSCRIPTOBJECT_H
#define _TJSCRIPTOBJECT_H

namespace tj {
	namespace script {
		template<typename T> class ScriptObjectMemberIterator;

		/** This object provides late-bound methods and reflection. It implements Scriptable **/
		template<typename T> class ScriptObject: public virtual tj::shared::Object, public Scriptable {
			friend class ScriptObjectMemberIterator<T>;

			protected:
				typedef tj::shared::ref<Scriptable> (T::*Member)(tj::shared::ref<ParameterList>);
				typedef std::map<CommandType, Member> MemberMap;

				ScriptObject();
				static void Bind(Command c, Member p);

				static typename MemberMap _members;
				static volatile long _initialized;

			public:
				// Scriptable
				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p);
				virtual ~ScriptObject();
		};

		template<typename T> class ScriptObjectMemberIterator: public Scriptable {
			public:
				ScriptObjectMemberIterator() {
					_it = T::_members.begin();
				}

				virtual ~ScriptObjectMemberIterator() {
				}

				virtual tj::shared::ref<Scriptable> Execute(Command c, tj::shared::ref<ParameterList> p) {
					if(c==L"next") {
						if(_it!=T::_members.end()) {
							Command memberName = _it->first;
							++_it;
							return tj::shared::GC::Hold(new ScriptString(memberName));
						}
						return ScriptConstants::Null;
					}
					else if(c==L"toString") {
						return tj::shared::GC::Hold(new ScriptString(L"[ScriptObjectMemberIterator]"));
					}
					return 0;
				}

			protected:
				typename ScriptObject<T>::MemberMap::const_iterator _it;
		};

		/* Implementation */
		template<typename T> ScriptObject<T>::ScriptObject() {
			/** InterlockedExchange is the thread-safe way to set the lock to 1
			and return the previous value. If the previous value was 0, initialize
			the object **/
			long lockVal = InterlockedExchange(&_initialized, 1);
			if(lockVal==0L) {
				T::Initialize();
			}
		}

		template<typename T> ScriptObject<T>::~ScriptObject() {
		}

		template<typename T> void ScriptObject<T>::Bind(Command c, Member p) {
			ScriptObject<T>::_members[c] = p;
		}

		template<typename T> tj::shared::ref<Scriptable> ScriptObject<T>::Execute(Command c, tj::shared::ref<ParameterList> p) {
			MemberMap::const_iterator it = _members.find(c);
			if(it!=_members.end()) {
				Member m = it->second;
				return (static_cast<T*>(this)->*m)(p);
			}
			else if(c==L"members") {
				return tj::shared::GC::Hold(new ScriptObjectMemberIterator<T>());
			}
			else if(c==L"class") {
				return tj::shared::GC::Hold(new ScriptString(Wcs(typeid(T).name())));
			}
			else {
				return 0;
			}
		}

		template<typename T> typename ScriptObject<T>::MemberMap ScriptObject<T>::_members = typename ScriptObject<T>::MemberMap();
		template<typename T> volatile long ScriptObject<T>::_initialized = 0;
	}
}

#endif