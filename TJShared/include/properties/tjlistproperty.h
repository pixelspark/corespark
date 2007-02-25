#ifndef _TJLISTPROPERTY_H
#define _TJLISTPROPERTY_H

namespace tj {
	namespace shared {
		template<typename T> class GenericListProperty: public Property {
			public:
				struct Item {
					std::wstring _name;
					T _value;
				};

				GenericListProperty(std::wstring name, T* value, T* also, T def): Property(name) {
					_value = value;
					_alsoSet = also;
					_default = def;
					_wnd = 0;
					_editable = false;
				}

				virtual ~GenericListProperty() {
					DestroyWindow(_wnd);
				}

				void SetEditable(bool e) {
					_editable = e;
				}

				void AddOption(std::wstring name, T value) {
					Item it;
					it._name = name;
					it._value = value;
					_options.push_back(it);
				}
			
				virtual HWND Create(HWND parent) {
					if(_wnd!=0) return _wnd;
					_wnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, L"COMBOBOX", L"", WS_CHILD|(_editable?CBS_DROPDOWN:CBS_DROPDOWNLIST)|WS_VSCROLL, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);

					// Add strings
					std::vector<Item>::iterator it = _options.begin();
					while(it!=_options.end()) {
						Item item = *it;
						SendMessage(_wnd, CB_ADDSTRING, 0, (LPARAM)item._name.c_str());
						it++;
					}
					return _wnd;
				}

				virtual void Changed() {
					T value = _default;
					int i = (int)SendMessage(_wnd, CB_GETCURSEL, 0, 0L);

					try {
						Item it = _options.at(i);
					
						value = it._value;
						if(*_value!=value) {
							(*_value) = value;
							if(_alsoSet!=0) {
								(*_alsoSet) = value;
							}
						}

						SetWindowText(_wnd, it._name.c_str());
					}
					catch(...) {
						return;
					}
				}

				virtual void Update() {
					std::vector<Item>::iterator x = _options.begin();

					int i = 0;
					while(x!=_options.end()) {
						Item item = *x;
						if(item._value==(*_value)) {
							SendMessage(_wnd, CB_SETCURSEL, i,0L);
							SetWindowText(_wnd, item._name.c_str());
							return;
						}
						x++;
						i++;
					}
				}

				virtual std::wstring GetValue() {
					assert(_value!=0);
					return Stringify(*_value);
				}

				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp) {
					return 0;
				}

				virtual void SetParent(HWND wnd) {
					::SetParent(_wnd, wnd);
				}

				virtual HWND GetWindow() {
					return _wnd;
				}

			protected:
				T* _value;
				T _default;
				T* _alsoSet;
				HWND _wnd;
				std::vector<Item> _options;
				bool _editable;
		};
	}
}

#endif