#ifndef _TJCOMMAND_H
#define _TJCOMMAND_H

#include "tjsharedinternal.h"
#include "tjlistener.h"
#include "tjthread.h"
#include "tjdispatch.h"

#include <deque>

namespace tj {
	namespace shared {
		class Change;

		// A ChangeDelegate object can be used if you want to get notified when a change,
		// once queued by your code, has executed.
		class EXPORTED ChangeDelegate: public virtual tj::shared::Object {
			public:
				virtual ~ChangeDelegate();
				virtual void OnChangeOccurred(ref<Change> change, bool wasUndo) = 0;
		};

		class EXPORTED Change: public virtual tj::shared::Object {
			public:
				Change(const std::wstring& description = L"", ref<ChangeDelegate> dlg = null);
				virtual ~Change();
				void UndoAndNotify();
				void RedoAndNotify();
				virtual void Redo() = 0;
				virtual void Undo() = 0;
				virtual const std::wstring& GetDescription();
				virtual bool CanUndo() = 0;
				virtual bool CanRedo() = 0;
				virtual void SetChangeDelegate(ref<ChangeDelegate> cdg);
				virtual ref<ChangeDelegate> GetChangeDelegate();

			protected:
				std::wstring _description;

			private:
				weak<ChangeDelegate> _delegate;
		};

		class EXPORTED UndoChanges {
			friend class UndoStack;

			public:
				UndoChanges(ref<Object> blockObject = null);
				~UndoChanges();
				std::wstring ToString();
				bool HasChanges() const;
				void Add(ref<Change> change);
				void Undo();
				void Redo();
				
				std::deque< ref<Change> > _changes;
				weak<Object> _blockObject;
		};

		class EXPORTED UndoBlock {
			public:
				UndoBlock(ref<Object> blockObject = null);
				~UndoBlock();

				static void AddChange(ref<Change> change);
				static void AddAndDoChange(ref<Change> change);
				

			private:
				static UndoBlock* GetCurrentBlock();
				UndoChanges _uc;
				static ThreadLocal _currentBlock;
				UndoBlock* _previousBlock;
		};

		class EXPORTED UndoStack: public virtual Object {
			friend class UndoBlock;
			friend class UndoChanges;

			public:
				static strong<UndoStack> Instance();
				virtual ~UndoStack();
				void Undo(unsigned int blocks = 1);
				void Redo(unsigned int blocks = 1);
				unsigned int GetUndoStepsAvailableCount() const;
				unsigned int GetRedoStepsAvailableCount() const;
				void Clear();

				struct UndoNotification {
				};

				Listenable<UndoNotification> EventUndoChange;

			protected:
				UndoStack();
				void AddBlock(UndoChanges& ub);
				
				static unsigned int KMaxUndoChangess;
				static ref<UndoStack> _instance;
				CriticalSection _stackLock;
				std::deque<UndoChanges> _undoStack;
				std::deque<UndoChanges> _redoStack;
				
		};

		/** The VariableChange class represents an undoable change to the value of a variable. It records both the old and the new value,
		and uses it to allow undo and redo. **/
		template<class O, typename V> class VariableChange: public Change {
			public:
				inline VariableChange(ref<O> object, V& member, const V& newValue, const std::wstring& description = L""): Change(description), _object(object), _newValue(newValue), _member(member) {
				}

				inline VariableChange(ref<O> object, V& member, const V& newValue, const V& oldValue, const std::wstring& description = L""): Change(description), _object(object), _newValue(newValue), _oldValue(oldValue), _member(member) {
				}

				virtual ~VariableChange() {
				}

				virtual void Redo() {
					ref<O> o = _object;
					if(!o) {
						Throw(L"Cannot redo operation; object does not exist anymore!", ExceptionTypeWarning);
					}

					_oldValue = _member;
					_member = _newValue;
				}

				virtual void Undo() {
					ref<O> o = _object;
					if(o) {
						_member = _oldValue;
					}
				}

				virtual bool CanUndo() {
					return ref<O>(_object);
				}

				virtual bool CanRedo() {
					return CanUndo();
				}

			protected:
				weak<O> _object;
				V& _member;
				V _newValue;
				V _oldValue;
		};

		/** The Undoable class allows for simple and straightforward notation of undo-able actions. The Undoable class wraps
		member variables of an object, and records the change in value as undo change when it is destructed. Thus, where you would
		normally do something like this:
		
		void MyObject::Change(int value) { this->mValue = value; }

		You can now do the following:

		void MyObject::Change(int value) { Undoable(this, &(this->mValue), L"Change value of MyObject") = value;  }

		This will queue a VariableChange in the current UndoBlock (and, of course, also execute the assignment).
		**/
		template<class O, typename V> class Undoable {
			public:
				inline Undoable(ref<O> object, V& member, ref<ChangeDelegate> dlg = null, const std::wstring& description = L""): _object(object), _delegate(dlg), _member(member), _oldValue(_member), _description(description) {
				}

				~Undoable() {
					if(_member!=_oldValue) {
						ref<Change> vc = GC::Hold(new VariableChange<O, V>(_object, _member, _member, _oldValue, _description));
						vc->SetChangeDelegate(_delegate);
						UndoBlock::AddChange(vc);
					}
				}

				inline operator V&() {
					return _member;
				}

				inline V& operator+=(const V& other) {
					_member += other;
					return _member;
				}

				inline V& operator-=(const V& other) {
					_member -= other;
					return _member;
				}

				inline V& operator*=(const V& other) {
					_member *= other;
					return _member;
				}

				inline V& operator/=(const V& other) {
					_member /= other;
					return _member;
				}

				inline V& operator=(const V& other) {
					_member = other;
					return _member;
				}

			private:
				ref<O> _object;
				weak<ChangeDelegate> _delegate;
				V& _member;
				V _oldValue;
				const std::wstring& _description;
		};

		class EXPORTED Action: public virtual tj::shared::Task {
			public:
				enum UndoSupport {
					UndoSupported = 1,	// Undo is supported, add this command to the undo stack when executed
					UndoBlocking = 2, // Undo is not supported, clear whole undo stack; this action cannot be undone
					UndoIgnore = 4, // This command doesn't need/support undo (it doesn't modify anything?) so don't bother
				};

				virtual ~Action();
				virtual void Execute() = 0;
				virtual void Undo();
				virtual void Run();

				UndoSupport GetUndoSupport() const;
				String GetName() const;

			protected:
				Action(const String& name, UndoSupport sup);
				Action(UndoSupport sup);
				String _name;
				UndoSupport _sup;
				
		};
	}
}

#endif
