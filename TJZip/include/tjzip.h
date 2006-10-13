#ifndef _TJZIP_H
#define _TJZIP_H

#ifdef TJZIP_EXPORTS
#define TJZIP_EXPORTED __declspec(dllexport)
#else
#define TJZIP_EXPORTED __declspec(dllimport)
#endif

#pragma warning(push)
#pragma warning(disable: 4251 4275)

#include "../../TJShared/include/tjshared.h"

#ifndef HZIP_DECLARED
#define HZIP_DECLARED
DECLARE_HANDLE(HZIP);
#endif

#include "tjpackage.h"

#pragma warning(pop)

#endif