#ifndef _TJUPDATERUI_H
#define _TJUPDATERUI_H

namespace tj {
	namespace updater {
		class UpdaterDialog {
			public:
				UpdaterDialog();
				virtual ~UpdaterDialog();
				void NotifyUser(const std::wstring& infoTitle, const std::wstring& infoText);
				bool DoModal(Updater& up);

			private:
				HWND _wnd;
		};
	}
}

#endif