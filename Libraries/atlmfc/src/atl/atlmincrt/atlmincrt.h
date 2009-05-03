// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#pragma once

#pragma warning(disable : 4668)	// is not defined as a preprocessor macro, replacing with '0' for '#if/#elif
#pragma warning(disable : 4820)	// padding added after member

#define __ATLMINCRT_H__

#ifdef _WIN64
#define DEFAULT_SECURITY_COOKIE 0x2B992DDFA23249D6
#else  /* _WIN64 */
#define DEFAULT_SECURITY_COOKIE 0xBB40E64E
#endif  /* _WIN64 */

#include "sal.h"

typedef void (*_PVFV)(void);

#pragma warning(push)
#pragma warning(disable:26020)
void * __cdecl calloc(size_t n, size_t s);
errno_t __cdecl _set_errno(int);
errno_t __cdecl _get_errno(int *);
errno_t __cdecl _get_osplatform(unsigned int *);
errno_t __cdecl memcpy_s(__out_bcount_part_opt(_DstSize, _MaxCount) void * _Dst, __in size_t _DstSize, __in_bcount_opt(_MaxCount) const void * _Src, __in size_t _MaxCount);
errno_t __cdecl memmove_s(__out_bcount_part_opt(_DstSize,_MaxCount) void * _Dst, __in size_t _DstSize, __in_bcount_opt(_MaxCount) const void * _Src, __in size_t _MaxCount);

errno_t __cdecl strcat_s(__out_ecount_z(sizeInBytes) char *dst, __in size_t sizeInBytes, __in_z const char *src);
errno_t __cdecl strncat_s(__out_ecount_z(sizeInBytes) char *dst, __in size_t sizeInBytes,  __in_z const char *src, __in size_t count);

errno_t __cdecl strcpy_s(__out_ecount_z(sizeInBytes) char *dst, __in size_t sizeInBytes, __in_z const char *src);
errno_t __cdecl strncpy_s(__out_ecount_z_opt(sizeInBytes) char *dst, __in size_t sizeInBytes, __in_z_opt const char *src, __in size_t count);

errno_t __cdecl wcscat_s(__out_ecount_z(sizeInWords) wchar_t *dst, __in size_t sizeInWords, __in_z const wchar_t *src);
errno_t __cdecl wcsncat_s(__out_ecount_z(sizeInBytes) wchar_t *dst, __in size_t sizeInBytes,  __in_z const wchar_t *src, __in size_t count);

errno_t __cdecl wcscpy_s(__out_ecount_z(sizeInWords) wchar_t *dst, __in size_t sizeInWords, __in_z const wchar_t *src);
errno_t __cdecl wcsncpy_s(__out_ecount_z_opt(sizeInWords) wchar_t *dst, __in size_t sizeInWords, __in_z_opt const wchar_t *, __in size_t src);

__checkReturn _CONST_RETURN wchar_t * __cdecl wcsrchr (__in_z  const wchar_t * _Str, __in wchar_t ch);
__checkReturn _CONST_RETURN char *  __cdecl strrchr(__in_z const char * _Str, __in int _Ch);

size_t __cdecl wcsnlen(const wchar_t *wcs, size_t maxsize);
size_t __cdecl strnlen(const char *str, size_t maxsize);

#pragma warning(pop)
