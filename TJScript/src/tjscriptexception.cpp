#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ScriptException::ScriptException(std::wstring msg): Exception(msg.c_str(),ExceptionTypeError) {
}

ScriptException::~ScriptException() {
}

BreakpointException::BreakpointException(): ScriptException(L"Breakpoint triggered") {
}

BreakpointException::~BreakpointException() {
}

ParserException::ParserException(std::wstring r): ScriptException(r) {
}

ParserException::~ParserException() {
}
