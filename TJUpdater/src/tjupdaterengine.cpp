#include "../include/tjupdater.h"
#include <winhttp.h>
#include <deque>
#include <shlwapi.h>
using namespace tj::updater;

/** Update **/
Update::Update() {
}

Update::~Update() {
	// Delete downloaded temp files
	std::map<std::wstring, std::wstring>::const_iterator it = _resourcesDownloaded.begin();
	while(it!=_resourcesDownloaded.end()) {
		DeleteFile(it->second.c_str());
		++it;
	}
}

bool Update::GetDownloadedResource(const std::wstring& rid, std::wstring& path) const {
	std::map<std::wstring,std::wstring>::const_iterator it = _resourcesDownloaded.find(rid);
	if(it!=_resourcesDownloaded.end()) {
		path = it->second;
		return true;
	}
	return false;
}

void Update::Load(TiXmlElement* el) {
	_description = GetAttribute<std::wstring>(el, "description", _description);

	TiXmlElement* resources = el->FirstChildElement("resources");
	if(resources!=0) {
		TiXmlElement* res = resources->FirstChildElement("file");
		while(res!=0) {
			std::wstring id = GetAttribute<std::wstring>(res, "id", L"");
			std::wstring url = GetAttribute<std::wstring>(res, "href", L"");
			_resourcesNeeded[id] = url;
			res = res->NextSiblingElement("file");
		}
	}

	TiXmlElement* actions = el->FirstChildElement("actions");
	if(actions!=0) {
		_actions.Load(actions);
	}
}

bool Update::IsReadyToInstall() const {
	return _resourcesNeeded.size()==0;
}

void Update::DownloadResources() {
	wchar_t tempPath[MAX_PATH+2];
	GetTempPath(MAX_PATH, tempPath);

	std::map<std::wstring, std::wstring>::iterator it = _resourcesNeeded.begin();
	while(it!=_resourcesNeeded.end()) {
		const std::pair<std::wstring, std::wstring>& data = *it;

		wchar_t tempFileName[MAX_PATH+2];
		if(GetTempFileName(tempPath, L"TJU", 0, tempFileName)==0) {
			UpdaterLog::Write(L"GetTempFileName failed");
		}
		else {
			if(!HTTPRequest::DownloadToFile(data.second, tempFileName, UpdaterSettings::GetSetting(L"updater.http.user-agent", L"TJUpdater-1.0"))) {
				DeleteFile(tempFileName);
				UpdaterLog::Write(L"Could not download file from URL "+data.second+L" to "+std::wstring(tempFileName));
			}
			else {
				_resourcesDownloaded[data.first] = tempFileName;
				UpdaterLog::Write(L"Downloaded file from "+data.second+L" to "+std::wstring(tempFileName));
			}
		}
		++it;
	}
	_resourcesNeeded.clear();
}

bool Update::Install() {
	return _actions.Run(*this);
}

const std::wstring& Update::GetDescription() const {
	return _description;
}

/** UpdatableComponent **/
UpdatableComponent::UpdatableComponent(const std::wstring& name, const std::wstring& versionSource): _name(name), _versionSource(versionSource) {
	CalculateCurrentVersion();
}

UpdatableComponent::~UpdatableComponent() {
}

const std::wstring& UpdatableComponent::GetName() const {
	return _name;
}

const std::wstring& UpdatableComponent::GetVersion() const {
	return _versionHash;
}

void UpdatableComponent::Load(TiXmlElement* el) {
	_name = GetAttribute<std::wstring>(el, "name", L"");
	_versionSource = GetAttribute<std::wstring>(el, "version-source", L"");
	_updateURL = GetAttribute<std::wstring>(el, "url", L"");
	CalculateCurrentVersion();
}

bool UpdatableComponent::CanBeUpdated() const {
	return _versionHash.length() > 0 && _updateURL.length() > 0;
}

bool UpdatableComponent::UpdatesAvailable() const {
	return _updates.size() > 0;
}

void UpdatableComponent::CalculateCurrentVersion() {
	if(GetFileAttributes(_versionSource.c_str())!=INVALID_FILE_ATTRIBUTES) {
		MD5HashFile(_versionSource, _versionHash);
	}
	else {
		_versionHash = L"";
	}
}

