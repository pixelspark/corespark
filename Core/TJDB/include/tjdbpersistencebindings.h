#ifndef _TJ_DB_PERSISTENCE_BINDINGS_H
#define _TJ_DB_PERSISTENCE_BINDINGS_H

#include "tjdbpersistence.h"

namespace tj {
	namespace db {
		template<class F, class T> class ToMany: Collection {
			public:
				ToMany(): _id(Entity::UnknownObjectID), _size(0), _iteratorVersion(0) {
				}

				virtual ~ToMany() {
					Commit();
				}

				virtual void Set(const CollectionType& ct, const ID& i, strong<EntityContext> oc) {
					ThreadLock lock(&_lock);
					_id = i;
					_type = ct;
					_ctx = oc;
					Update();
				}

				virtual void Commit() {
				}

				virtual int64 GetSize() const {
					ThreadLock lock(&_lock);
					return _size;
				}

				virtual void Update() {
					ThreadLock lock(&_lock);
					_size = _ctx->GetCollectionSize(_type, _id);
					++_iteratorVersion;
				}

				virtual void RemoveAll(rel<T> object) {
					ThreadLock lock(&_lock);
					if(IsValid()) {
						_ctx->RemoveFromCollection(_type, T::Schema(), _id, object.GetID());
					}
					Update();
				}

				virtual void Add(rel<T> object) {
					ThreadLock lock(&_lock);
					if(IsValid()) {
						object.Commit();
						ID objectID = object.GetID();
						if(objectID!=Entity::NewObjectID) {
							_ctx->AddToCollection(_type, _id, objectID);
						}
						Update();
					}
					else {
						Throw(L"Cannot add to an invalid collection (maybe commit parent object first?)", ExceptionTypeError);
					}
				}

				virtual bool IsValid() const {
					return bool(_ctx) && _id!=Entity::UnknownObjectID && _id!=Entity::NewObjectID && _id!=Entity::RemovedObjectID;
				}

				inline rel<T> operator[](const int64& i) {
					return Get(i);
				}

				virtual rel<T> Get(const int64& idx) {
					if(IsValid() && idx < _size) {
						ID obj = _ctx->GetCollectionItem(_type, T::Schema(), _id, idx);
						if(obj!=Entity::UnknownObjectID) {
							return _ctx->Get<T>(obj);
						}
					}
					return rel<T>();
				}

				mutable CriticalSection _lock;

			protected:
				CollectionType _type;
				ID _id;
				mutable int64 _iteratorVersion;
				mutable int64 _size;
				mutable ref<EntityContext> _ctx;
		};

		template<class T> class ObjectCache: public EntityCache {
			public:
				ObjectCache() {
				}

				virtual ~ObjectCache() {
				}

				virtual void Clean() {
					ThreadLock lock(&_lock);
					// Clean the cache, get rid of objects that have been deleted
					std::map<ID, weak<T> >::iterator cit = _cache.begin();
					while(cit!=_cache.end()) {
						ref<T> object = cit->second;
						if(object) {
							++cit;
						}
						else {
							_cache.erase(cit++);
						}
					}
				}

				ref<T> Get(const ID& i) {
					ThreadLock lock(&_lock);
					std::map<ID, weak<T> >::iterator cit = _cache.find(i);
					if(cit!=_cache.end()) {
						return cit->second;
					}
					return null;
				}

				void Put(const ID& i, ref<T> object) {
					ThreadLock lock(&_lock);
					_cache[i] = object;
				}

				virtual ref<Entity> GetEntity(const ID& i) {
					ref<T> object = Get(i);
					if(object) {
						return object;
					}
					return null;
				}

				virtual void PutEntity(const ID& i, ref<Entity> object) {
					if(!object.IsCastableTo<T>()) {
						Throw(L"Wrong object type for this cache!", ExceptionTypeError);
					}
					ThreadLock lock(&_lock);
					_cache[i] = ref<T>(object);
				}

				std::map< ID, weak<T> > _cache;
		};

