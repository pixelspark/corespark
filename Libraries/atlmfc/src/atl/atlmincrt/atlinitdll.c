// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

//#include "StdAfx.H"
#pragma warning(disable : 4668)	// is not defined as a preprocessor macro, replacing with '0' for '#if/#elif
#pragma warning(disable : 4820)	// padding added after member
#pragma warning(disable : 4255)	// no function prototype given: converting '()' to '(void)'

#include <windows.h>
#include <process.h>

#pragma warning( disable: 4206 )

void DoInitialization();
void DoCleanup();

// Declare DllMain
BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD dwReason, LPVOID lpReserved);

#pragma warning(push)
#pragma warning(disable:4132)
/* _pRawDllMain is a communal, and it's really treated as a const pointer */
extern BOOL (WINAPI* const _pRawDllMain)(HINSTANCE, DWORD, LPVOID);
#pragma warning(pop)
extern BOOL (WINAPI* const _pDefaultRawDllMain)(HINSTANCE, DWORD, LPVOID) = NULL;
#if defined(_M_IX86)
#pragma comment(linker, "/alternatename:__pRawDllMain=__pDefaultRawDllMain")
#elif defined(_M_IA64) || defined(_M_AMD64)
#pragma comment(linker, "/alternatename:_pRawDllMain=_pDefaultRawDllMain")
#else
#error Unsupported platform
#endif

extern
BOOL __cdecl __DllMainCRTStartup(HINSTANCE hDllHandle, DWORD dwReason, LPVOID lpReserved);

/*
 * Make sure CALL/RET -> JMP optimization doesn't trigger in entrypoint, so
 * it will appear in callstacks.
 */
#pragma optimize("", off)

BOOL WINAPI _DllMainCRTStartup(HINSTANCE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		/*
		* The /GS security cookie must be initialized before any exception
		* handling targetting the current image is registered.  No function
		* using exception handling can be called in the current image until
		* after __security_init_cookie has been called.
		*/
		__security_init_cookie();
	}

	return __DllMainCRTStartup(hDllHandle, dwReason, lpReserved);
}

#pragma optimize("", on)

__declspec(noinline)
BOOL __cdecl __DllMainCRTStartup(HINSTANCE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
	BOOL bResult = TRUE;

	if ((dwReason == DLL_PROCESS_ATTACH) || (dwReason == DLL_THREAD_ATTACH))
	{
		if (_pRawDllMain != NULL)
		{
			bResult = _pRawDllMain(hDllHandle, dwReason, lpReserved);
			if (!bResult)
				return FALSE;
		}
	}

	if (dwReason == DLL_PROCESS_ATTACH)
	{
			DoInitialization();
	}

	bResult = DllMain(hDllHandle, dwReason, lpReserved);

	if (dwReason == DLL_PROCESS_DETACH)
	{
			DoCleanup();
	}

	if ((dwReason == DLL_PROCESS_DETACH) || (dwReason == DLL_THREAD_DETACH))
	{
		if (bResult && (_pRawDllMain != NULL))
			bResult = _pRawDllMain(hDllHandle, dwReason, lpReserved);
	}

	return bResult;
}

