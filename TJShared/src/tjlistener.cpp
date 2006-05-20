#include "../include/tjshared.h"

ListenerWrapper::ListenerWrapper(Listener* lp) {
	assert(_listener!=0);
	_listener = lp;
}

ListenerWrapper::~ListenerWrapper() {
}

void ListenerWrapper::Notify(Wnd* source, Notification evt) {
	_listener->Notify(source,evt);
}