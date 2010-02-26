#ifndef _TJ_DB_PERSISTENCE_H
#define _TJ_DB_PERSISTENCE_H

#include "internal/tjdbinternal.h"
#include "tjdbdatabase.h"

namespace tj {
	namespace db {
		using namespace tj::shared;

		typedef int64 ID;
		typedef String Key;
		typedef String EntityType;
		typedef String CollectionType;

		class Entity;
		class EntityContext;
		class Binding;
		class EntityCache;

		class DB_EXPORTED Schema {
			public:
				Schema();
				virtual ~Schema();
				virtual const EntityType& GetEntityType() const;
				virtual void SetSuperSchema(const Schema* s);
				virtual bool IsSubSchema() const;
				virtual const Schema* GetSuperSchema() const;

				virtual const std::set<Key>& GetKeys() const = 0;
				virtual Binding& GetBindingForKey(const Key& k) const = 0;
				virtual strong<EntityCache> GetCache(strong<EntityContext> ctx) const = 0;
				virtual ref<Entity> CreateEntity() const = 0;

			protected:
				EntityType _type;
				const Schema* _super;
		};

		class DB_EXPORTED Entity: public virtual Object {
			public:
				virtual ~Entity();
				virtual const Schema& GetSchema() = 0;
				virtual void OnMaterialized();

				CriticalSection _lock;

				const static ID NewObjectID = -1;
				const static ID RemovedObjectID = -2;
				const static ID UnknownObjectID = -3;
		};

		class DB_EXPORTED Relation {
			public:
				virtual ~Relation();
				virtual void Set(const ID& i, strong<EntityContext> oc) = 0;
				virtual ID GetID() const = 0;
				virtual ref<EntityContext> GetContext() const = 0;
				virtual void SetContext(ref<EntityContext> ec) = 0;
				virtual void Commit() = 0;
		};

		class DB_EXPORTED Collection {
			public:
				virtual ~Collection();
				virtual void Set(const CollectionType& ct, const ID& i, strong<EntityContext> oc) = 0;
				virtual void Commit() = 0;
		};

		class DB_EXPORTED Binding {
			public:
				virtual ~Binding();
				virtual bool IsRelation() const = 0;
				virtual bool IsCollection() const = 0;
				virtual Relation& GetRelation(strong<Entity> object) const = 0;
				virtual Collection& GetCollection(strong<Entity> object) const = 0;
				virtual Any Get(strong<Entity> object) const = 0;
				virtual void Set(strong<Entity> object, const Any& v) const = 0;
				virtual const Schema* GetReferencedSchema() const = 0;
		};

		class DB_EXPORTED EntityCache {
			public:
				virtual ~EntityCache();
				virtual void Clean();
				virtual void PutEntity(const ID& i, ref<Entity> object) = 0;
				virtual ref<Entity> GetEntity(const ID& i) = 0;

				CriticalSection _lock;
		};

		template<typename T> class Stored: public Relation {
			friend class EntityContext;

			public:
				Stored(): _id(Entity::UnknownObjectID) {
				}

				Stored(ref<T> obj): _materialized(obj), _id(Entity::NewObjectID) {
				}

				template<class Q> Stored(ref< Stored<Q> > o);

				virtual ~Stored() {
					Commit();
				}

				virtual ID GetID() const {
					return _id;
				}

				virtual ref<EntityContext> GetContext() const {
					return _ctx;
				}

				virtual void SetContext(ref<EntityContext> ec) {
					_ctx = ec;
				}

				virtual void Set(const ID& i, strong<EntityContext> oc) {
					if(_id!=Entity::UnknownObjectID) {
						Throw(L"Cannot load entity reference, reference was already set!", ExceptionTypeError);
					}
					_id = i;
					_ctx = oc;
					_materialized = null;
				}

				virtual void Commit();
				virtual void Remove();
				virtual void Materialize() const;

				inline ref<T> Object() const {
					Materialize();
					return _materialized;
				}

				inline bool Equals(const ref< Stored<T> >& o) const {
					return o && (o->_id == _id);
				}

				mutable ref<T> _materialized;
				mutable ref<EntityContext> _ctx;
				ID _id;

			private:
				Stored(ref<EntityContext> ctx, const ID& i): _ctx(ctx), _id(i) {
				}
		};

