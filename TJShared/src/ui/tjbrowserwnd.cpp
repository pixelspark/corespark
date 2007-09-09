#include "../../include/tjshared.h"
using namespace Gdiplus;
using namespace tj::shared;

#include <atlbase.h>
#include <atlwin.h>
#include <atlhost.h>
#include <exdispid.h>

CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

/* Browser event sink */
namespace tj {
	namespace shared {
		class BrowserSink:  public IDispEventImpl<1,BrowserSink,&DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1> {
			public:
				BrowserSink(BrowserWnd* wnd) {
					_browser = wnd;
				}

				virtual ~BrowserSink() {
				}

				void __stdcall OnNavigateComplete(IDispatch* dp, VARIANT *url) {
					if(url->bstrVal!=0 && url->vt==VT_BSTR) {
						
						CComBSTR urls(url->bstrVal);
						ref<BrowserToolbarWnd> tbw = _browser->_tools;
						tbw->SetURL(std::wstring(urls));
					}
				}

				void __stdcall OnBeforeNavigate(IDispatch *pDisp,VARIANT *&url,VARIANT *&Flags,VARIANT *&TargetFrameName,VARIANT *&PostData,VARIANT *&Headers,VARIANT_BOOL *&Cancel) {
					
				}	

				BEGIN_SINK_MAP(BrowserSink)
					SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, OnNavigateComplete)
					SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, OnBeforeNavigate)
				END_SINK_MAP()

			protected:
				BrowserWnd* _browser;
		};

		class NavigateRunnable: public Runnable {
			public:
				NavigateRunnable(CComPtr<IWebBrowser2> br, CComBSTR url) {
					_browser = br;
					_url = url;
				}

				virtual ~NavigateRunnable() {
				}

				virtual void Run() {
					CComVariant empty;
					_browser->Navigate(_url, &empty, &empty, &empty, &empty);
				}

				CComPtr<IWebBrowser2> _browser;
				CComBSTR _url;
		};
	}
}

BrowserWnd::BrowserWnd(std::wstring title): ChildWnd(title.c_str(), true, false) {
	_Module.Init(ObjectMap, (HINSTANCE)GetModuleHandle(L"tjshared.dll"), &LIBID_ATLLib);
	_Module.Init(ObjectMap, (HINSTANCE)GetModuleHandle(L"tjshared.dll"), &LIBID_MSHTML);
	AtlAxWinInit();

	_ax = new CAxWindow();
	_sink = new BrowserSink(this);
	_tools = GC::Hold(new BrowserToolbarWnd(this));
	Add(_tools);

	SetStyle(WS_CLIPCHILDREN);
	SetStyleEx(WS_EX_CONTROLPARENT);
	RECT rc = {0,0,100,100};

	HWND wnd = GetWindow();
	if(((CAxWindow*)_ax)->Create(wnd, rc,_T("Mozilla.Browser.1"),WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,0)) {
		Log::Write(L"BrowserWnd/Constructor", L"Using Mozilla/Gecko");
	}
	else if(((CAxWindow*)_ax)->Create(wnd, rc,_T("Shell.Explorer.2"),WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,0)) {
		Log::Write(L"BrowserWnd/Constructor", L"Using MSHTML");
	}

	// set some settings
	CComPtr<IWebBrowser2> web;
	((CAxWindow*)_ax)->QueryControl(IID_IWebBrowser2, (void**)&web);
	if(web) {
		web->put_Silent(TRUE);
		web->put_RegisterAsBrowser(FALSE);
	}

	_sink->DispEventAdvise(web, &DIID_DWebBrowserEvents2);
	Layout();
}

BrowserWnd::~BrowserWnd() {
	IWebBrowser2* ctrl = 0;
	if(SUCCEEDED(((CAxWindow*)_ax)->QueryControl(IID_IWebBrowser2, (void**)&ctrl)) && ctrl!=0) {
		_sink->DispEventUnadvise(ctrl, &DIID_DWebBrowserEvents2);
		ctrl->Release();
	}

	delete ((CAxWindow*)_ax);
	delete _sink;
}

void BrowserWnd::Back() {
	CComPtr<IWebBrowser2> web;
	((CAxWindow*)_ax)->QueryControl(IID_IWebBrowser2, (void**)&web);
	if(web) {
		web->GoBack();
	}
}

void BrowserWnd::Forward() {
	CComPtr<IWebBrowser2> web;
	((CAxWindow*)_ax)->QueryControl(IID_IWebBrowser2, (void**)&web);
	if(web) {
		web->GoForward();
	}
}

void BrowserWnd::Reload() {
	CComPtr<IWebBrowser2> web;
	((CAxWindow*)_ax)->QueryControl(IID_IWebBrowser2, (void**)&web);
	if(web) {
		web->Refresh();
	}
}

void BrowserWnd::Stop() {
	CComPtr<IWebBrowser2> web;
	((CAxWindow*)_ax)->QueryControl(IID_IWebBrowser2, (void**)&web);
	if(web) {
		web->Stop();
	}
}

