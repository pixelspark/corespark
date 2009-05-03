// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

// These declspecs should be turned off when building this static lib
#define _CRTNOALIAS
#define _CRTRESTRICT

// ignore the declaration of _osplatform, because we are going to provide one internally
#define _osplatform __ignore_osplatform
#include <stdlib.h>
#undef _osplatform

#include <atlbase.h>
