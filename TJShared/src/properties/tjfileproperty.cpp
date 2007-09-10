#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace Gdiplus;

/* FilePropertyWnd */
FilePropertyWnd::FilePropertyWnd(std::wstring name, std::wstring* path, const wchar_t* filter): ChildWnd(L""), _name(name), _filter(filter), _path(path), _linkIcon(L"icons/shared/file.png") {
	assert(path!=0);
	SetWantMouseLeave(true);
	SetDropTarget(true);
}

FilePropertyWnd::~FilePropertyWnd() {
}

void FilePropertyWnd::Paint(Graphics& g) {
	ref<Theme> theme = ThemeManager::GetTheme();
	Area rc = GetClientArea();

	SolidBrush back(theme->GetBackgroundColor());
	g.FillRectangle(&back, rc);
	if(IsMouseOver()) {
		theme->DrawToolbarBackground(g, 0.0f, 0.0f, float(rc.GetWidth()), float(rc.GetHeight()));
	}

	g.DrawImage(_linkIcon, PointF(0.0f, 0.0f));

	SolidBrush tbr(File::Exists(ResourceManager::Instance()->Get(*_path, true))?theme->GetActiveStartColor():theme->GetCommandMarkerColor());
	Area text = rc;
	text.Narrow(20,2,0,0);
	StringFormat sf;
	sf.SetTrimming(StringTrimmingEllipsisPath);
	g.DrawString(_path->c_str(), (int)_path->length(), theme->GetGUIFont(), text, &sf, &tbr);
}

void FilePropertyWnd::OnDropFiles(const std::vector< std::wstring >& files) {
	if(files.size()>0) {
		SetFile(files.at(0));
	}
}

void FilePropertyWnd::OnMouse(MouseEvent me, Pixels x, Pixels y) {
	if(me==MouseEventLDown) {
		SetFile(Dialog::AskForOpenFile(GetWindow(), _name, _filter, L""));
	}
	else if(me==MouseEventMove||me==MouseEventLeave) {
		Repaint();
	}
}

void FilePropertyWnd::SetFile(const std::wstring& file) {
	*_path = file;
	ref<ResourceManager> rm = ResourceManager::Instance();

	if(rm) {
		*_path = rm->GetRelative(*_path);
	}
	Repaint();
}

/* FileProperty */
FileProperty::FileProperty(std::wstring name, std::wstring* path, const wchar_t* filter): Property(name), _path(path), _filter(filter) {
}

FileProperty::~FileProperty() {
}

HWND FileProperty::GetWindow() {
	if(!_pw) {
		return 0;
	}
	return _pw->GetWindow();
}

std::wstring FileProperty::GetValue() {
	return *_path;
}

HWND FileProperty::Create(HWND p) {
	if(!_pw) {
		_pw = GC::Hold(new FilePropertyWnd(_name, _path, _filter));
		SetParent(_pw->GetWindow(), p);
		_pw->SetStyle(WS_CHILD);
	}

	return _pw->GetWindow();
}

void FileProperty::Changed() {
}

void FileProperty::Update() {
	if(_pw) _pw->Update();
}