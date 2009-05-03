// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#include "StdAfx.H"
#include "atlmincrt.h"
#include "sal.h"

#pragma warning(disable : 4711)	// function selected for automatic inline expansion
#if _SECURE_ATL
#pragma warning(disable : 4706)	// function selected for automatic inline expansion
#endif

static HANDLE g_hProcessHeap = NULL;

extern "C" _PVFV* __onexitbegin = NULL;
extern "C" _PVFV* __onexitend = NULL;

extern "C" _PVFV __xi_a[], __xi_z[];    /* C initializers */
extern "C" _PVFV __xc_a[], __xc_z[];    /* C++ initializers */
extern "C" _PVFV __xp_a[], __xp_z[];    /* C pre-terminators */
extern "C" _PVFV __xt_a[], __xt_z[];    /* C terminators */

// Critical section to protect initialization/exit code
static CRITICAL_SECTION g_csInit;

extern "C"
{
unsigned int _osplatform;
}

static 	OSVERSIONINFO osi;

extern "C" void DoInitialization()
{
	_PVFV* pf;

	memset(&osi, 0, sizeof(OSVERSIONINFO));
	osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osi);
	_osplatform = osi.dwPlatformId;


	InitializeCriticalSection( &g_csInit );

	EnterCriticalSection( &g_csInit );

	__try
	{
		g_hProcessHeap = GetProcessHeap();

		// Call initialization routines (contructors for globals, etc.)
		for(pf = __xi_a; pf < __xi_z; pf++)
		{
			if(*pf != NULL)
			{
				(**pf)();
			}
		}

		for(pf = __xc_a; pf < __xc_z; pf++)
		{
			if(*pf != NULL)
			{
				(**pf)();
			}
		}
	}
	__finally
	{
		LeaveCriticalSection(&g_csInit);
	}
}

extern "C" void DoCleanup()
{
	_PVFV* pf;

	EnterCriticalSection(&g_csInit);  // Protect access to the atexit table

	__try
	{
		// Call routines registered with atexit() from most recently registered
		// to least recently registered
		if(__onexitbegin != NULL)
		{
			for(pf = __onexitend-1; pf >= __onexitbegin; pf--)
			{
				if (*pf != NULL)
					(**pf)();
			}
		}

		free(__onexitbegin);
		__onexitbegin = NULL;
		__onexitend = NULL;

		for(pf = __xp_a; pf < __xp_z; pf++)
		{
			if( *pf != NULL )
			{
				(**pf)();
			}
		}

		for(pf = __xt_a; pf < __xt_z; pf++)
		{
			if( *pf != NULL )
			{
				(**pf)();
			}
		}
	}
	__finally
	{
		LeaveCriticalSection(&g_csInit);
		DeleteCriticalSection(&g_csInit);		
	}
}

/////////////////////////////////////////////////////////////////////////////
// Heap Allocation

void* __cdecl malloc(size_t n)
{
	return (void*) HeapAlloc(g_hProcessHeap, 0, n);
}

void* __cdecl calloc(size_t n, size_t s)
{
	void* p;

	if(n!=0 && SIZE_MAX/n<s)
	{
		return NULL;
	}
	p = malloc(n*s);
	if(p != NULL)
		memset(p, 0, n*s);

	return p;
}

size_t __cdecl _msize(void*p)
{
	return HeapSize(g_hProcessHeap, 0, p);
}

void* __cdecl realloc(void* p, size_t n)
{
	if (p == NULL)
		return malloc(n);
	else if (n == 0)
	{
		free(p);
		return NULL;
	}
	else
		return HeapReAlloc(g_hProcessHeap, 0, p, n);
}

void* __cdecl _recalloc(void* p, size_t n, size_t c)
{
	if(n!=0 && SIZE_MAX/n<c)
	{
		return NULL;
	}
	return realloc(p, n*c);
}

void __cdecl free(void* p)
{
	if (p != NULL)
		HeapFree(g_hProcessHeap, 0, p);
}