void UpdatableComponent::DownloadResourcesForUpdates() {
	std::deque<Update>::iterator it = _updates.begin();
	while(it!=_updates.end()) {
		Update& upd = *it;
		upd.DownloadResources();
		++it;
	}
}

void UpdatableComponent::FindAvailableUpdates() {
	UpdaterLog::Write(L"FindAvailableUpdates for component "+GetName());

	if(!CanBeUpdated()) {
		UpdaterLog::Write(L"Component cannot be updated, aborting");
		return;
	}

	std::wostringstream finalURL;
	std::wstring mac;
	Machine::GetUniqueIdentifier(mac);
	finalURL << _updateURL << L"?name=" << GetName() << L"&version=" << GetVersion() << L"&machine=" << mac;

	HTTPRequest request(finalURL.str(), UpdaterSettings::GetSetting(L"updater.http.user-agent", L"TJUpdater-1.0"));
	
	// Retrieve update file from server
	TiXmlDocument doc;
	if(request.Download(doc)) {
		TiXmlElement* root = doc.FirstChildElement();
		if(root!=0) {
			TiXmlElement* update = root->FirstChildElement("update");
			while(update!=0) {
				// Check if our version is among the target 'for' versions
				bool isUpdateForThisVersion = false;
				TiXmlElement* updateFor = update->FirstChildElement("for");
				while(updateFor!=0) {
					std::wstring version = GetAttribute<std::wstring>(updateFor, "version", L"");
					std::wstring name = GetAttribute<std::wstring>(updateFor, "name", L"");
					UpdaterLog::Write(L"- Available update for-name="+name+L" for-version="+version);
					if(version==GetVersion() && name==GetName()) {
						isUpdateForThisVersion = true;
						UpdaterLog::Write(L"=> Matches installed component, suitable update");
						break;
					}
					updateFor = updateFor->NextSiblingElement("for");
				}

				if(isUpdateForThisVersion) {
					Update upd;
					upd.Load(update);
					_updates.push_back(upd);
				}
				update = update->NextSiblingElement("update");
			}
		}
	}
	else {
		UpdaterLog::Write(L"Download failed");
	}
}

/** Updater **/
Updater::Updater() {
}

Updater::~Updater() {
}

void Updater::Load(TiXmlElement* root) {
	TiXmlElement* update = root->FirstChildElement("update");
	while(update!=0) {
		UpdatableComponent uc;
		uc.Load(update);
		_components.push_back(uc);
		update = update->NextSiblingElement("update");
	}
}

void Updater::FindAvailableUpdates() {
	std::deque<UpdatableComponent>::iterator it = _components.begin();
	while(it!=_components.end()) {
		UpdatableComponent& uc = *it;
		uc.FindAvailableUpdates();
		++it;
	}
}

void Updater::DownloadResources() {
	std::deque<UpdatableComponent>::iterator it = _components.begin();
	while(it!=_components.end()) {
		UpdatableComponent& uc = *it;
		uc.DownloadResourcesForUpdates();
		++it;
	}
}

bool Updater::InstallUpdates() {
	bool anyFailed = false;
	std::deque<UpdatableComponent>::iterator it = _components.begin();
	while(it!=_components.end()) {
		UpdatableComponent& uc = *it;
		if(uc.CanBeUpdated()) {
			std::wostringstream wos;
			wos << L"Updating component " << uc.GetName() << L" (" << uc.GetVersion() << L") " << (uc.CanBeUpdated() ? L"updatable" : L"not-updatable") << std::endl;	
			UpdaterLog::Write(wos.str());

			std::deque<Update>::iterator uit = uc._updates.begin();
			while(uit!=uc._updates.end()) {
				Update& upd = *uit;
				if(upd.IsReadyToInstall()) {
					UpdaterLog::Write(L"- Installing update "+upd.GetDescription());
					if(!upd.Install()) {
						UpdaterLog::Write(L"  FAILED!");
						anyFailed = true;
					}
					else {
						UpdaterLog::Write(L"  Succeeded");
					}
				}
				++uit;
			}
		}		
		++it;
	}

	return !anyFailed;
}

