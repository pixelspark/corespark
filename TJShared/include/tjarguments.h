#ifndef _TJARGUMENTS_H
#define _TJARGUMENTS_H

#include <vector>
#pragma warning(push)
#pragma warning(disable: 4251)

/** Command-line argument parser. **/
class EXPORTED Arguments {
	public:
		Arguments(); 
		virtual ~Arguments();
		std::vector<wchar_t*>* GetOptions();

	protected:
		void Parse();
		wchar_t* m_cmdline;

		/* Gives C4251 in VC++: _options needs to have dll-interface to be used by clients. Since
		_options is protected, it should and normally cannot be used by  'clients' */
		std::vector<wchar_t*> _options;
};

#pragma warning(pop)
#endif