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
			public:
				static ref<ResourceManager> Instance();
				virtual ~ResourceManager();

				/** Returns a full path to the specified resource.**/
				std::wstring Get(const std::wstring& identifier, bool silent=false);
				std::wstring GetRelative(std::wstring path);
				void AddSearchPath(std::wstring path);
				void RemoveSearchPath(std::wstring path);
				void SetListener(ref<ResourceListener> listener);
				std::wstring GetSearchPath() const;

			protected:
				ResourceManager();
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