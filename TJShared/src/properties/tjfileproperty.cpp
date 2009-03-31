#include "../../include/properties/tjproperties.h"
using namespace tj::shared;
using namespace tj::shared::graphics;

/* FilePropertyWnd */
FilePropertyWnd::FilePropertyWnd(const std::wstring& name, std::wstring* path, strong<ResourceProvider> rmg, const wchar_t* filter): _rmg(rmg), _name(name), _filter(filter), _path(path), _linkIcon(Icons::GetIconPath(Icons::IconFile)) {
	assert(path!=0);
	SetWantMouseLeave(true);
	SetDropTarget(true);
}

FilePropertyWnd::~FilePropertyWnd() {
}

void FilePropertyWnd::Paint(Graphics& g, strong<Theme> theme) {
	Area rc = GetClientArea();

	SolidBrush back(theme->GetColor(Theme::ColorBackground));
	g.FillRectangle(&back, rc);
	if(IsMouseOver()) {
		theme->DrawToolbarBackground(g, 0.0f, 0.0f, float(rc.GetWidth()), float(rc.GetHeight()));
	}

	_linkIcon.Paint(g, Area(0,0,16,16));

	// TODO fix with new resource system
	//SolidBrush tbr(File::Exists(ResourceManager::Instance()->Get(*_path, true))?theme->GetColor(Theme::ColorActiveStart):theme->GetColor(Theme::ColorCommandMarker));
	SolidBrush tbr(theme->GetColor(Theme::ColorActiveStart));
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
			_edit = null;
			Layout();
		}
		return 0;
	}

	return ChildWnd::Message(msg,wp,lp);
}

void FilePropertyWnd::OnMouse(MouseEvent me, Pixels x, Pixels y) {
	if(me==MouseEventLDown) {
		SetFile(Dialog::AskForOpenFile(ref<Wnd>(this), _name, _filter, L""));
	}
	else if(me==MouseEventMove||me==MouseEventLeave) {
		Repaint();
	}
	else if(me==MouseEventRDown) {
		enum { KCManualEntry=1, };
		ContextMenu cm;
		cm.AddItem(TL(file_property_manual_entry), KCManualEntry, false, _edit!=0);

		int r = cm.DoContextMenu(ref<Wnd>(this));
		if(r==KCManualEntry) {
			if(!_edit) {
				_edit = GC::Hold(new EditWnd());
				Add(_edit,true);
			}
			else {
				_edit->Show(false);
				_edit = null;
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
	Repaint();
}

void FilePropertyWnd::Layout() {
	if(_edit) {
		Area rc = GetClientArea();
		_edit->Fill(LayoutFill, rc);
	}
}

void FilePropertyWnd::SetFile(const std::wstring& file) {
	*_path = _rmg->GetRelative(file);
	Repaint();
}

/* FileProperty */
FileProperty::FileProperty(const std::wstring& name, ResourceIdentifier* path, strong<ResourceProvider> rmg, const wchar_t* filter): Property(name), _rmg(rmg), _path(path), _filter(filter) {
}

FileProperty::~FileProperty() {
}

ref<Wnd> FileProperty::GetWindow() {
	if(!_pw) {
		_pw = GC::Hold(new FilePropertyWnd(_name, _path, _rmg, _filter));
	}

	return _pw;
}

void FileProperty::Update() {
	if(_pw) {
		_pw->Update();
	}
}