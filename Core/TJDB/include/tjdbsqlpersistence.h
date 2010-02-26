#ifndef _TJ_DB_SQLPERSISTENCE_H
#define _TJ_DB_SQLPERSISTENCE_H

#include "internal/tjdbinternal.h"
#include "tjdbpersistence.h"

namespace tj {
	namespace db {
		using namespace tj::shared;

		class DB_EXPORTED SQLEntityContext: public EntityContext {
			friend class SQLUpdateEntityTask;
			
			public:
				SQLEntityContext(strong<Database> db, bool readOnly = false, bool asynchronousUpdates = true);
				virtual ~SQLEntityContext();
				virtual void UpdateDatabaseSchema(const Schema& s);
				virtual ref<Entity> Materialize(const Schema& s, const ID& i);
				virtual ID Add(strong<Entity> so);
				virtual void Remove(const ID& i, const Schema& schema);
				virtual void Update(const ID& i, strong<Entity> et);
				virtual int64 GetCollectionSize(const CollectionType& ct, const ID& i);
				virtual ID GetCollectionItem(const CollectionType& ct, const Schema& referenced, const ID& i, int64 idx);
				virtual void RemoveFromCollection(const CollectionType& ct, const Schema& referenced, const ID& i, const ID& toid);
				virtual void AddToCollection(const CollectionType& ct, const ID& from, const ID& toid);

			protected:
				virtual void Remove(const ID& i, const Schema& schema, bool transaction);
				virtual ID Add(strong<Entity> so, const Schema& s, const EntityType& subtype);
				virtual void Update(const ID& i, strong<Entity> et, bool async);
				virtual void Update(const ID& i, strong<Entity> et, const Schema& schema);

				virtual ref<Query> RetrieveData(const Schema& schema, const ID& i);
				virtual void MaterializeWithData(ref<Query> q, ref<Entity> e, const Schema& schema, const ID& i);
				virtual CollectionType GetCollectionType(const Schema& from, const Schema& to, const Key& k);

				std::set<EntityType> _goodSchemas;
				strong<Database> _db;
				bool _readOnly;
				bool _async;
		};
	}
}

#endif