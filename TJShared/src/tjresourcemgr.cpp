#include "../include/tjshared.h"
using namespace tj::shared;

namespace tj {
	namespace shared {
		class AddNotificationRunnable: public Runnable {
			public:
				AddNotificationRunnable(RootWnd* root, std::wstring text, std::wstring icon, int time) {
					_root = root;
					_text = text;
					_icon = icon;
					_time = time;
				}
				
				virtual ~AddNotificationRunnable() {
				
				}
				
				virtual void Run() {
					if(_root!=0) {
						_root->AddNotification(_text, _icon, _time);
					}
				}

			protected:
				RootWnd* _root;
				std::wstring _text;
				std::wstring _icon;
				int _time;
		};
	}
}

ref<ResourceManager> ResourceManager::_instance;

ResourceManager::ResourceManager() {
	/** Add location of tjshared.dll to the search list **/
	wchar_t* buf = new wchar_t[MAX_PATH];
	memset(buf,0,sizeof(wchar_t)*MAX_PATH);
	GetModuleFileName(GetModuleHandle(NULL), buf, MAX_PATH);
	PathRemoveFileSpec(buf);
	_paths.push_back(std::wstring(buf));
	_root = 0;
}

ResourceManager::~ResourceManager() {
}

void ResourceManager::AddSearchPath(std::wstring path) {
	_paths.push_back(path);
}

void ResourceManager::SetRootWindowForNotifications(RootWnd* rw) {
	_root = rw;
}

std::wstring ResourceManager::Get(std::wstring ident) {
	Log::Write(L"TJShared/ResourceManager", ident);
	std::vector<std::wstring>::iterator it = _paths.begin();
	while(it!=_paths.end()) {
		std::wstring path = *it + L"\\"+ident;

		// file exists
		if(GetFileAttributes(path.c_str())!=INVALID_FILE_ATTRIBUTES) {
			return path;
		}
		it++;
	}

	if(GetFileAttributes(ident.c_str())!=INVALID_FILE_ATTRIBUTES) {
		return ident;
	}

	// Add a notification, the threadsafe way©
	ref<Runnable> rn = GC::Hold(new AddNotificationRunnable(_root, TL(resource_not_found)+ident, L"icons/notifications/error.png", NotificationWnd::NotificationTimeoutDefault));
	Core::Instance()->AddAction(rn);

	return L"";
}

ref<ResourceManager> ResourceManager::Instance() {
	if(!_instance) {
		_instance = GC::Hold(new ResourceManager());
	}

	return _instance;
}