# This is a part of the Active Template Library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Active Template Library Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Active Template Library product.

TARGETNAME=atlmincrt
TARGETTYPE=LIB

!include ..\atlcommon.mak

OBJS=$(D)\atlinit.obj $(D)\atlinitdll.obj $(D)\atlinitexe.obj $(D)\atlinitexeu.obj

OBJS= \
	$(OBJS) \
!if "$(PLATFORM)" == "INTEL"	
	$(D)\atlloadcfg.obj \
!endif

!include ..\atltarg.mak
