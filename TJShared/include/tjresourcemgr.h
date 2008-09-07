#ifndef _TJRESOURCEMGR_H
#define _TJRESOURCEMGR_H

namespace tj {
	namespace shared {
		class RootWnd;
		typedef std::wstring ResourceIdentifier;

		class EXPORTED ResourceListener: public virtual Object {
			public:
				virtual ~ResourceListener();
				virtual std::wstring OnResourceNotFound(std::wstring ident) = 0;
		};

		/** The ResourceManager class maps between ResourceIdentifiers and paths. A resource identifier is an identifier
		that uniquely identifies a resource, but doesn't necessarily tell where that resource can be found. For example,
		'localfile.jpg' is a resource identifier, but 'http://www.file.com/file.txt' is a valid resource identifier too. **/
		class EXPORTED ResourceManager: public virtual Object {
			public:
				static strong<ResourceManager> Instance();
				virtual ~ResourceManager();

				/** Returns a full path to the specified resource.**/
				std::wstring Get(const ResourceIdentifier& identifier, bool silent = false);
				ResourceIdentifier GetRelative(const std::wstring& path);
				void AddSearchPath(const std::wstring& path);
				void RemoveSearchPath(const std::wstring& path);
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
				ResourceBundle(ref<ResourceManager> mgr, const std::wstring& path);
				virtual ~ResourceBundle();

			protected:
				std::wstring _path;
				ref<ResourceManager> _mgr;
		};

		/** Represents a resource **/
		class EXPORTED Resource {
			public:
				virtual ~Resource();
				virtual bool Exists() const = 0;
				virtual bool IsScript() const = 0;
				virtual Bytes GetSize() = 0;
				virtual ResourceIdentifier GetIdentifier() const = 0;

				virtual void Save(TiXmlElement* el) = 0;
				static ref<Resource> LoadResource(TiXmlElement* serialized);
		};

		/** Represents a resource on the local filesystem **/
		class EXPORTED LocalFileResource: public Resource {
			public:
				LocalFileResource(const ResourceIdentifier& rid);
				LocalFileResource(TiXmlElement* ti);
				virtual ~LocalFileResource();
				std::wstring GetPath() const;
				
				virtual bool Exists() const;
				virtual bool IsScript() const;
				virtual std::wstring GetExtension() const;
				virtual void Open() const;
				virtual void OpenFolder() const;
				virtual Bytes GetSize();
				virtual ResourceIdentifier GetIdentifier() const;
				virtual void Save(TiXmlElement* el);
		
			protected:
				ResourceIdentifier _rid;
				Bytes _cachedSize;
		};
	}
}

#endif