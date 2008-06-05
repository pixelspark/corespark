#include "../../include/tjshared.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

/* FilePropertyWnd */
FilePropertyWnd::FilePropertyWnd(std::wstring name, std::wstring* path, const wchar_t* filter): ChildWnd(L""), _name(name), _filter(filter), _path(path), _linkIcon(Icons::GetIconPath(Icons::IconFile)) {
	assert(path!=0);
	SetWantMouseLeave(true);
	SetDropTarget(true);
}

FilePropertyWnd::~FilePropertyWnd() {
}

void FilePropertyWnd::Paint(Graphics& g, ref<Theme> theme) {
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

LRESULT FilePropertyWnd::Message(UINT msg, WPARAM wp, LPARAM lp) {
	if(msg==WM_COMMAND) {
		// if we have an edit box, update its value
		if(_edit) {
			*_path = _edit->GetText();
		}
		if(HIWORD(wp)==EN_KILLFOCUS) {
			_edit->Show(false);
			_edit = 0;
			Layout();
		}
		return 0;
	}

	return ChildWnd::Message(msg,wp,lp);
}

void FilePropertyWnd::OnMouse(MouseEvent me, Pixels x, Pixels y) {
	if(me==MouseEventLDown) {
		SetFile(Dialog::AskForOpenFile(GetWindow(), _name, _filter, L""));
	}
	else if(me==MouseEventMove||me==MouseEventLeave) {
		Repaint();
	}
	else if(me==MouseEventRDown) {
		enum { KCManualEntry=1, };
		ContextMenu cm;
		cm.AddItem(TL(file_property_manual_entry), KCManualEntry, false, _edit!=0);

		int r = cm.DoContextMenu(this);
		if(r==KCManualEntry) {
			if(!_edit) {
				_edit = GC::Hold(new EditWnd());
				Add(_edit,true);
			}
			else {
				_edit->Show(false);
				_edit = 0;
			}
			Update();
			Layout();
		}
	}
}

void FilePropertyWnd::Update() {
	if(_edit) {
		_edit->SetText(*_path);
	}
}

void FilePropertyWnd::OnSize(const Area& ns) {
	Layout();	
}

void FilePropertyWnd::Layout() {
	if(_edit) {
		Area rc = GetClientArea();
		_edit->Fill(LayoutFill, rc);
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