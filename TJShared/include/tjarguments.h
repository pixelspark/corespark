#ifndef _TJARGUMENTS_H
#define _TJARGUMENTS_H

namespace tj {
	namespace shared {
		/** Command-line argument parser. **/
		class EXPORTED Arguments: public virtual Object {
			public:
				Arguments(); 
				virtual ~Arguments();
				std::vector<wchar_t*>* GetOptions();
				bool IsSet(const std::wstring& option);

			protected:
				void Parse();
				wchar_t* m_cmdline;

				/* Gives C4251 in VC++: _options needs to have dll-interface to be used by clients. Since
				_options is protected, it should and normally cannot be used by  'clients' */
				std::vector<wchar_t*> _options;
		};
	}
}

#endif