		template<class T> class MemberBinding: public Binding {
			public:
				typedef Any T::*AnyMember;
				typedef int T::*IntMember;
				typedef String T::*StringMember;
				typedef double T::*DoubleMember;
				typedef bool T::*BoolMember;
				typedef Relation T::*RelationMember;
				typedef Collection T::*CollectionMember;

				enum Type {
					TypeNone = 0,
					TypeAny,
					TypeInt,
					TypeString,
					TypeDouble,
					TypeBool,
					TypeRelation,
					TypeCollection,
				};

				MemberBinding(): _type(TypeNone), _referenced(0) {
				}

				MemberBinding(IntMember i): _intMember(i), _type(TypeInt), _referenced(0) {
				}

				MemberBinding(RelationMember i, const Schema* referenced): _relationMember(i), _type(TypeRelation), _referenced(referenced) {
				}

				MemberBinding(CollectionMember i, const Schema* referenced): _collectionMember(i), _type(TypeCollection), _referenced(referenced) {
				}
				
				MemberBinding(AnyMember i): _anyMember(i), _type(TypeAny), _referenced(0) {
				}

				MemberBinding(StringMember i): _stringMember(i), _type(TypeString), _referenced(0) {
				}

				MemberBinding(DoubleMember i): _doubleMember(i), _type(TypeDouble), _referenced(0) {
				}

				MemberBinding(BoolMember i): _boolMember(i), _type(TypeBool), _referenced(0) {
				}

				virtual ~MemberBinding() {
				}

				virtual bool IsRelation() const {
					return _type==TypeRelation;
				}

				virtual bool IsCollection() const {
					return _type==TypeCollection;
				}

				virtual Relation& GetRelation(strong<T> object) const {
					if(_type!=TypeRelation) {
						Throw(L"Cannot get relation; this binding does not point to a relation!", ExceptionTypeError);
					}
					return ref<T>(object).GetPointer()->*_relationMember;
				}

				virtual Collection& GetCollection(strong<T> object) const {
					if(_type!=TypeCollection) {
						Throw(L"Cannot get collection; this binding does not point to a collection!", ExceptionTypeError);
					}
					return ref<T>(object).GetPointer()->*_collectionMember;
				}

				Any Get(strong<T> object) const {
					if(_type==TypeAny) {
						return ref<T>(object).GetPointer()->*_anyMember;
					}
					else if(_type==TypeInt) {
						return Any(ref<T>(object).GetPointer()->*_intMember);
					}
					else if(_type==TypeString) {
						return Any(ref<T>(object).GetPointer()->*_stringMember);
					}
					else if(_type==TypeDouble) {
						return Any(ref<T>(object).GetPointer()->*_doubleMember);
					}
					else if(_type==TypeBool) {
						return Any(ref<T>(object).GetPointer()->*_boolMember);
					}
					else {
						Throw(L"Get binding failed; invalid type", ExceptionTypeError);
					}
				}

				void Set(strong<T> object, const Any& v) const {
					if(_type==TypeAny) {
						ref<T>(object).GetPointer()->*_anyMember = v;
					}
					else if(_type==TypeInt) {
						ref<T>(object).GetPointer()->*_intMember = int(v);
					}
					else if(_type==TypeString) {
						ref<T>(object).GetPointer()->*_stringMember = String(v);
					}
					else if(_type==TypeDouble) {
						ref<T>(object).GetPointer()->*_doubleMember = double(v);
					}
					else if(_type==TypeBool) {
						ref<T>(object).GetPointer()->*_boolMember = bool(v);
					}
					else {
						Throw(L"Set binding failed; invalid type", ExceptionTypeError);
					}
				}

				virtual Relation& GetRelation(strong<Entity> object) const {
					ref<Entity> entity = object;
					if(entity.IsCastableTo<T>()) {
						return GetRelation(ref<T>(entity));
					}
					Throw(L"Invalid object type in MemberBinding::GetRelation!", ExceptionTypeError);
				}

				virtual Collection& GetCollection(strong<Entity> object) const {
					ref<Entity> entity = object;
					if(entity.IsCastableTo<T>()) {
						return GetCollection(ref<T>(entity));
					}
					Throw(L"Invalid object type in MemberBinding::GetCollection!", ExceptionTypeError);
				}


				virtual const Schema* GetReferencedSchema() const {
					return _referenced;
				}

				virtual Any Get(strong<Entity> object) const {
					ref<Entity> entity = object;
					if(entity.IsCastableTo<T>()) {
						return Get(ref<T>(entity));
					}
					Throw(L"Invalid object type in MemberBinding::Get!", ExceptionTypeError);
				}

				virtual void Set(strong<Entity> object, const Any& v) const {
					ref<Entity> entity = object;
					if(entity.IsCastableTo<T>()) {
						Set(ref<T>(entity), v);
					}
					else {
						Throw(L"Invalid object type!", ExceptionTypeError);
					}
				}

			protected:
				union {
					AnyMember _anyMember;
					IntMember _intMember;
					StringMember _stringMember;
					DoubleMember _doubleMember;
					BoolMember _boolMember;
					RelationMember _relationMember;
					CollectionMember _collectionMember;
				};
				Type _type;
				const Schema* _referenced;
		};

