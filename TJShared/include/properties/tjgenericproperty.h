#ifndef _TJGENERICPROPERTY_H
#define _TJGENERICPROPERTY_H

namespace tj {
	namespace shared {
		template<typename T> class GenericProperty: public Property, public Listener<EditWnd::NotificationTextChanged> {
			public:
				GenericProperty(const std::wstring& name, T* value, T* alsoSet, T defaultValue): Property(name), _multiLine(false), _value(value), _alsoSet(alsoSet), _defaultValue(defaultValue) {
					if(value==0) Throw(L"Property value pointer cannot be null", ExceptionTypeWarning);
				}

				virtual ~GenericProperty() {
					_value = 0;
					_alsoSet = 0;
				}

				virtual void Notify(ref<Object> source, const EditWnd::NotificationTextChanged& ev) {
					if(_wnd && _value!=0) {
						T value = StringTo<T>(_wnd->GetText(), _defaultValue);
						*_value = value;
						if(_alsoSet!=0) {
							*_alsoSet = value;
						}
					}
				}

				virtual ref<Wnd> GetWindow() {
					if(!_wnd) {
						ref<EditWnd> ew = GC::Hold(new EditWnd(_multiLine));
						ew->EventTextChanged.AddListener(this);
						ew->SetBorder(true);
						ew->SetCue(Stringify(_defaultValue));
						_wnd = ew;
						Update();
					}
					return _wnd;
				}

				virtual void Update() {
					if(_wnd) {
						_wnd->SetText(Stringify(*_value));
						_wnd->Repaint();
					}
				}

			protected:
				virtual void SetMultiline(bool t) {
					_multiLine = t;
				}

				T* _value;
				T* _alsoSet;
				T _defaultValue;
				ref<Wnd> _wnd;
				bool _multiLine;
		};

		// GenericProperty<bool>
		template<> ref<Wnd> EXPORTED GenericProperty<bool>::GetWindow();
		template<> void EXPORTED GenericProperty<bool>::Update();
		
		// GenericProperty<Time>
		// The default for Stringify(Time) is to output a number of milliseconds, we want the formatted string
		// since StringTo<Time> can perfectly handle that.
		template<> void EXPORTED GenericProperty<Time>::Update();
	}
}

#endif