void* __cdecl operator new(size_t n)
{
	return malloc(n);
}

void* __cdecl operator new[](size_t n)
{
	return operator new(n);
}

void __cdecl operator delete(void* p)
{
	free(p);
}

void __cdecl operator delete[](void* p)
{
	operator delete(p);
}

int __cdecl atexit(_PVFV pf)
{
	size_t nCurrentSize;
	int nRet = 0;	

	EnterCriticalSection(&g_csInit);

	__try
	{
		if(__onexitbegin == NULL)
		{
			__onexitbegin = (_PVFV*)calloc(16,sizeof(_PVFV));
			if(__onexitbegin == NULL)
			{
				LeaveCriticalSection(&g_csInit);
				return(-1);
			}
			__onexitend = __onexitbegin;
		}

		nCurrentSize = _msize(__onexitbegin);
		if((nCurrentSize+sizeof(_PVFV)) < ULONG(((const BYTE*)__onexitend-
			(const BYTE*)__onexitbegin)))
		{
			_PVFV* pNew;

			pNew = (_PVFV*)_recalloc(__onexitbegin, 2,nCurrentSize);
			if(pNew == NULL)
			{
				LeaveCriticalSection(&g_csInit);	  
				return(-1);
			}
		}

		*__onexitend = pf;
		__onexitend++;
	}
	__except (1)
	{
		nRet = -1;
	}

	LeaveCriticalSection(&g_csInit);	

	return(nRet);
}

int __cdecl _purecall()
{
	DebugBreak();
	return 0;
}

extern "C" const int _fltused=0;

///////////////////////////////////////////////////////////////////////////////
// Secure CRT 

#ifndef _ERRCODE_DEFINED
#define _ERRCODE_DEFINED
/* errno_t is deprecated in favor or errno_t, which is part of the standard proposal */
__declspec(deprecated) typedef int errcode;
typedef int errno_t;
#endif//_ERRCODE_DEFINED

#ifndef _VALIDATE_COMPONENT_RETURN
#define _VALIDATE_COMPONENT_RETURN(exp, exp_len)          \
        if(exp)                                           \
        {                                                 \
            if(exp_len == 0)                              \
                goto err_return;                          \
            exp[0] = 0;                                   \
        }
#endif

#ifndef _VALIDATE_RETURN_ERRCODE
#define _VALIDATE_RETURN_ERRCODE( expr, errorcode )       \
    {                                                     \
        _ASSERTE( ( expr ) );                             \
        if ( !( expr ) )                                  \
        {                                                 \
            errno = errorcode;                            \
            return ( errorcode );                         \
        }                                                 \
    }    
#endif

#ifndef _VALIDATE_RETURN
#define _VALIDATE_RETURN( expr, errorcode, retexpr )      \
    {                                                     \
        _ASSERTE( ( expr ) );                             \
        if ( !( expr ) )                                  \
        {                                                 \
            errno = errorcode;                            \
            return ( retexpr );                           \
        }                                                 \
    }
#endif

#ifndef _VALIDATE_RETURN_NOERRNO
#define _VALIDATE_RETURN_NOERRNO( expr, retexpr )         \
    {                                                     \
        _ASSERTE( ( expr ) );                             \
        if ( !( expr ) )                                  \
        {                                                 \
            return ( retexpr );                           \
        }                                                 \
    }
#endif 

/* We completely fill the buffer only in debug. */
#if !defined(_FILL_BUFFER)
#ifdef _DEBUG
#define _FILL_BUFFER 1
#else
#define _FILL_BUFFER 0
#endif
#endif

/* __FILL_BUFFER_PATTERN is the same as _bNoMansLandFill */
#define _FILL_BUFFER_PATTERN 0xFD