		template<class T> class ObjectSchema: public Schema {
			public:
				ObjectSchema() {
				}

				virtual ~ObjectSchema() {
				}

				virtual ref<Entity> CreateEntity() const {
					return GC::Hold(new T());
				}

				void Bind(const Key& k, const MemberBinding<T>& m) {
					if(k==L"id" || k==L"super" || k==L"subtype") {
						Throw(L"Cannot bind with this key; is reserved column name", ExceptionTypeError);
					}
					_members[k] = m;
					_keys.insert(k);
				}

				void SetEntityType(const EntityType& ot) {
					Schema::_type = ot;
				}

				virtual const std::set<Key>& GetKeys() const {
					return _keys;
				}

				virtual Binding& GetBindingForKey(const Key& k) const {
					std::map<Key,MemberBinding<T> >::iterator it = _members.find(k);
					if(it!=_members.end()) {
						return it->second;
					}
					Throw(L"ObjectSchema::GetBindingForKey: invalid key, does not exist in schema!", ExceptionTypeError);
				}

				virtual strong<EntityCache> GetCache(strong<EntityContext> ctx) const {
					// Get or create a cache for this type in the context
					ref< ObjectCache<T> > cache;
					{
						ThreadLock cachesLock(&(ctx->_cacheLock));
						cache = ctx->GetCacheForEntityType(Persistable<T>::Schema().GetEntityType());
						if(!cache) {
							cache = GC::Hold(new ObjectCache<T>());
							ctx->SetCacheForEntityType(Persistable<T>::Schema().GetEntityType(), ref<EntityCache>(cache));
						}
					}
					return cache;
				}

			protected:
				std::set<Key> _keys;
				mutable std::map<Key, MemberBinding<T> > _members;
		};

		template<class T> class Persistable: public virtual Entity {
			public:	
				typedef Relation T::*RelationMember;
				typedef Collection T::*CollectionMember;

				virtual ~Persistable() {
				}

				virtual const Schema& GetSchema() {
					return Persistable<T>::Schema();
				}

				static void Bind(const Key& k, RelationMember r, const Schema* referenced) {
					_schema.Bind(k, MemberBinding<T>(r, referenced));
				}

				static void Bind(const Key& k, CollectionMember r, const Schema* referenced) {
					_schema.Bind(k, MemberBinding<T>(r, referenced));
				}

				static void Bind(const Key& k, const MemberBinding<T>& m) {
					_schema.Bind(k,m);
				}

				static void SetEntityType(const EntityType& ot) {
					_schema.SetEntityType(ot);
				}

				static const Schema& Schema() {
					if(!_initialized) {
						_initialized = true;
						T::Initialize();
						EntityContext::RegisterSchema(_schema.GetEntityType(), &_schema);
					}
					return _schema;
				}

			protected:
				Persistable() {
				}

				static volatile bool _initialized;
				static ObjectSchema<T> _schema;
		};

		template<class T, class S> class PersistableSubclass: public T {
			public:
				virtual ~PersistableSubclass() {
				}

				static void Bind(const Key& k, const MemberBinding<S>& m) {
					_schema.Bind(k,m);
				}

				static const tj::db::Schema& Schema() {
					if(!_initialized) {
						const tj::db::Schema& superSchema = T::Schema();
						_schema.SetSuperSchema(&(T::_schema));
						S::Initialize();
						EntityContext::RegisterSchema(_schema.GetEntityType(), &_schema);
						_initialized = true;
					}
					return _schema;
				}

				static void SetEntityType(const EntityType& ot) {
					_schema.SetEntityType(ot);
				}

				virtual const tj::db::Schema& GetSchema() {
					return PersistableSubclass<T,S>::Schema();
				}

			protected:
				PersistableSubclass() {
				}

				static volatile bool _initialized;
				static ObjectSchema<S> _schema;
		};

		template<class T> volatile bool Persistable<T>::_initialized = false;
		template<class T> ObjectSchema<T> Persistable<T>::_schema;
		template<class T, class S> volatile bool PersistableSubclass<T,S>::_initialized = false;
		template<class T, class S> ObjectSchema<S> PersistableSubclass<T,S>::_schema;
	}
}

#endif