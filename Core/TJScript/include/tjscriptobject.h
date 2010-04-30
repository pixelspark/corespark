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
 
 #ifndef _TJSCRIPTOBJECT_H
#define _TJSCRIPTOBJECT_H

#include <typeinfo>

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

				static typename ScriptObject<T>::MemberMap _members;
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
			long lockVal = tj::shared::Atomic::Exchange(&_initialized, 1);
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
			typename MemberMap::const_iterator it = _members.find(c);
			if(it!=_members.end()) {
				Member m = it->second;
				return (static_cast<T*>(this)->*m)(p);
			}
			else if(c==L"members") {
				return tj::shared::GC::Hold(new ScriptObjectMemberIterator<T>());
			}
			else if(c==L"class") {
				return tj::shared::GC::Hold(new ScriptString(tj::shared::Wcs(typeid(T).name())));
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