#if _FILL_BUFFER
#define _FILL_STRING(_String, _Size, _Offset)                                       \
    if ((_Size) != ((size_t)-1) && (_Size) != INT_MAX &&                            \
        ((size_t)(_Offset)) < (_Size))                                              \
    {                                                                               \
        memset((_String) + (_Offset),                                               \
            _FILL_BUFFER_PATTERN,                                                   \
            ((_Size) - (_Offset)) * sizeof(*(_String)));                            \
    }
#else
#define _FILL_STRING(_String, _Size, _Offset)
#endif

#define _RESET_STRING(_String, _Size) \
    *(_String) = 0; \
    _FILL_STRING((_String), (_Size), 1);

#define _VALIDATE_STRING_ERROR(_String, _Size, _Ret) \
    _VALIDATE_RETURN((_String) != NULL && (_Size) > 0, EINVAL, (_Ret))

#define _VALIDATE_STRING(_String, _Size) \
    _VALIDATE_STRING_ERROR((_String), (_Size), EINVAL)

#define _VALIDATE_POINTER_ERROR_RETURN(_Pointer, _ErrorCode, _Ret) \
    _VALIDATE_RETURN((_Pointer) != NULL, (_ErrorCode), (_Ret))

#define _VALIDATE_POINTER_ERROR(_Pointer, _Ret) \
    _VALIDATE_POINTER_ERROR_RETURN((_Pointer), EINVAL, (_Ret))

#define _VALIDATE_POINTER(_Pointer) \
    _VALIDATE_POINTER_ERROR((_Pointer), EINVAL)

#define _VALIDATE_POINTER_RESET_STRING_ERROR(_Pointer, _String, _Size, _Ret) \
    if ((_Pointer) == NULL) \
    { \
        _RESET_STRING((_String), (_Size)); \
        _VALIDATE_POINTER_ERROR_RETURN((_Pointer), EINVAL, (_Ret)) \
    }

#define _VALIDATE_POINTER_RESET_STRING(_Pointer, _String, _Size) \
    _VALIDATE_POINTER_RESET_STRING_ERROR((_Pointer), (_String), (_Size), EINVAL)

#define _RETURN_BUFFER_TOO_SMALL_ERROR(_String, _Size, _Ret) \
    _VALIDATE_RETURN((L"Buffer is too small" && 0), ERANGE, _Ret)

#define _RETURN_BUFFER_TOO_SMALL(_String, _Size) \
    _RETURN_BUFFER_TOO_SMALL_ERROR((_String), (_Size), ERANGE)

#define _RETURN_DEST_NOT_NULL_TERMINATED(_String, _Size) \
    _VALIDATE_RETURN((L"String is not null terminated" && 0), EINVAL, EINVAL)

#define _RETURN_EINVAL \
    _VALIDATE_RETURN((L"Invalid parameter", 0), EINVAL, EINVAL)

#define _RETURN_ERROR(_Msg, _Ret) \
    _VALIDATE_RETURN(((_Msg), 0), EINVAL, _Ret)

/* returns without calling _invalid_parameter */
#define _RETURN_NO_ERROR \
    return 0

/* note that _RETURN_TRUNCATE does not set errno */
#define _RETURN_TRUNCATE \
    return STRUNCATE

// ----------------------------------------------------------------------------

#ifdef errno
#undef errno
#endif

int errno;

int * __cdecl _errno(void)
{
	return & errno;
}

// ----------------------------------------------------------------------------

errno_t _set_errno(int value)
{
    errno = value;
    return 0;
}

// ----------------------------------------------------------------------------

errno_t _get_errno(int *pValue)
{
    /* validation section */
    _VALIDATE_RETURN_NOERRNO(pValue != NULL, EINVAL);

    *pValue = errno;
    return 0;
}

// ----------------------------------------------------------------------------

errno_t _get_osplatform(unsigned int *pValue)
{
    /* validation section */
    _VALIDATE_RETURN_ERRCODE(pValue != NULL, EINVAL);

    *pValue = _osplatform;
    return 0;
}

// ----------------------------------------------------------------------------

