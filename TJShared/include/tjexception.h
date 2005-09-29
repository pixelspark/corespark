#ifndef _EXCEPTION_H
#define _EXCEPTION_H

enum ExceptionType {
	ExceptionTypeSevere=0,
	ExceptionTypeError,
	ExceptionTypeWarning,
	ExceptionTypeMessage,
};

#define Throw(msg,t) throw Exception((const wchar_t*)msg,t,(const char*)__FILE__, (int)__LINE__)
#define ErrorMessage(msg,t) MessageBox(0L, msg, L"Error", MB_OK|MB_ICONWARNING)

class EXPORTED Exception {
	public:
		Exception(const wchar_t* message, ExceptionType type,const char* file="", int line=0) {
			_message = message;
			_type = type;
			_file = file;
			_line = line;
		}

		virtual ~Exception() {}

		const wchar_t* GetMsg() { return _message; }
		ExceptionType GetType() { return _type; }
		int GetLine() { return _line; }
		const char* GetFile() { return _file; }
		std::wstring ToString();

	protected:
		const wchar_t* _message;
		ExceptionType _type;
		const char* _file;
		int _line;
};

#endif