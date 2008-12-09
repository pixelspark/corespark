#ifndef _TJUPDATERHTTP_H
#define _TJUPDATERHTTP_H

namespace tj {
	namespace updater {
		class URL {
			public:
				URL(const std::wstring& url);
				~URL();
				bool IsOK() const;
				const wchar_t* GetHostName() const;
				const wchar_t* GetPath() const;
				const wchar_t* GetExtra() const;
				bool IsSecure() const;
				unsigned short GetPort() const;

			protected:
				bool _ok;
				URL_COMPONENTS* _uc;
		};

		class HTTPRequest {
			public:
				HTTPRequest(const std::wstring& url, const std::wstring& ua, const std::wstring& method = L"GET");
				~HTTPRequest();
				bool Download(TiXmlDocument& doc);
				bool Download(const std::wstring& localFile);
				static bool DownloadToFile(const std::wstring& url, const std::wstring& localFile, const std::wstring& ua);

			private:
				bool CheckRequestStatusCode();
				bool _connected;
				HINTERNET _internet;
				HINTERNET _connection;
				HINTERNET _request;
				std::wstring _host;
		};
	}
}

#endif