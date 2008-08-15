#include "../../include/ui/tjui.h" 
using namespace tj::shared;
using namespace tj::shared::graphics;

#include <commdlg.h>

/* DialogWnd */
DialogWnd::DialogWnd(const std::wstring& title): TopWnd(title.c_str()) {
	SetStyle(WS_POPUPWINDOW|WS_THICKFRAME);
	_ok = GC::Hold(new ButtonWnd(Icons::GetIconPath(Icons::IconOK).c_str(), TL(ok)));
	Add(_ok, true);
	SetSize(400,300);
}

DialogWnd::~DialogWnd() {
}

void DialogWnd::OnCreated() {
	_ok->EventClicked.AddListener(ref<Listener<ButtonWnd::NotificationClicked> >(this));
}

void DialogWnd::Notify(ref<Object> source, const ButtonWnd::NotificationClicked& evt) {
	if(source==ref<Object>(_ok)) {
		EndModal(true);
	}
}

void DialogWnd::Layout() {
	static const Pixels KOKButtonWidth = 90;
	static const Pixels KOKButtonHeight = 20;

	Area rc = TopWnd::GetClientArea();
	Pixels hh = ThemeManager::GetTheme()->GetMeasureInPixels(Theme::MeasureDialogHeaderHeight);
	Pixels margin = (hh-KOKButtonHeight)/2;
	_ok->Move(rc.GetRight()-KOKButtonWidth-margin, rc.GetBottom()-hh+margin, KOKButtonWidth, KOKButtonHeight);
}

bool DialogWnd::DoModal(ref<Wnd> parent) {
	// Get root window of parent
	HWND parentNative = parent ? parent->GetWindow() : GetForegroundWindow();
	HWND root = GetAncestor(parentNative, GA_ROOT);
	EnableWindow(root,FALSE);

	/* Center this window */
	ref<Theme> theme = ThemeManager::GetTheme();
	float df = theme->GetDPIScaleFactor();
	Area rc = TopWnd::GetClientArea();
	int w = int(rc.GetWidth()*df);
	int h = int(rc.GetHeight()*df);

	// Get root rectangle
	RECT rootRect;
	GetWindowRect(root, &rootRect);
	SetWindowPos(GetWindow(), 0L, rootRect.left + ((rootRect.right-rootRect.left - w)/2), rootRect.top + ((rootRect.bottom-rootRect.top - h)/2), 0,0, SWP_NOSIZE|SWP_NOZORDER);
	Show(true);
	OnAfterShowDialog();

	ModalLoop::Result result = _loop.Enter(GetWindow(),true);

	EnableWindow(root, TRUE);
	SetForegroundWindow(root);
	return result == ModalLoop::ResultSucceeded;
}

void DialogWnd::OnAfterShowDialog() {
}

void DialogWnd::EndModal(bool result) {
	_loop.End(result?ModalLoop::ResultSucceeded:ModalLoop::ResultCancelled);
}

void DialogWnd::OnSize(const Area& ns) {
	Layout();
	Repaint();
}

LRESULT DialogWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_CLOSE) {
		EndModal(false);
		return 0;
	}
	return TopWnd::Message(msg,wp,lp);
}

void DialogWnd::Paint(graphics::Graphics& g, ref<Theme> theme) {
	Area buttons = TopWnd::GetClientArea();
	Pixels hh = ThemeManager::GetTheme()->GetMeasureInPixels(Theme::MeasureDialogHeaderHeight);
	buttons.Narrow(0,buttons.GetHeight()-hh, 0, 0);

	// Footer shadow
	SolidBrush backBr(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&backBr, buttons);
	theme->DrawInsetRectangle(g, buttons);
}

/* PropertyDialogWnd */
PropertyDialogWnd::PropertyDialogWnd(const std::wstring& title, const std::wstring& question): DialogWnd(title), _question(question) {
	_grid = GC::Hold(new PropertyGridWnd(false));
	Add(_grid, true);
	Layout();
}

PropertyDialogWnd::~PropertyDialogWnd() {
}

bool PropertyDialogWnd::HasQuestion() const {
	return _question.size() > 0;
}

void PropertyDialogWnd::SetSize(Pixels w, Pixels h) {
	if(_grid) _grid->SetNameWidth(Pixels(w*0.383f));
	DialogWnd::SetSize(w,h);
}

