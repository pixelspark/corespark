#ifndef _TJSCRIPTEXCEPTION_H
#define _TJSCRIPTEXCEPTION_H

namespace tj {
	namespace script {
		class SCRIPT_EXPORTED ScriptException: public tj::shared::Exception {
			public:
				ScriptException(const std::wstring& msg);
				virtual ~ScriptException();
		};

		class SCRIPT_EXPORTED BreakpointException: public ScriptException {
			public:
				BreakpointException();
				virtual ~BreakpointException();
		};

		class SCRIPT_EXPORTED ParserException: public ScriptException {
			public:
				ParserException(const std::wstring& error);
				virtual ~ParserException();
		};
	}
}

#endif