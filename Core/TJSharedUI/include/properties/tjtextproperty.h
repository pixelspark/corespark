#ifndef _TJTEXTPROPERTY_H
#define _TJTEXTPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED TextProperty: public GenericProperty<String> {
			public:
				TextProperty(const String& name, ref<Inspectable> holder, String* value, Pixels height = 100);
				virtual ~TextProperty();
				virtual Pixels GetHeight();

			protected:
				Pixels _height;
		};

		class EXPORTED SuggestionProperty: public Property, public Listener<EditWnd::EditingNotification> {
			public:
				SuggestionProperty(const String& name, ref<Inspectable> holder, String* value, bool multiLine = false);
				virtual ~SuggestionProperty();
				virtual void Notify(ref<Object> source, const EditWnd::EditingNotification& ev);
				virtual ref<Wnd> GetWindow();
				virtual void Update();
				virtual Pixels GetHeight();
				virtual strong<Menu> GetSuggestionMenu();
				virtual void SetSuggestionMode(SuggestionEditWnd::SuggestionMode sm);

			protected:
				weak<Inspectable> _holder;
				String* _value;
				String _oldValue;
				ref<SuggestionEditWnd> _wnd;
				bool _multiLine;
		};
	}
}

#endif