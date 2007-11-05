#ifndef _TJPROPERTY_H
#define _TJPROPERTY_H

namespace tj {
	namespace shared {
		class EXPORTED Property: public virtual Object {
			public:
				Property(const std::wstring& name);
				virtual ~Property();
				virtual std::wstring GetName();
				virtual int GetHeight();

				virtual std::wstring GetValue() = 0;
				virtual HWND GetWindow() = 0;
				virtual HWND Create(HWND parent) = 0;
				
				// Called when the value in the edit window has changed (EN_CHANGED)
				virtual void Changed() = 0;
				
				// Called when a repaint is about to begin and the value in the window needs to be updated
				virtual void Update() = 0;

				// Hints
				virtual const std::wstring& GetHint() const;
				virtual void SetHint(const std::wstring& h);

			protected:
				std::wstring _name;
				std::wstring _hint;
		};

		class EXPORTED Inspectable {
			public:
				virtual ~Inspectable();
				virtual ref< std::vector< ref<Property> > > GetProperties()=0;
		};

		template<typename T> class GenericInspectable: public Inspectable {
			public:
				inline GenericInspectable(T* o): _object(o) {
				}

				virtual ~GenericInspectable() {
				}

				virtual ref< std::vector< ref<Property> > > GetProperties() {
					return _object->GetProperties();
				}
			
			protected:
				T* _object;
		};

		template<typename T> class GenericProperty: public Property {
			public:
				GenericProperty(std::wstring name, T* value, T* alsoSet, T defaultValue): Property(name) {
					if(value==0) Throw(L"Property value pointer cannot be null", ExceptionTypeWarning);
					_value = value;
					_alsoSet = alsoSet;
					assert(value!=0);
					_wnd = 0;
					_default = defaultValue;
				}

				virtual HWND Create(HWND parent) {
					if(_wnd!=0) return _wnd;
					HINSTANCE hinst;
					hinst = GetModuleHandle(NULL);
					_wnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, TJ_PROPERTY_EDIT_CLASS_NAME, Stringify(*_value).c_str(), ES_AUTOHSCROLL|WS_CHILD, 0, 0, 100, 100, parent, (HMENU)0, hinst, 0);
					if(_wnd==0) {
						int x = GetLastError();
						Throw(L"Property window creation failed ", ExceptionTypeError);
					}
					return _wnd;
				}

				virtual ~GenericProperty() {
					DestroyWindow(_wnd);
					_wnd = 0;
					_value = 0;
				}

				virtual HWND GetWindow() {
					return _wnd;
				}

				virtual void Changed() {
					int l = GetWindowTextLength(_wnd);
					wchar_t* cp = new wchar_t[l+2];
					GetWindowText(_wnd, cp, l+1);
					
					std::wstring cs(cp);
					cs += L".0";
					delete[] cp;

					T value = _default;
					std::wistringstream os(cs);
					os >> value;

					if(*_value!=value) {
						(*_value) = value;
						if(_alsoSet!=0) {
							(*_alsoSet) = value;
						}
					}
				}

				virtual void Update() {
					try {
						if(_value!=0) {
							std::wostringstream os;
							os << T(*_value);
							std::wstring v = os.str();
							SetWindowText(_wnd, v.c_str());
						}
					}
					catch(std::exception e) {
						// ignore
					}
				}

				virtual std::wstring GetValue() {
					assert(_value!=0);
					return Stringify(T(*_value));
				}

				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp) {
					return 0;
				}

			protected:
				T* _value;
				T _default;
				T* _alsoSet;
				HWND _wnd;
		};

		template<> HWND EXPORTED GenericProperty<Time>::Create(HWND parent);
		template<> void EXPORTED GenericProperty<Time>::Changed();
		template<> void EXPORTED GenericProperty<Time>::Update();

		template<> HWND EXPORTED GenericProperty<bool>::Create(HWND parent);
		template<> void EXPORTED GenericProperty<bool>::Changed();
		template<> void EXPORTED GenericProperty<bool>::Update();

		// for numeric edit stuff, spinner boxes and lots of other candy
		template<> HWND EXPORTED GenericProperty<unsigned int>::Create(HWND parent);
		template<> HWND EXPORTED GenericProperty<int>::Create(HWND parent);
		template<> void EXPORTED GenericProperty<std::wstring>::Changed();
	}
}

#endif