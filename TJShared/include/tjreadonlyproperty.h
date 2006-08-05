#ifndef _TJREADONLYPROPERTY_H
#define _TJREADONLYPROPERTY_H

template<typename T> class GenericReadOnlyProperty: public GenericProperty<T> {
	public:
		GenericReadOnlyProperty(std::wstring name, T* value): GenericProperty<T>(name,value,0, *value) {
		}

		virtual ~GenericReadOnlyProperty() {
		}

		virtual HWND Create(HWND parent) {
			if(_wnd!=0) return _wnd;
			_wnd = ::CreateWindowEx(0, TJ_PROPERTY_LABEL_CLASS_NAME, Stringify(*_value).c_str(), WS_CHILD, 0, 0, 100, 100, parent, (HMENU)0, GetModuleHandle(NULL), 0);
			if(_wnd==0) {
				Throw(L"Property window creation failed", ExceptionTypeError);
			}
			return _wnd;
		}
};

#endif