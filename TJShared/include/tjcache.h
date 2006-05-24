#ifndef _TJCACHE_H
#define _TJCACHE_H

template<typename T> struct CacheItem {
	CacheItem(T value=0, int age=0) {
		_value = value;
		_age = age;
	}
	T _value;
	int _age;
};

template<typename K, typename V> class Cache {
	public:
		Cache(unsigned int maxItems) {
			_max = maxItems;
		}

		virtual ~Cache() {
		}

		int GetSize() {
			return _items.size();
		}

		V Get(K key) {
			std::map<K,CacheItem<V> >::iterator it = _items.find(key);
			if(it==_items.end()) return 0;
			return (*it).second._value;
		}

		void Put(K key, V value) {
			if(_items.size()+1 > _max) {
				int maxAge = (int)time(0);
				std::map<K,CacheItem<V> >::iterator maxItem = _items.end();

				// reached maximum item count, delete last item
				std::map<K,CacheItem<V> >::iterator it = _items.begin();
				while(it!=_items.end()) {
					CacheItem<V>& item = (*it).second;
					if(item._age < maxAge) {
						maxAge = item._age;
						maxItem = it;
					}
					it++;
				}

				if(maxItem != _items.end()) _items.erase(maxItem);
			}
			_items[key] = CacheItem<V>(value, (int)time(0));
		}
	protected:
		unsigned int _max;
		std::map<K,CacheItem<V> > _items; 
};

#endif