Area PropertyDialogWnd::GetClientArea() const {
	Area rc = TopWnd::GetClientArea();
	rc.Narrow(0,0,0,GetHeaderHeight());
	return rc;
}

Pixels PropertyDialogWnd::GetHeaderHeight() const {
	return HasQuestion() ? ThemeManager::GetTheme()->GetMeasureInPixels(Theme::MeasureDialogHeaderHeight) : 0;
}

void PropertyDialogWnd::Layout() {
	Area rc = GetClientArea();
	rc.Narrow(0,GetHeaderHeight(),0,0); // for question bar
	_grid->Fill(LayoutFill, rc);	
	DialogWnd::Layout();
}

void PropertyDialogWnd::OnAfterShowDialog() {
	// Focus first property
	_grid->FocusFirstProperty();
}

void PropertyDialogWnd::Paint(graphics::Graphics& g, ref<Theme> theme) {
	Area header = GetClientArea();
	header.SetHeight(GetHeaderHeight());

	// Header
	SolidBrush back(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&back, header);

	SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
	LinearGradientBrush headerBrush(PointF(0.0f, 0.0f), PointF(0.0f, float(GetHeaderHeight())), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
	g.FillRectangle(&headerBrush, header);
	SolidBrush tbr(theme->GetColor(Theme::ColorText));
	//SolidBrush stbr(theme->GetColor(Theme::ColorShadow));
	StringFormat sf;
	sf.SetAlignment(StringAlignmentNear);
	//g.DrawString(_question.c_str(), (int)_question.length(), theme->GetGUIFontBold(), PointF(5.0f, 7.0f), &sf, &stbr);
	g.DrawString(_question.c_str(), (int)_question.length(), theme->GetGUIFontBold(), PointF(4.0f, 6.0f), &sf, &tbr);
	DialogWnd::Paint(g,theme);
}

void PropertyDialogWnd::OnSize(const Area& ns) {
	Layout();
	DialogWnd::OnSize(ns);
}

ref<PropertyGridWnd> PropertyDialogWnd::GetPropertyGrid() {
	return _grid;
}

std::wstring Dialog::AskForSaveFile(ref<Wnd> owner, const std::wstring& title, const wchar_t* filter, const std::wstring& defExt) {
	#ifdef _WIN32
		wchar_t* fname = new wchar_t[MAX_PATH];
		memset(fname,0,sizeof(wchar_t)*MAX_PATH);
		OPENFILENAME fn;
		memset(&fn, 0, sizeof(OPENFILENAME));
		fn.lStructSize = sizeof(OPENFILENAME); 
		fn.hwndOwner = owner ? owner->GetWindow() : 0;
		fn.hInstance = GetModuleHandle(NULL);
		fn.lpstrFilter = filter;
		fn.lpstrFile = fname;
		fn.nMaxFile = 1023; 
		fn.Flags = OFN_HIDEREADONLY;
		fn.lpstrTitle = title.c_str();
		fn.lpstrDefExt = defExt.c_str();

		BOOL result = GetSaveFileName(&fn);

		std::wstring filename(fname);
		delete[] fname;
		return result?filename:L"";
	#else
		#error AskForSaveFile not implemented on this platform
	#endif
}

std::wstring Dialog::AskForOpenFile(ref<Wnd> owner, const std::wstring& title, const wchar_t* filter, const std::wstring& defExt) {
	#ifdef _WIN32
		wchar_t* fname = new wchar_t[MAX_PATH];
		memset(fname,0,sizeof(wchar_t)*MAX_PATH);
		OPENFILENAME fn;
		memset(&fn, 0, sizeof(OPENFILENAME));
		fn.lStructSize = sizeof(OPENFILENAME); 
		fn.hwndOwner = owner ? owner->GetWindow() : 0;
		fn.hInstance = GetModuleHandle(NULL);
		fn.lpstrFilter = filter;
		fn.lpstrFile = fname;
		fn.nMaxFile = 1023; 
		fn.Flags = OFN_HIDEREADONLY;
		fn.lpstrTitle = title.c_str();
		fn.lpstrDefExt = defExt.c_str();

		BOOL result = GetOpenFileName(&fn);

		std::wstring filename(fname);
		delete[] fname;
		return result?filename:L"";
	#else
		#error AskForSaveFile not implemented on this platform
	#endif
}