void BrowserWnd::SetShowToolbar(bool t) {
	if(t) {
		if(!_tools) {
			_tools = GC::Hold(new BrowserToolbarWnd(this));
		}
		_tools->Show(true);
		Layout();
	}
	else {
		if(_tools) _tools->Show(false);
		Layout();
	}
}

void BrowserWnd::Layout() {
	Area r = GetClientArea();
	if(_tools && _tools->IsShown()) {
		_tools->Fill(LayoutTop, r);
	}

	// manually do the DPI stuff
	ref<Theme> theme = ThemeManager::GetTheme();
	r.Multiply(theme->GetDPIScaleFactor());

	if(((CAxWindow*)_ax)->IsWindow()) {
		((CAxWindow*)_ax)->SetParent(GetWindow());
		((CAxWindow*)_ax)->MoveWindow(r.GetLeft(),r.GetTop(), r.GetWidth(), r.GetHeight(), TRUE);
	}
}

void BrowserWnd::OnSize(const Area& ns) {
	Layout();
}

void BrowserWnd::Hide(bool t) {
	((CAxWindow*)_ax)->ShowWindowAsync(t?SW_HIDE:SW_SHOW);
	//Repaint();
}

void BrowserWnd::Navigate(std::wstring url) {
	CComPtr<IWebBrowser2> browser;
	((CAxWindow*)_ax)->QueryControl(IID_IWebBrowser2, (void**)&browser);
	if(browser) {
		std::string urlmbs = Mbs(url);
		Core::Instance()->AddAction(GC::Hold(new NavigateRunnable(browser, CComBSTR(urlmbs.c_str()))));
	}

	if(_tools) {
		_tools->SetURL(url);
	}
}

void BrowserWnd::Paint(Gdiplus::Graphics& g) {
	//if(_hidden) {
		Area rc = GetClientArea();
		SolidBrush br(Color(0,0,0));
		g.FillRectangle(&br, rc);
	//}
}

BrowserToolbarWnd::BrowserToolbarWnd(BrowserWnd *browser): ToolbarWnd() {
	_browser = browser;
	Gdiplus::Bitmap* bmp = Bitmap::FromFile(ResourceManager::Instance()->Get(L"icons/step_backward.png").c_str());
	Add(GC::Hold(new ToolbarItem(CmdBack, bmp)));

	bmp = Bitmap::FromFile(ResourceManager::Instance()->Get(L"icons/step_forward.png").c_str());
	Add(GC::Hold(new ToolbarItem(CmdForward, bmp)));

	bmp = Bitmap::FromFile(ResourceManager::Instance()->Get(L"icons/browser/stop.png").c_str());
	Add(GC::Hold(new ToolbarItem(CmdStop, bmp)));

	bmp = Bitmap::FromFile(ResourceManager::Instance()->Get(L"icons/browser/reload.png").c_str());
	Add(GC::Hold(new ToolbarItem(CmdReload, bmp)));

	bmp = Bitmap::FromFile(ResourceManager::Instance()->Get(L"icons/browser/go.png").c_str());
	Add(GC::Hold(new ToolbarItem(CmdGo, bmp)));
	

	_url = GC::Hold(new EditWnd());
	ChildWnd::Add(_url);
	Layout();
}

void BrowserToolbarWnd::OnCommand(int c) {
	if(_browser==0) return;

	switch(c) {
		case CmdBack:
			_browser->Back();
			break;

		case CmdForward:
			_browser->Forward();
			break;

		case CmdStop:
			_browser->Stop();
			break;

		case CmdReload:
			_browser->Reload();
			break;

		case CmdGo:
			_browser->Navigate(_url->GetText());
			break;
	}
}

void BrowserToolbarWnd::Paint(Gdiplus::Graphics& g) {
	ToolbarWnd::Paint(g);
}

BrowserToolbarWnd::~BrowserToolbarWnd() {
}

void BrowserToolbarWnd::Layout() {
	Area a = GetClientArea();
	_url->Move(5*24+2, 2, a.GetRight()-5*24-6,20);
	ToolbarWnd::Layout();
}

LRESULT BrowserToolbarWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	static Color back;
	static HBRUSH editBackground = 0;
	if(msg==WM_CTLCOLOREDIT) {
		ref<Theme> theme = ThemeManager::GetTheme();
		Color c = theme->GetBackgroundColor();

		if(editBackground==0 || back.GetValue()!=back.GetValue()) {
			if(editBackground!=0) DeleteObject(editBackground);
			editBackground = CreateSolidBrush(RGB(c.GetR(), c.GetG(), c.GetB()));
		}
		return (LRESULT)(HBRUSH)editBackground;
	}
	return ToolbarWnd::Message(msg, wp, lp);
}

void BrowserToolbarWnd::SetURL(std::wstring url) {
	_url->SetText(url);
}