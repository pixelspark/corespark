#include "../TJShared/include/tjshared.h"
#include <breakpad/src/client/windows/sender/crash_report_sender.h>
#include <shlwapi.h> 
using namespace tj::shared;
using namespace google_breakpad;

static std::wstring AirbagURL = L"http://tj.ahref.nl/airbag/report";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmd, int nShow) {
	// Get program directory
	wchar_t* buf = new wchar_t[MAX_PATH];
	memset(buf,0,sizeof(wchar_t)*MAX_PATH);
	GetModuleFileName(GetModuleHandle(NULL), buf, MAX_PATH);
	PathRemoveFileSpec(buf);
	std::wstring programDir = buf;
	delete[] buf;	

	// Find out in what language TJShow was used
	std::wstring settingsPath = SettingsStorage::GetSettingsPath(L"TJ", L"TJShow", L"settings");
	ref<SettingsStorage> st = GC::Hold(new SettingsStorage());
	st->Load(settingsPath);
	Language::Load(programDir + L"\\locale\\"+st->GetValue(L"locale", L"en")+L"\\tjcrashreporter.tjs");

	// Get arguments
	std::map<std::wstring, std::wstring> params;
	std::wostringstream os;
	os << cmd;
	std::wstring dumpFile = os.str();
	if(dumpFile.length()>0) {
		dumpFile = dumpFile.substr(1,dumpFile.length()-2);
	}
	else {
		MessageBox(0L, TL(crash_argument_error), TL(crash_title), MB_ICONERROR|MB_OK);
		return 0;
	}

	bool send = MessageBox(0L, TL(crash_question), TL(crash_title), MB_ICONERROR|MB_YESNO)==IDYES;
	std::wostringstream message;

	if(send) {
		bool error = false;
		std::wstring code;
		CrashReportSender* cr = new CrashReportSender(L"");
		ReportResult rs = cr->SendCrashReport(AirbagURL,params,dumpFile,&code);
		switch(rs) {
			case RESULT_SUCCEEDED:
				message << TL(crash_successfully_sent);
				DeleteFile(dumpFile.c_str());
				break;

			default:
				message << TL(crash_not_sent);
				error = true;
		}

		std::wstring msg = message.str();
		MessageBox(0L, msg.c_str(), TL(crash_title), MB_OK);
	}
	
	return 0;
}