#ifndef _TJLISTENER_H
#define _TJLISTENER_H

namespace tj {
	namespace shared {
		/** Listener **/
		template<typename NotificationType> class EXPORTED Listener {
			public:
				virtual ~Listener() {
				}

				virtual void Notify(ref<Object> source, const NotificationType& data) = 0;
		};

		template<typename NotificationType> class EXPORTED Listenable {
			public:
				inline Listenable() {
				}

				inline ~Listenable() {
				}

				inline void Fire(ref<Object> source, const NotificationType& data) {
					ThreadLock lock(&_lock);

					typename std::vector< weak< Listener< NotificationType > > >::iterator it = _listeners.begin();
					while(it!=_listeners.end()) {
						ref< Listener<NotificationType> > listener = *it;
						if(listener) {
							listener->Notify(source, data);
							++it;
						}
						else {
							it = _listeners.erase(it);
						}
					}
				}

				inline bool HasListener() const {
					return _listeners.size() > 0;
				}

				inline void AddListener(strong< Listener<NotificationType> > listener) {
					ThreadLock lock(&_lock);
					_listeners.push_back(ref <Listener<NotificationType> >(listener));
				}

				inline void AddListener(ref< Listener<NotificationType> > listener) {
					ThreadLock lock(&_lock);
					if(listener) {
						_listeners.push_back(weak <Listener<NotificationType> >(listener));
					}
				}

				inline void RemoveListener(ref< Listener<NotificationType> > listener) {
					ThreadLock lock(&_lock);
					if(listener) {
						typename std::vector< weak<Listener<NotificationType> > >::iterator it = _listeners.begin();
						while(it!=_listeners.end()) {
							if(*it == listener) {
								it = _listeners.erase(it);
							}
							else {
								++it;
							}
						}
					}
				}

			private:
				CriticalSection _lock;
				std::vector< weak< Listener<NotificationType> > > _listeners;
		};
	}
}

#endif