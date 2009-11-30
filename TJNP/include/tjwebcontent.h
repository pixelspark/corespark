#ifndef _TJ_WEB_CONTENT_H
#define _TJ_WEB_CONTENT_H

#include "tjnpinternal.h"

#pragma warning(push)
#pragma warning(disable:4251 4275)

namespace tj {
	namespace np {
		class WebItem;

		enum Resolution {
			ResolutionNone = 0,
			ResolutionNotFound,
			ResolutionFile,
			ResolutionData,
			ResolutionEmpty,
		};

		class NP_EXPORTED WebRequest: public tj::shared::Object {
			public:
				virtual ~WebRequest();
				virtual const tj::shared::String& GetParameter(const std::string& parameter, const tj::shared::String& defaultValue) = 0;
				virtual const tj::shared::String& GetPath() const = 0;
		};

		class NP_EXPORTED WebItemWalker: public virtual tj::shared::Object {
			public:
				virtual ~WebItemWalker();
				virtual void Add(const tj::shared::String& prefix, tj::shared::ref<WebItem> wi, int level) = 0;
		};

		class NP_EXPORTED WebItem: public virtual tj::shared::Object {
			public:
				enum Permission {
					PermissionNone = 0,
					PermissionGet = 1,
					PermissionPropertyRead = 2,
					PermissionDelete = 4,
					PermissionPut = 8,
					PermissionPropertyWrite = 16,
				};

				virtual ~WebItem();
				virtual void Touch() = 0;
				virtual tj::shared::String GetETag() const = 0;
				virtual tj::shared::String GetDisplayName() const = 0;
				virtual tj::shared::String GetName() const = 0;
				virtual tj::shared::String GetContentType() const = 0;
				virtual unsigned int GetContentLength() const = 0;
				virtual bool IsCollection() const = 0;
				virtual void Walk(tj::shared::strong<WebItemWalker> wiw, const tj::shared::String& prefix, int level = -1) = 0;
				virtual tj::shared::ref<WebItem> Resolve(const tj::shared::String& file) = 0;
				virtual Resolution Get(tj::shared::ref<WebRequest> frq, tj::shared::String& error, char** data, unsigned int& dataLength) = 0;
				virtual tj::shared::Flags<Permission> GetPermissions() const = 0;
		};

		class NP_EXPORTED WebItemResource: public virtual WebItem {
			public:
				WebItemResource(const tj::shared::String& fn, const tj::shared::String& dn, const tj::shared::String& contentType, unsigned int length);
				WebItemResource(const tj::shared::String& contentType);
				virtual ~WebItemResource();
				virtual void Touch();
				virtual tj::shared::String GetETag() const;
				virtual tj::shared::String GetDisplayName() const;
				virtual tj::shared::String GetName() const;
				virtual tj::shared::String GetContentType() const;
				virtual tj::shared::Flags<Permission> GetPermissions() const;
				virtual unsigned int GetContentLength() const;
				virtual bool IsCollection() const;
				virtual tj::shared::ref<WebItem> Resolve(const tj::shared::String& file);
				virtual void Walk(tj::shared::strong<WebItemWalker> wiw, const tj::shared::String& prefix, int level = -1);

			protected:
				tj::shared::String _fn;
				tj::shared::String _dn;
				tj::shared::String _contentType;
				tj::shared::String _etag;
				unsigned int _length;
		};

		class NP_EXPORTED WebItemResolver: public WebItem {
			public:
				virtual ~WebItemResolver();
				virtual void Touch();
				virtual tj::shared::String GetETag() const;
				virtual tj::shared::String GetDisplayName() const;
				virtual tj::shared::String GetName() const;
				virtual tj::shared::String GetContentType() const;
				virtual unsigned int GetContentLength() const;
				virtual bool IsCollection() const;
				virtual void Walk(tj::shared::strong<WebItemWalker> wiw, const tj::shared::String& prefix, int level = -1);
				virtual Resolution Get(tj::shared::ref<WebRequest> frq, tj::shared::String& error, char** data, unsigned int& dataLength);
				virtual tj::shared::Flags<Permission> GetPermissions() const;
		};

		class NP_EXPORTED WebItemCollection: public WebItemResource {
			public:
				WebItemCollection(const tj::shared::String& fn, const tj::shared::String& dn, const tj::shared::String& contentType);
				virtual ~WebItemCollection();
				virtual bool IsCollection() const;
				virtual void Walk(tj::shared::strong<WebItemWalker> wiw, const tj::shared::String& prefix, int level = -1);
				virtual void Add(tj::shared::ref<WebItem> item);
				virtual tj::shared::ref<WebItem> Resolve(const tj::shared::String& file);
				virtual Resolution Get(tj::shared::ref<WebRequest> frq, tj::shared::String& error, char** data, unsigned int& dataLength);

			protected:
				std::deque< tj::shared::ref<WebItem> > _children;
		};
	}
}

#pragma warning(pop)

#endif