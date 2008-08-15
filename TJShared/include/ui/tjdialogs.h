#ifndef _TJSHAREDDIALOGS_H
#define _TJSHAREDDIALOGS_H

namespace tj {
	namespace shared {
		class EXPORTED DialogWnd: public TopWnd, public Listener<ButtonWnd::NotificationClicked> {
			public:	
				DialogWnd(const std::wstring& title);
				virtual ~DialogWnd();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void Layout();
				virtual void OnSize(const Area& ns);
				virtual void OnCreated();
				virtual bool DoModal(ref<Wnd> parent);
				virtual void Notify(ref<Object> source, const ButtonWnd::NotificationClicked& evt);

			protected:
				virtual void OnAfterShowDialog();
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				virtual void EndModal(bool result);

			private:
				ModalLoop _loop;
				ref<ButtonWnd> _ok;

		};

		class EXPORTED PropertyDialogWnd: public DialogWnd {
			public:
				PropertyDialogWnd(const std::wstring& title, const std::wstring& question = L"");
				virtual ~PropertyDialogWnd();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual ref<PropertyGridWnd> GetPropertyGrid();
				virtual void Layout();
				virtual Area GetClientArea() const;
				virtual void SetSize(Pixels w, Pixels h);
		
			protected:
				virtual void OnSize(const Area& ns);
				virtual void OnAfterShowDialog();
				virtual bool HasQuestion() const;
				virtual Pixels GetHeaderHeight() const;
				
			private:
				ref<PropertyGridWnd> _grid;
				std::wstring _question;
		};

		class EXPORTED Dialog {
			public:
				static std::wstring AskForSaveFile(ref<Wnd> owner, const std::wstring& title, const wchar_t* filter, const std::wstring& defExt);
				static std::wstring AskForOpenFile(ref<Wnd> owner, const std::wstring& title, const wchar_t* filter, const std::wstring& defExt);
		};
	}
}

#endif