void Updater::Dump() {
	std::wostringstream wos;
	std::deque<UpdatableComponent>::const_iterator it = _components.begin();
	while(it!=_components.end()) {
		const UpdatableComponent& uc = *it;
		wos << L"- " << uc.GetName() << L" (" << uc.GetVersion() << L") " << (uc.CanBeUpdated() ? L"updatable" : L"not-updatable") << std::endl;
		if(uc.CanBeUpdated() && uc.UpdatesAvailable()) {
			std::deque<Update>::const_iterator uit = uc._updates.begin();
			while(uit!=uc._updates.end()) {
				const Update& upd = *uit;
				wos << L"\tUpdate " << upd.GetDescription() << std::endl;
				++uit;
			}
			wos << std::endl;
		}
		++it;
	}

	UpdaterLog::Write(wos.str());
}

bool Updater::UpdatesAvailable() const {
	std::deque<UpdatableComponent>::const_iterator it = _components.begin();
	while(it!=_components.end()) {
		const UpdatableComponent& uc = *it;
		if(uc.UpdatesAvailable()) {
			return true;
		}
		++it;
	}
	return false;
}

/** UpdateActions **/
UpdateActions::UpdateActions() {
}

UpdateActions::~UpdateActions() {
}

bool UpdateActions::Run(const Update& parent) {
	std::deque<UpdateAction>::iterator it = _actions.begin();
	while(it!=_actions.end()) {
		UpdateAction& ua = *it;
		if(!ua.Run(parent)) {
			return false;
		}
		++it;
	}
	return true;
}

void UpdateActions::Load(TiXmlElement* actions) {
	TiXmlElement* action = actions->FirstChildElement("action");
	while(action!=0) {
		_actions.push_back(UpdateAction(action));
		action = action->NextSiblingElement("action");
	}
}

/** UpdateAction **/
UpdateActions::UpdateAction::UpdateAction(TiXmlElement* action): _type(ActionNothing) {
	std::wstring typeString = GetAttribute<std::wstring>(action, "type", L"");
	if(typeString==L"run") {
		_type = ActionRun;
	}

	_file = GetAttribute<std::wstring>(action, "source", L"");
	_parameters = GetAttribute<std::wstring>(action, "parameters", L"");
}

UpdateActions::UpdateAction::UpdateAction(ActionType at, const std::wstring& file): _type(at), _file(file) {
}

bool UpdateActions::UpdateAction::Run(const Update& upd) {
	if(_type==ActionRun) {
		UpdaterLog::Write(L"- Run "+_file);
		std::wstring path;
		if(upd.GetDownloadedResource(_file, path)) {
			UpdaterLog::Write(L"  File resolves to "+path);
			
			// Rename the file to a .exe file
			wchar_t* pathChanged = new wchar_t[path.length()+6];
			wcscpy_s(pathChanged, path.length()+6, path.c_str());
			wchar_t* extension = PathFindExtension(pathChanged);
			wcscpy_s(extension, 5, L".exe");
			UpdaterLog::Write(std::wstring(L"Changed path: ") + pathChanged);
			if(!MoveFile(path.c_str(), pathChanged)) {
				UpdaterLog::Write(L"Could not move file to have a .exe extension");
				delete[] pathChanged;
				return false;
			}

			// Execute the file and wait
			SHELLEXECUTEINFO shi;
			memset(&shi, 0, sizeof(shi));
			shi.cbSize = sizeof(shi);
			shi.fMask = SEE_MASK_NOCLOSEPROCESS;
			shi.lpVerb = L"open";
			shi.lpFile = pathChanged;
			shi.lpParameters = _parameters.c_str();
			shi.nShow = SW_SHOW;
			if(!ShellExecuteEx(&shi)) {
				UpdaterLog::Write(L"Could not ShellExecute installler; on Vista: user might have disallowed UAC");
			}
			else {
				if(shi.hProcess!=NULL) {
					UpdaterLog::Write(std::wstring(L"Process was launched (parameters were '")+_parameters+L"') let's wait for it to finish.");
					WaitForSingleObject(shi.hProcess, INFINITE);
					CloseHandle(shi.hProcess);
					UpdaterLog::Write(L"Process finished");
				}
			}

			// Move the file back, since another action may have to use it
			MoveFile(pathChanged, path.c_str());
			delete[] pathChanged;
		}
	}
	return true;
}