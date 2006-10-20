#ifndef _TJCOMMAND_H
#define _TJCOMMAND_H

class EXPORTED Action: public virtual tj::shared::Object {
	public:
		enum UndoSupport {
			UndoSupported = 1,	// Undo is supported, add this command to the undo stack when executed
			UndoBlocking = 2, // Undo is not supported, clear whole undo stack; this action cannot be undone
			UndoIgnore = 4, // This command doesn't need/support undo (it doesn't modify anything?) so don't bother
		};

		virtual ~Action();
		virtual void Execute() = 0;
		virtual void Undo() = 0;
		virtual UndoSupport GetUndoSupport() const = 0;
		virtual std::wstring GetName() const = 0;

		
};


#endif
