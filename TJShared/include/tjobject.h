#ifndef _TJOBJECT_H
#define _TJOBJECT_H

namespace tj {
	namespace shared {
		/**  All classes that want Object functionality should inherit from it virtually:

		class X: public virtual Object, public OtherClass, public virtual SomeInterface {..};

		Without the virtual keyword, duplicate base instances of Object could be instantiated, which is not
		guaranteed to work. Object should be listed first in the list of extended classes.

		**/
		class EXPORTED Object {
			friend class GC;

			public:
				inline Object(): _resource(0) {
				}

				struct EXPORTED Notification {}; // useful for empty notifications; then just use Type::Notification

				// To make it a polymorphic type
				virtual ~Object() {
				}

				virtual void OnCreated();

				intern::Resource* _resource;
		};
	}
}

#endif