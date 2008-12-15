#ifndef _TJTEXTPROPERTY_H
#define _TJTEXTPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED TextProperty: public GenericProperty<std::wstring> {
			public:
				TextProperty(const std::wstring& name, std::wstring* value, Pixels height = 100);
				virtual ~TextProperty();
				virtual Pixels GetHeight();

			protected:
				Pixels _height;
		};

		class EXPORTED SuggestionProperty: public Property, public Listener<EditWnd::NotificationTextChanged> {
			public:
				SuggestionProperty(const std::wstring& name, std::wstring* value, bool multiLine = false);
				virtual ~SuggestionProperty();
				virtual void Notify(ref<Object> source, const EditWnd::NotificationTextChanged& ev);
				virtual ref<Wnd> GetWindow();
				virtual void Update();
				virtual Pixels GetHeight();
				virtual strong<Menu> GetSuggestionMenu();
				virtual void SetSuggestionMode(SuggestionEditWnd::SuggestionMode sm);

			protected:
				std::wstring* _value;
				ref<SuggestionEditWnd> _wnd;
				bool _multiLine;
		};
	}
}

#endif