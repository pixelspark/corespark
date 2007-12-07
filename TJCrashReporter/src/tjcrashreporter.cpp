#define _WIN32_IE 0x0600

#include <windows.h>
#include <string>
#include <sstream>
#include <map>
#include <breakpad/src/client/windows/sender/crash_report_sender.h>
using namespace google_breakpad;

static std::wstring AirbagURL = L"http://tj.ahref.nl/airbag/report";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmd, int nShow) {
	std::map<std::wstring, std::wstring> params;
	std::wostringstream os;
	os << cmd;
	std::wstring dumpFile = os.str();
	dumpFile = dumpFile.substr(1,dumpFile.length()-2);

	bool send = MessageBox(0L, L"Het programma waarmee u werkte is zojuist door een onbekende oorzaak vastgelopen of beëindigd. Mogelijk zijn daarbij gegevens verloren gegaan.\r\n\r\n" 
		L"Van deze gebeurtenis zijn gegevens opgeslagen die de makers " 
		L"van het programma kunnen helpen het probleem op te lossen en in de toekomst te voorkomen. Wilt u deze informatie verzenden " 
		L"naar de makers? De informatie bevat interne gegevens van het programma net " 
		L"voor de fout optrad, uw IP-adres en mogelijk gegevens uit geopende bestanden in het programma.\r\n\r\n" 
		L"Deze gegevens zullen alleen gebruikt worden voor het oplossen van fouten in het programma en zullen nooit" 
		L"voor andere doeleinden worden gebruikt of worden vrijgegeven aan derden.", L"Fout in programma", MB_ICONERROR|MB_YESNO)==IDYES;

	std::wostringstream message;

	if(send) {
		bool error = false;
		std::wstring code;
		CrashReportSender* cr = new CrashReportSender(L"");
		ReportResult rs = cr->SendCrashReport(AirbagURL,params,dumpFile,&code);
		switch(rs) {
			case RESULT_SUCCEEDED:
				message << L"Het foutenrapport is succesvol verzonden naar de makers. Bedankt voor uw medewerking!\r\nRapportnummer: " << code;
				DeleteFile(dumpFile.c_str());
				break;

			default:
				message << L"Het foutenrapport kon niet verzonden worden naar de makers van het programma. Mogelijk bent u niet verbonden met het internet.";
				error = true;
		}

		std::wstring info = message.str();
		NOTIFYICONDATA nid;
		nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		nid.dwInfoFlags = error?NIIF_ERROR:NIIF_INFO;
		nid.hWnd = 0;
		nid.uID = 1;
		nid.uFlags = NIF_INFO;
		wcscpy_s(nid.szInfo, 255, info.c_str());
		wcscpy_s(nid.szInfoTitle, 62, L"TJ Crash Reporter");
		nid.uTimeout = 0;
		Shell_NotifyIcon(NIM_ADD, &nid);
		
		Sleep(30000);
		Shell_NotifyIcon(NIM_DELETE, &nid);
	}
	
	return 0;
}