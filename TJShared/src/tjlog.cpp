#include "../include/tjshared.h"

LoggerWnd* Log::_logger = 0;

void Log::Write(std::wstring source, std::wstring message) {
	if(_logger==0) _logger = new LoggerWnd(0L);
	_logger->Log(source + L": " + message);
}

void Log::Show(bool t) {
	if(_logger==0) return;
	_logger->Show(t);
}