		template<class T> class rel: public Relation {
			public:
				rel() {
				}

				template<class Q> rel(rel<Q>& o) {
					if(o._object) {
						_object = GC::Hold(new Stored<T>(o._object));
					}
				}

				rel(ref< Stored<T> > obj): _object(obj) {
				}

				rel(ref<T> obj): _object(GC::Hold(new Stored<T>(obj))) {
				}

				virtual ~rel() {
				}

				virtual ID GetID() const {
					return _object ? _object->GetID() : Entity::UnknownObjectID;
				}

				virtual void SetContext(ref<EntityContext> ec) {
					if(_object) {
						_object->SetContext(ec);
					}
				}

				virtual ref<EntityContext> GetContext() const {
					return _object ? _object->GetContext() : null;
				}

				virtual void Set(const ID& i, strong<EntityContext> ctx) {
					if(!_object) {
						_object = GC::Hold(new Stored<T>());
					}
					_object->Set(i, ctx);
				}

				inline T* operator->() {
					ref<T> obj = _object->Object();
					if(obj) {
						return obj.GetPointer();
					}
					throw NullPointerException();
				}

				inline const T* operator->() const {
					ref<T> obj = _object->Object();
					if(obj) {
						return obj.GetPointer();
					}
					throw NullPointerException();
				}

				inline operator bool() const {
					return bool(_object) && bool(_object->Object());
				}

				template<class X> bool IsCastableTo() const {
					ref<T> obj = _object->Object();
					if(obj) {
						return obj.IsCastableTo<X>();
					}
					return false;
				}

				inline bool operator==(const rel<T>& o) const {
					return o._object->Equals(_object);
				}

				void Commit() {
					if(_object) {
						_object->Commit();
					}
				}

				void Remove() {
					if(_object) {
						_object->Remove();
					}
				}

				ref< Stored<T> > _object;
		};

		class DB_EXPORTED EntityContext: public Object {
			public:
				virtual ~EntityContext();

				virtual ref<EntityCache> GetCacheForEntityType(const EntityType& ot);
				virtual void SetCacheForEntityType(const EntityType& ot, strong<EntityCache> oc);

				virtual void UpdateDatabaseSchema(const Schema& s) = 0;
				virtual ref<Entity> Materialize(const Schema& s, const ID& i) = 0;
				virtual ID Add(strong<Entity> so) = 0;
				virtual void Remove(const ID& i, const Schema& schema) = 0;
				virtual void Update(const ID& i, strong<Entity> et) = 0;
				virtual int64 GetCollectionSize(const CollectionType& ct, const ID& i) = 0;
				virtual void RemoveFromCollection(const CollectionType& ct, const Schema& referenced, const ID& i, const ID& toid) = 0;
				virtual ID GetCollectionItem(const CollectionType& ct, const Schema& referenced, const ID& i, int64 idx) = 0;
				virtual void AddToCollection(const CollectionType& ct, const ID& from, const ID& toid) = 0;

				template<class T> rel<T> Create() {
					const Schema& schema = T::Schema();
					UpdateDatabaseSchema(schema);
					return rel<T>(GC::Hold(new Stored<T>(ref<EntityContext>(this), Entity::NewObjectID)));
				}

				template<class T> rel<T> Get(const ID& id) {
					const Schema& schema = T::Schema();
					UpdateDatabaseSchema(schema);
					return rel<T>(GC::Hold(new Stored<T>(ref<EntityContext>(this), id)));
				}

				static void RegisterSchema(const EntityType& et, const Schema* schema);

				CriticalSection _cacheLock;

			protected:
				EntityContext();
				static std::map< EntityType, const Schema*>* _schemas;
				std::map< EntityType, ref<EntityCache> > _cache;
		};
		
		/** Stored<T> implementation **/
		template<typename T> void Stored<T>::Commit() {
			if(_id==Entity::UnknownObjectID || !_ctx) {
				// Unknown object; this reference is a null reference so do not commit anything
			}
			else if(_id==Entity::RemovedObjectID) {
				// Never commit removed objects
			}
			else if(_id==Entity::NewObjectID) {
				// Create object in context
				if(!_materialized) {
					Materialize();
				}
				_id = _ctx->Add(ref<Entity>(_materialized));
			}
			else {
				if(_materialized) {
					_ctx->Update(_id, ref<Entity>(_materialized));
				}
				else {
					// No need to save an object if we haven't even loaded and not modified yet
				}
			}
		}
		
		template<typename T> void Stored<T>::Remove() {
			if(!_materialized) {
				Materialize();
			}
			if(_materialized) {
				_ctx->Remove(_id, _materialized->GetSchema());
			}
		}
		
		template<typename T> void Stored<T>::Materialize() const {
			if(!_materialized) {
				if(_id==Entity::UnknownObjectID || !_ctx) {
					// Do nothing, null relation
				}
				else if(_id==Entity::NewObjectID) {
					_materialized = T::Schema().CreateEntity();
				}
				else if(_id==Entity::RemovedObjectID) {
					Throw(L"Cannot materialize object, it has been removed!", ExceptionTypeError);
				}
				else {
					_materialized = ref<T>(_ctx->Materialize(T::Schema(), _id));
				}
			}
		}
		
		template<typename T> template <class Q> Stored<T>::Stored(ref< Stored<Q> > o): _id(Entity::UnknownObjectID) {
			if(o) {
				if(o->_id==Entity::NewObjectID) {
					/* Need to commit, because otherwise two Stored<>'s with Entity::NewObjectID would exist for the same object,
					 leading to the same object be committed twice. */
					o->Commit();
				}
				_id = o->_id;
				_ctx = o->_ctx;
				
				if(o->_materialized) {
					if(dynamic_cast<T*>(o->_materialized.GetPointer())==0) {
						throw BadCastException();
					}
					_materialized = o->_materialized;
				}
			}
		}
	}
}

#endif