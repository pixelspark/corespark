#include "../include/tjaction.h"
#include "../include/tjlog.h"
#include "../include/tjutil.h"
using namespace tj::shared;

/** UndoStack **/
ref<UndoStack> UndoStack::_instance;
unsigned int UndoStack::KMaxUndoChangess = 30;
ThreadLocal UndoBlock::_currentBlock;

strong<UndoStack> UndoStack::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new UndoStack());
	}
	return _instance;
}

UndoStack::UndoStack() {
}

UndoStack::~UndoStack() {
}

void UndoStack::Undo(unsigned int blocks) {
	for(unsigned int a = 0; a < blocks; a++) {
		ThreadLock lock(&_stackLock);
		std::deque<UndoChanges>::reverse_iterator it = _undoStack.rbegin();
		if(it==_undoStack.rend()) {
			break;
		}
		UndoChanges& block = *it;
		_redoStack.push_back(block);
		block.Undo();
		_undoStack.pop_back();
	}

	EventUndoChange.Fire(ref<UndoStack>(this), UndoNotification());
	Log::Write(L"TJShared/UndoStack", L"Undid "+Stringify(blocks)+L" blocks, undo stack size="+Stringify(_undoStack.size())+L" redo="+Stringify(_redoStack.size()));
}

void UndoStack::Redo(unsigned int blocks) {
	for(unsigned int a = 0; a < blocks; a++) {
		ThreadLock lock(&_stackLock);
		std::deque<UndoChanges>::reverse_iterator it = _redoStack.rbegin();
		if(it==_redoStack.rend()) {
			break;
		}
		UndoChanges& block = *it;
		_undoStack.push_back(block);
		block.Redo();
		_redoStack.pop_back();
	}

	EventUndoChange.Fire(ref<UndoStack>(this), UndoNotification());
	Log::Write(L"TJShared/UndoStack", L"Redid "+Stringify(blocks)+L" blocks, undo stack size="+Stringify(_undoStack.size())+L" redo="+Stringify(_redoStack.size()));
}

unsigned int UndoStack::GetUndoStepsAvailableCount() const {
	return _undoStack.size();
}

unsigned int UndoStack::GetRedoStepsAvailableCount() const {
	return _redoStack.size();
}

void UndoStack::Clear() {
	ThreadLock lock(&_stackLock);
	_redoStack.clear();
	_undoStack.clear();
	EventUndoChange.Fire(ref<UndoStack>(this), UndoNotification());
}

void UndoStack::AddBlock(UndoChanges& ub) {
	ThreadLock lock(&_stackLock);
	_undoStack.push_back(ub);
	_redoStack.clear();

	if(_undoStack.size() > KMaxUndoChangess) {
		_undoStack.pop_front();
	}

	EventUndoChange.Fire(ref<UndoStack>(this), UndoNotification());
}

/** ChangeDelegate **/
ChangeDelegate::~ChangeDelegate() {
}

/** Change **/
Change::~Change() {
}

Change::Change(const std::wstring& desc, ref<ChangeDelegate> dlg): _description(desc), _delegate(dlg) {
}

void Change::SetChangeDelegate(ref<ChangeDelegate> cd) {
	_delegate = cd;
}

ref<ChangeDelegate> Change::GetChangeDelegate() {
	return _delegate;
}

void Change::UndoAndNotify() {
	this->Undo();
	ref<ChangeDelegate> dlg = GetChangeDelegate();
	if(dlg) {
		dlg->OnChangeOccurred(this, true);
	}
}

void Change::RedoAndNotify() {
	this->Redo();
	ref<ChangeDelegate> dlg = GetChangeDelegate();
	if(dlg) {
		dlg->OnChangeOccurred(this, false);
	}
}

const std::wstring& Change::GetDescription() {
	return _description;
}

/** UndoBlock **/
UndoBlock::UndoBlock(ref<Object> blockObject): _uc(blockObject), _previousBlock(0) {
	_previousBlock = reinterpret_cast<UndoBlock*>(_currentBlock.GetValue());
	_currentBlock.SetValue(reinterpret_cast<void*>(this));
}

UndoBlock::~UndoBlock() {
	if(_uc.HasChanges()) {
		// If there is no previous block, add the changes to the undo stack as block
		if(_previousBlock==0) {
			strong<UndoStack> us = UndoStack::Instance();
			us->AddBlock(_uc);
		}
		else {
			// Merge into upper block
			std::deque< ref<Change> >::iterator it = _uc._changes.begin();
			while(it!=_uc._changes.end()) {
				_previousBlock->_uc.Add(*it);
				++it;
			}
		}
	}
	_currentBlock.SetValue(reinterpret_cast<void*>(_previousBlock));
}

UndoBlock* UndoBlock::GetCurrentBlock() {
	return reinterpret_cast<UndoBlock*>(_currentBlock.GetValue());
}

void UndoBlock::AddChange(ref<Change> change) {
	UndoBlock* current = GetCurrentBlock();
	if(current!=0) {
		current->_uc.Add(change);
	}
	else {
		// Push as single operation (TODO make optional)
		UndoChanges uc;
		uc.Add(change);
		UndoStack::Instance()->AddBlock(uc);
	}
}

void UndoBlock::AddAndDoChange(ref<Change> change) {
	if(change) {
		AddChange(change);
		change->Redo(); // Does not notify delegate, which is good
	}
}

/** UndoChanges **/
UndoChanges::UndoChanges(ref<Object> blockObject): _blockObject(blockObject) {
}

UndoChanges::~UndoChanges() {
}

bool UndoChanges::HasChanges() const {
	return _changes.size() > 0;
}

std::wstring UndoChanges::ToString() {
	std::wostringstream wos;
	std::deque< ref<Change> >::iterator it = _changes.begin();
	wos << L'[';
	while(it!=_changes.end()) {
		ref<Change> change = *it;
		if(change) {
			wos << change->GetDescription() << L"; ";
		}
		++it;
	}
	wos << L']';
	return wos.str();
}

void UndoChanges::Add(ref<Change> change) {
	_changes.push_back(change);
}

void UndoChanges::Undo() {
	//ref<Object> blockObject = _blockObject;
	//if(blockObject) {
		// Undo is executed in reverse order
		std::deque< ref<Change> >::reverse_iterator it = _changes.rbegin();
		while(it!=_changes.rend()) {
			ref<Change> change = *it;
			if(change && change->CanUndo()) {
				change->UndoAndNotify();
			}
			++it;
		}
	//}
	//else {
	//	Log::Write(L"TJShared/UndoChanges", L"Could not undo changes block, parent block object was deleted");
	//}
}

void UndoChanges::Redo() {
	///ref<Object> blockObject = _blockObject;
	///if(blockObject) {
		std::deque< ref<Change> >::iterator it = _changes.begin();
		while(it!=_changes.end()) {
			ref<Change> change = *it;
			if(change && change->CanUndo()) {
				change->RedoAndNotify();
			}
			++it;
		}
	///}
	///else {
	///	Log::Write(L"TJShared/UndoChanges", L"Could not redo changes block, parent block object was deleted");
	///}
}

/** Action **/
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