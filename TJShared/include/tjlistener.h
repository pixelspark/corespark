#ifndef _TJLISTENER_H
#define _TJLISTENER_H

namespace tj {
	namespace shared {
		/** Listener **/
		template<typename NotificationType> class Listener {
			public:
				virtual ~Listener() {
				}

				virtual void Notify(ref<Object> source, const NotificationType& data) = 0;
		};

		template<typename NotificationType> class Listenable {
			public:
				inline Listenable() {
				}

				inline ~Listenable() {
				}

				inline void Fire(ref<Object> source, const NotificationType& data) {
					ThreadLock lock(&_lock);

					std::vector< weak< Listener< NotificationType > > >::iterator it = _listeners.begin();
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

				inline void AddListener(strong< Listener<NotificationType> > listener) {
					ThreadLock lock(&_lock);
					_listeners.push_back(ref <Listener<NotificationType> >(listener));
				}

			private:
				CriticalSection _lock;
				std::vector< weak< Listener<NotificationType> > > _listeners;
		};
	}
}

#endif