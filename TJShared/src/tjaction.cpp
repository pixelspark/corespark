#include "../include/tjshared.h"
using namespace tj::shared;

Action::Action(const std::wstring& name, UndoSupport sup): _name(name), _sup(sup) {
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

std::wstring Action::GetName() const {
	return _name;
}