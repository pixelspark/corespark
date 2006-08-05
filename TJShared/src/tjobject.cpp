#include "../include/tjshared.h"
using namespace tj::shared;

/** Some notes on Object::This<T>:

- This<T> is only guaranteed to work when T is the same T with which GC::Hold<T> is called. Other 
  combinations could give problems when using strange inheritance types.

- This<T> returns a null reference when called from inside a constructor, because the resource pointer
  is set after construction.

- This<T> should not be used in a destructor, because it creates a reference to an object that's being
  deleted. When the created reference goes out of scope, the object is deleted twice, which doesn't
  really work ;-)

All classes that want Object functionality should inherit from it virtually:

class X: public virtual Object, public OtherClass, public virtual SomeInterface {..};

Without the virtual keyword, duplicate base instances of Object could be instantiated, which is not
guaranteed to work. Object should be listed first in the list of extended classes.

**/