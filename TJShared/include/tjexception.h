#ifndef _EXCEPTION_H
#define _EXCEPTION_H

namespace tj {
	namespace shared {	
		enum ExceptionType {
			ExceptionTypeSevere=0,
			ExceptionTypeError,
			ExceptionTypeWarning,
			ExceptionTypeMessage,
		};

		#define Throw(msg,t) throw Exception((const wchar_t*)msg,t,(const char*)__FILE__, (int)__LINE__)

		class EXPORTED Exception {
			public:
				Exception(std::wstring message, ExceptionType type,const char* file="", int line=0) {
					_message = message;
					_type = type;
					_file = file;
					_line = line;
				}

				virtual ~Exception() {}

				const std::wstring GetMsg() const { return _message; }
				ExceptionType GetType() const { return _type; }
				int GetLine() const { return _line; }
				const char* GetFile() const { return _file; }
				std::wstring ToString() const;
			protected:
				std::wstring _message;
				ExceptionType _type;
				const char* _file;
				int _line;
		};
	}
}

#endif