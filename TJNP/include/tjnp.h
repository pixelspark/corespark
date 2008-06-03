#ifndef _TJNP_H
#define _TJNP_H

#ifdef TJNP_EXPORTS
	#define NP_EXPORTED __declspec(dllexport)
#else
	#define NP_EXPORTED __declspec(dllimport)
#endif

#pragma warning(push)
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)

/** TJNP is an implementation of TNP, the TJ Network Protocol. **/
#include "../../TJShared/include/tjshared.h"

#include "tjprotocol.h"
#include "tjstream.h"
#include "tjclient.h"
#include "tjtransaction.h"
#include "tjsocket.h"

#pragma warning(pop)
#endif