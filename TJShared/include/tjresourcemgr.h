#ifndef _TJRESOURCEMGR_H
#define _TJRESOURCEMGR_H

#pragma warning(push)
#pragma warning(disable: 4251)

class RootWnd;

class EXPORTED ResourceManager: public virtual Object {
	friend class Resource<ResourceManager>; // so it can call the destructor
	public:
		static ref<ResourceManager> Instance();

		/** Returns a full path to the specified resource.**/
		std::wstring Get(std::wstring identifier);
		void AddSearchPath(std::wstring path);
		void SetRootWindowForNotifications(RootWnd* rw);

	protected:
		ResourceManager();
		virtual ~ResourceManager();
		static ref<ResourceManager> _instance;
		std::vector<std::wstring> _paths;
		RootWnd* _root;
};


#pragma warning(pop)
#endif