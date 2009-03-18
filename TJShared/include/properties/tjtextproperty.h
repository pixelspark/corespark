#ifndef _TJTEXTPROPERTY_H
#define _TJTEXTPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED TextProperty: public GenericProperty<String> {
			public:
				TextProperty(const String& name, String* value, Pixels height = 100);
				virtual ~TextProperty();
				virtual Pixels GetHeight();

			protected:
				Pixels _height;
		};

		class EXPORTED SuggestionProperty: public Property, public Listener<EditWnd::NotificationTextChanged> {
			public:
				SuggestionProperty(const String& name, String* value, bool multiLine = false);
				virtual ~SuggestionProperty();
				virtual void Notify(ref<Object> source, const EditWnd::NotificationTextChanged& ev);
				virtual ref<Wnd> GetWindow();
				virtual void Update();
				virtual Pixels GetHeight();
				virtual strong<Menu> GetSuggestionMenu();
				virtual void SetSuggestionMode(SuggestionEditWnd::SuggestionMode sm);

			protected:
				String* _value;
				ref<SuggestionEditWnd> _wnd;
				bool _multiLine;
		};
	}
}

#endif