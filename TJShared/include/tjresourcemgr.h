#ifndef _TJRESOURCEMGR_H
#define _TJRESOURCEMGR_H

namespace tj {
	namespace shared {
		class RootWnd;

		class EXPORTED ResourceListener: public virtual Object {
			public:
				virtual ~ResourceListener();
				virtual std::wstring OnResourceNotFound(std::wstring ident) = 0;
		};

		class EXPORTED ResourceManager: public virtual Object {
			friend class tj::shared::intern::Resource<ResourceManager>; // so it can call the destructor

			public:
				static ref<ResourceManager> Instance();

				/** Returns a full path to the specified resource.**/
				std::wstring Get(std::wstring identifier, bool silent=false);
				void AddSearchPath(std::wstring path);
				void RemoveSearchPath(std::wstring path);
				void SetListener(ref<ResourceListener> listener);

			protected:
				ResourceManager();
				virtual ~ResourceManager();
				static ref<ResourceManager> _instance;
				std::vector<std::wstring> _paths;
				ref<ResourceListener> _listener;
		};

		class EXPORTED ResourceBundle: public virtual Object {
			public:
				ResourceBundle(ref<ResourceManager> mgr, std::wstring path);
				virtual ~ResourceBundle();

			protected:
				std::wstring _path;
				ref<ResourceManager> _mgr;
		};
	}
}

#endif