errno_t __cdecl memcpy_s(
    void * dst,
    size_t sizeInBytes,
    const void * src,
    size_t count
)
{
    if (count == 0)
    {
        /* nothing to do */
        return 0;
    }

    /* validation section */
    _VALIDATE_RETURN_ERRCODE(dst != NULL, EINVAL);
    if (src == NULL || sizeInBytes < count)
    {
        /* zeroes the destination buffer */
        ZeroMemory(dst, sizeInBytes);

        _VALIDATE_RETURN_ERRCODE(src != NULL, EINVAL);
        _VALIDATE_RETURN_ERRCODE(sizeInBytes >= count, EINVAL);
    }

    RtlCopyMemory(dst, src, count);
    return 0;
}

// ----------------------------------------------------------------------------

errno_t __cdecl memmove_s(
    void * dst,
    size_t sizeInBytes,
    const void * src,
    size_t count
)
{
    if (count == 0)
    {
        /* nothing to do */
        return 0;
    }

    /* validation section */
    _VALIDATE_RETURN_ERRCODE(dst != NULL, EINVAL);
    _VALIDATE_RETURN_ERRCODE(src != NULL, EINVAL);
    _VALIDATE_RETURN_ERRCODE(sizeInBytes >= count, EINVAL);

    RtlMoveMemory(dst, src, count);
    return 0;
}

// ----------------------------------------------------------------------------

