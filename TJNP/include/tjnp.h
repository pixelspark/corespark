#ifndef _TJNP_H
#define _TJNP_H

/** TJNP is an implementation of TNP, the TJ Network Protocol. **/
#include "../../TJShared/include/tjshared.h"

#ifdef TJNP_EXPORTS
	#ifdef TJ_OS_WIN
		#define NP_EXPORTED __declspec(dllexport)
	#else
		#define NP_EXPORTED __attribute__ ((visibility("default")))
	#endif
#else
	#ifdef TJ_OS_WIN
		#define NP_EXPORTED __declspec(dllimport)
	#else
		#define NP_EXPORTED
	#endif
#endif

#pragma warning(push)
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)

#include "tjprotocol.h"
#include "tjstream.h"
#include "tjclient.h"
#include "tjtransaction.h"
#include "tjsocket.h"

#pragma warning(pop)
#endif