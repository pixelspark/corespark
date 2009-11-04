#ifndef _TJRESOURCEMGR_H
#define _TJRESOURCEMGR_H

#include "internal/tjpch.h"
#include "tjutil.h"
#include <deque>

namespace tj {
	namespace shared {
		class RootWnd;
		typedef String ResourceIdentifier;

		/** Represents a resource **/
		class EXPORTED Resource {
			public:
				virtual ~Resource();
				virtual bool Exists() const = 0;
				virtual bool IsScript() const = 0;
				virtual Bytes GetSize() = 0;
				virtual ResourceIdentifier GetIdentifier() const = 0;

				virtual void Save(TiXmlElement* el);
		};

		/** A ResourceProvider can be seen as a 'converter' from resource identifier to a resource (or a local file path,
		which is the 'fast track' used in most cases) **/
		class EXPORTED ResourceProvider {
			public:
				virtual ~ResourceProvider();
				virtual ref<Resource> GetResource(const ResourceIdentifier& rid) = 0;
				virtual bool GetPathToLocalResource(const ResourceIdentifier& rid, String& path);
				virtual ResourceIdentifier GetRelative(const String& path) = 0;
		};

		class EXPORTED LocalFileResourceProvider: public ResourceProvider {
			public:
				LocalFileResourceProvider(const String& searchPath);
				virtual ~LocalFileResourceProvider();
				virtual ref<Resource> GetResource(const ResourceIdentifier& rid);
				virtual bool GetPathToLocalResource(const ResourceIdentifier& rid, String& path);
				virtual ResourceIdentifier GetRelative(const String& path);

			protected:
				String _searchPath;
		};

		class EXPORTED AbsoluteLocalFileResourceProvider: public ResourceProvider {
			public:
				AbsoluteLocalFileResourceProvider();
				virtual ~AbsoluteLocalFileResourceProvider();
				virtual ref<Resource> GetResource(const ResourceIdentifier& rid);
				virtual bool GetPathToLocalResource(const ResourceIdentifier& rid, String& path);
				virtual ResourceIdentifier GetRelative(const String& path);
		};

		/** The ResourceManager class maps between ResourceIdentifiers and paths. A resource identifier is an identifier
		that uniquely identifies a resource, but doesn't necessarily tell where that resource can be found. For example,
		'localfile.jpg' is a resource identifier, but 'http://www.file.com/file.txt' is a valid resource identifier too. **/
		class EXPORTED ResourceManager: public virtual Object, public ResourceProvider {
			public:
				ResourceManager();
				virtual ~ResourceManager();
				static strong<ResourceManager> Instance();

				/** Returns a full path to the specified resource.**/
				virtual bool GetPathToLocalResource(const ResourceIdentifier& identifier, String& path);
				virtual ref<Resource> GetResource(const ResourceIdentifier& rid);
				virtual ResourceIdentifier GetRelative(const String& path);
				
				virtual void AddProvider(strong<ResourceProvider> rp, bool upFront = false);
				virtual void RemoveProvider(strong<ResourceProvider> rp);

			protected:
				static ref<ResourceManager> _instance;
				std::deque< ref<ResourceProvider> > _paths;
		};

		class EXPORTED ResourceBundle: public virtual Object {
			public:
				ResourceBundle(strong<ResourceManager> mgr, strong<ResourceProvider> rp, bool addUpFront = false);
				virtual ~ResourceBundle();

			protected:
				strong<ResourceProvider> _rp;
				strong<ResourceManager> _mgr;
		};

		/** Represents a resource on the local filesystem **/
		class EXPORTED LocalFileResource: public Resource {
			public:
				LocalFileResource(const ResourceIdentifier& rid, const String& path);
				virtual ~LocalFileResource();
				String GetPath() const;
				
				virtual bool Exists() const;
				virtual bool IsScript() const;
				virtual String GetExtension() const;
				virtual void Open() const;
				virtual void OpenFolder() const;
				virtual Bytes GetSize();
				virtual ResourceIdentifier GetIdentifier() const;
				virtual void Save(TiXmlElement* el);
		
			protected:
				ResourceIdentifier _rid;
				String _path;
				Bytes _cachedSize;
		};
	}
}

#endif