#include "../../include/tjshared.h"
using namespace tj::shared;

std::wstring Dialog::AskForSaveFile(HWND owner, std::wstring title, const wchar_t* filter, std::wstring defExt) {
	wchar_t* fname = new wchar_t[MAX_PATH];
	memset(fname,0,sizeof(wchar_t)*MAX_PATH);
	OPENFILENAME fn;
	memset(&fn, 0, sizeof(OPENFILENAME));
	fn.lStructSize = sizeof(OPENFILENAME); 
	fn.hwndOwner = owner;
	fn.hInstance = GetModuleHandle(NULL);
	fn.lpstrFilter = filter;
	fn.lpstrFile = fname;
	fn.nMaxFile = 1023; 
	fn.lpstrTitle = title.c_str();
	fn.lpstrDefExt = defExt.c_str();

	BOOL result = GetSaveFileName(&fn);

	std::wstring filename(fname);
	delete[] fname;
	return result?filename:L"";
}


std::wstring Dialog::AskForOpenFile(HWND owner, std::wstring title, const wchar_t* filter, std::wstring defExt) {
	wchar_t* fname = new wchar_t[MAX_PATH];
	memset(fname,0,sizeof(wchar_t)*MAX_PATH);
	OPENFILENAME fn;
	memset(&fn, 0, sizeof(OPENFILENAME));
	fn.lStructSize = sizeof(OPENFILENAME); 
	fn.hwndOwner = owner;
	fn.hInstance = GetModuleHandle(NULL);
	fn.lpstrFilter = filter;
	fn.lpstrFile = fname;
	fn.nMaxFile = 1023; 
	fn.lpstrTitle = title.c_str();
	fn.lpstrDefExt = defExt.c_str();

	BOOL result = GetOpenFileName(&fn);

	std::wstring filename(fname);
	delete[] fname;
	return result?filename:L"";
}