// empty namespace for template versions of strcpy, strcat, strncpy
namespace
{

template <typename _CharType>
inline
errno_t _tcscpy_s(
    __out_ecount_z(_Size) _CharType *_Dest,
    __in size_t _Size,
    __in_z const _CharType *_Src
)
{
    _CharType *p;
    size_t available;
 
    /* validation section */
    _VALIDATE_STRING(_Dest, _Size);
    _VALIDATE_POINTER_RESET_STRING(_Src, _Dest, _Size);
 
    p = _Dest;
    available = _Size;
    while ((*p++ = *_Src++) != 0 && --available > 0)
    {
    }
 
    if (available == 0)
    {
        _RESET_STRING(_Dest, _Size);
        _RETURN_BUFFER_TOO_SMALL(_Dest, _Size);
    }
    _FILL_STRING(_Dest, _Size, _Size - available + 1);
    _RETURN_NO_ERROR;
}

template <typename _CharType>
inline
errno_t _tcscat_s(
    __out_ecount_z(_Size) _CharType *_Dest,
    __in size_t _Size,
    __in_z const _CharType *_Src
)
{
    _CharType *p;
    size_t available;
 
    /* validation section */
    _VALIDATE_STRING(_Dest, _Size);
    _VALIDATE_POINTER_RESET_STRING(_Src, _Dest, _Size);
 
    p = _Dest;
    available = _Size;
    while (available > 0 && *p != 0)
    {
        p++;
        available--;
    }
 
    if (available == 0)
    {
        _RESET_STRING(_Dest, _Size);
        _RETURN_DEST_NOT_NULL_TERMINATED(_Dest, _Size);
    }
 
    while ((*p++ = *_Src++) != 0 && --available > 0)
    {
    }
 
    if (available == 0)
    {
        _RESET_STRING(_Dest, _Size);
        _RETURN_BUFFER_TOO_SMALL(_Dest, _Size);
    }
    _FILL_STRING(_Dest, _Size, _Size - available + 1);
    _RETURN_NO_ERROR;
}

template <typename _CharType>
inline
errno_t _tcsncat_s(
    __out_ecount_z(_Size) _CharType *_Dest,
    __in size_t _Size,
    __in_z const _CharType *_Src,
    __in size_t _Count
)
{
    // _CharType *_Dest, size_t _Size, const _CharType *_Src, size_t _Count
    _CharType *p;
    size_t available;

    if (_Count == 0 && _Dest == NULL && _Size == 0)
    {
        /* this case is allowed; nothing to do */
        _RETURN_NO_ERROR;
    }

    /* validation section */
    _VALIDATE_STRING(_Dest, _Size);
    if (_Count != 0)
    {
        _VALIDATE_POINTER_RESET_STRING(_Src, _Dest, _Size);
    }
 
    p = _Dest;
    available = _Size;
    while (available > 0 && *p != 0)
    {
        p++;
        available--;
    }
 
    if (available == 0)
    {
        _RESET_STRING(_Dest, _Size);
        _RETURN_DEST_NOT_NULL_TERMINATED(_Dest, _Size);
    }
 
    if (_Count == _TRUNCATE)
    {
        while ((*p++ = *_Src++) != 0 && --available > 0)
        {
        }
    }
    else
    {
        _ASSERT_EXPR((!_CrtGetCheckCount() || _Count < available), L"Buffer is too small");

        while (_Count > 0 && (*p++ = *_Src++) != 0 && --available > 0)
        {
            _Count--;
        }
        if (_Count == 0)
        {
            *p = 0;
        }
    }
 
    if (available == 0)
    {
        if (_Count == _TRUNCATE)
        {
            _Dest[_Size - 1] = 0;
            _RETURN_TRUNCATE;
        }
        _RESET_STRING(_Dest, _Size);
        _RETURN_BUFFER_TOO_SMALL(_Dest, _Size);
    }
    _FILL_STRING(_Dest, _Size, _Size - available + 1);
    _RETURN_NO_ERROR;
}

template <typename _CharType>
inline
errno_t _tcsncpy_s(
    __out_ecount_z_opt(_Size) _CharType *_Dest,
    __in size_t _Size,
    __in_z_opt const _CharType *_Src,
    __in size_t _Count
)
{
    _CharType *p;
    size_t available;
 
    if (_Count == 0 && _Dest == NULL && _Size == 0)
    {
        /* this case is allowed; nothing to do */
        _RETURN_NO_ERROR;
    }
 
    /* validation section */
    _VALIDATE_STRING(_Dest, _Size);
    if (_Count == 0)
    {
        /* notice that the source string pointer can be NULL in this case */
        _RESET_STRING(_Dest, _Size);
        _RETURN_NO_ERROR;
    }
    _VALIDATE_POINTER_RESET_STRING(_Src, _Dest, _Size);
 
    p = _Dest;
    available = _Size;
    if (_Count == _TRUNCATE)
    {
        while ((*p++ = *_Src++) != 0 && --available > 0)
        {
        }
    }
    else
    {
        while ((*p++ = *_Src++) != 0 && --available > 0 && --_Count > 0)
        {
        }
        if (_Count == 0)
        {
            *p = 0;
        }
    }
 
    if (available == 0)
    {
        if (_Count == _TRUNCATE)
        {
            _Dest[_Size - 1] = 0;
            _RETURN_TRUNCATE;
        }
        _RESET_STRING(_Dest, _Size);
        _RETURN_BUFFER_TOO_SMALL(_Dest, _Size);
    }
    _FILL_STRING(_Dest, _Size, _Size - available + 1);
    _RETURN_NO_ERROR;
}

} // empty namespace

// ----------------------------------------------------------------------------

// sal annotations are already in atlmincrt.h

errno_t __cdecl strcpy_s(
    char *dst,
    size_t sizeInBytes,
    const char *src
)
{
    return _tcscpy_s<char>(dst, sizeInBytes, src);
}

errno_t __cdecl wcscpy_s(
    wchar_t *dst,
    size_t sizeInWords,
    const wchar_t *src
)
{
    return _tcscpy_s<wchar_t>(dst, sizeInWords, src);
}

// ----------------------------------------------------------------------------

errno_t __cdecl strcat_s(
    char *dst,
    size_t sizeInBytes,
    const char *src
)
{
    return _tcscat_s<char>(dst, sizeInBytes, src);
}

