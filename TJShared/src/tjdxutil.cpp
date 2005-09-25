#include "../include/tjshared.h"

#include <atlbase.h>
#include <windows.h>
#include <dshow.h>
#include <stdio.h>

HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)  {
	return S_OK;

	IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (!pUnkGraph || !pdwRegister)
        return E_POINTER;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    wsprintfW(wsz, L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}


// Removes a filter graph from the Running Object Table
void RemoveGraphFromRot(DWORD pdwRegister) {
	return;

    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}

// ROT Regisration holder
RotRegistration::RotRegistration(IGraphBuilder* ig) {
	HRESULT hr = AddGraphToRot(ig, &_magic);
    if(FAILED(hr)) {
		_magic = 0;
    }
}

RotRegistration::~RotRegistration() {
	 RemoveGraphFromRot(_magic);
}