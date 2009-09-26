#ifndef _TJ_ZIPTYPES_H
#define _TJ_ZIPTYPES_H

#ifdef TJ_OS_WIN
	typedef DWORD ZRESULT;
	// return codes from any of the zip functions. Listed later.
	#ifndef HZIP_DECLARED
	#define HZIP_DECLARED
	DECLARE_HANDLE(HZIP);
	#endif
#else
	typedef int DWORD;
	typedef int ZRESULT;
	typedef void* HZIP;
	typedef char TCHAR;
#endif

#endif