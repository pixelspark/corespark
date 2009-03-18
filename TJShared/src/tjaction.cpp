#include "../include/tjcore.h"
using namespace tj::shared;

Action::Action(const String& name, UndoSupport sup): _name(name), _sup(sup) {
}

Action::Action(UndoSupport sup): _sup(sup) {
}

Action::~Action() {
}

void Action::Undo() {
}

Action::UndoSupport Action::GetUndoSupport() const {
	return _sup;
}

String Action::GetName() const {
	return _name;
}