#ifndef _TJCOMMAND_H
#define _TJCOMMAND_H

namespace tj {
	namespace shared {
		class EXPORTED Action: public virtual tj::shared::Object {
			public:
				enum UndoSupport {
					UndoSupported = 1,	// Undo is supported, add this command to the undo stack when executed
					UndoBlocking = 2, // Undo is not supported, clear whole undo stack; this action cannot be undone
					UndoIgnore = 4, // This command doesn't need/support undo (it doesn't modify anything?) so don't bother
				};

				virtual ~Action();
				virtual void Execute() = 0;
				virtual void Undo();

				UndoSupport GetUndoSupport() const;
				std::wstring GetName() const;

			protected:
				Action(const std::wstring& name, UndoSupport sup);
				Action(UndoSupport sup);
				std::wstring _name;
				UndoSupport _sup;
				
		};
	}
}

#endif
