#include "../include/internal/tjscript.h"
using namespace tj::script;
using namespace tj::shared;

ParameterException::ParameterException(std::wstring param): ScriptException(std::wstring(L"parameter ") + param + L" is missing or invalid") {
}

ParameterException::~ParameterException() {
}

