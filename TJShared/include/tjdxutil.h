#ifndef _TJDXUTIL_H
#define _TJDXUTIL_H

#include <atlbase.h>
#include <dshow.h>

HRESULT EXPORTED AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
void EXPORTED RemoveGraphFromRot(DWORD pdwRegister);

class EXPORTED RotRegistration {
	public:
		RotRegistration(IGraphBuilder* ig);
		virtual ~RotRegistration();

	protected:
		DWORD _magic;
};

#endif