errno_t __cdecl wcscat_s(
    wchar_t *dst,
    size_t sizeInWords,
    const wchar_t *src
)
{
    return _tcscat_s<wchar_t>(dst, sizeInWords, src);
}

// ----------------------------------------------------------------------------

errno_t __cdecl strncat_s(
    char *dst, 
    size_t sizeInWords, 
    const char *src, 
    size_t count
)
{
    return _tcsncat_s<char>(dst, sizeInWords, src, count);
}

errno_t __cdecl wcsncat_s(
    wchar_t *dst, 
    size_t sizeInWords, 
    const wchar_t *src, 
    size_t count)
{
    return _tcsncat_s<wchar_t>(dst, sizeInWords, src, count);
}

// ----------------------------------------------------------------------------

errno_t __cdecl strncpy_s(
    char *dst,
    size_t sizeInBytes,
    const char *src,
    size_t count
)
{
    return _tcsncpy_s<char>(dst, sizeInBytes, src, count);
}

errno_t __cdecl wcsncpy_s(
    wchar_t *dst,
    size_t sizeInWords,
    const wchar_t *src,
    size_t count
)
{
    return _tcsncpy_s<wchar_t>(dst, sizeInWords, src, count);
}


// ----------------------------------------------------------------------------

const char * __cdecl strrchr (
    const char * string,
    int ch
)
{
    char *start = (char *)string;

    while (*string++)                   /* find end of string */
        ;
                                        /* search towards front */
    while (--string != start && *string != (char)ch)
        ;

    if (*string == (char)ch)            /* char found ? */
        return( (char *)string );

    return(NULL);
}

const wchar_t * __cdecl wcsrchr (
    const wchar_t * string,
    wchar_t ch
)
{
    wchar_t *start = (wchar_t *)string;

    while (*string++)                   /* find end of string */
        ;
                                        /* search towards front */
    while (--string != start && *string != (wchar_t)ch)
        ;

    if (*string == (wchar_t)ch)         /* wchar_t found ? */
        return( (wchar_t *)string );

    return(NULL);
}

// ----------------------------------------------------------------------------

size_t __cdecl strnlen(
    const char *str,
    size_t maxsize
)
{
    size_t n;

    /* Note that we do not check if s == NULL, because we do not
     * return errno_t...
     */

    for (n = 0; n < maxsize && *str; n++, str++)
        ;

    return n;
}

size_t __cdecl wcsnlen(
    const wchar_t *wcs,
    size_t maxsize
)
{
    size_t n;

    /* Note that we do not check if s == NULL, because we do not
     * return errno_t...
     */

    for (n = 0; n < maxsize && *wcs; n++, wcs++)
        ;

    return n;
}

////////////////////////////////////////////////////////////////////////////////
// Encode / Decode Pointer

extern "C" {

void * __cdecl _encode_pointer(void *ptr)
{
    PVOID (APIENTRY *pfnEncodePointer)(PVOID) = NULL;

    HINSTANCE hKernel32 = GetModuleHandle("Kernel32.dll");

    if (hKernel32)
    {
        pfnEncodePointer = (PVOID (APIENTRY *)(PVOID))
            GetProcAddress(hKernel32, "EncodePointer");
    }

    if (pfnEncodePointer != NULL)
    {
        ptr = (*pfnEncodePointer)(ptr);
    }

    return ptr;
}

void * __cdecl _encoded_null()
{
    return _encode_pointer(NULL);
}

void * __cdecl _decode_pointer(void *codedptr)
{
    PVOID (APIENTRY *pfnDecodePointer)(PVOID) = NULL;

    HINSTANCE hKernel32 = GetModuleHandle("Kernel32.dll");

    if (hKernel32)
    {
        pfnDecodePointer = (PVOID (APIENTRY *)(PVOID))
            GetProcAddress(hKernel32, "DecodePointer");
    }

    if (pfnDecodePointer != NULL)
    {
        codedptr = (*pfnDecodePointer)(codedptr);
    }

    return codedptr;
}

} // end extern "C"
