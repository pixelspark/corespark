/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
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
				bool CheckRequestStatusDataReader();
				bool _connected;
				HINTERNET _internet;
				HINTERNET _connection;
				HINTERNET _request;
				std::wstring _host;
		};
	}
}

#endif