#ifndef _TJSHAREDDIALOGS_H
#define _TJSHAREDDIALOGS_H

namespace tj {
	namespace shared {
		class EXPORTED DialogWnd: public TopWnd, public Listener {
			public:
				DialogWnd(const std::wstring& title, const std::wstring& question);
				virtual ~DialogWnd();
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual ref<PropertyGridWnd> GetPropertyGrid();
				virtual void Layout();
				virtual bool DoModal(HWND parent);
				virtual void Notify(Wnd* source, Notification evt);

			protected:
				virtual void OnSize(const Area& ns);
				virtual void EndModal(bool result);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);

				static const Pixels KHeaderHeight = 24;
				
			private:
				ref<PropertyGridWnd> _grid;
				ref<ButtonWnd> _ok;
				bool _result;
				bool _running;
				std::wstring _question;
				

		};

		class EXPORTED Dialog {
			public:
				static std::wstring AskForSaveFile(HWND owner, std::wstring title, const wchar_t* filter, std::wstring defExt);
				static std::wstring AskForOpenFile(HWND owner, std::wstring title, const wchar_t* filter, std::wstring defExt);
		};
	}
}

#endif