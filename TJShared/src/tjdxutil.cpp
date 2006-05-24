#include "../include/tjshared.h"
using namespace tj::shared;

HRESULT RotRegistration::AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)  {
	IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

	if (!pUnkGraph || !pdwRegister) {
        return E_POINTER;
	}

	if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
	}

    wsprintfW(wsz, L"TJShow FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, pMoniker, pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}


// Removes a filter graph from the Running Object Table
void RotRegistration::RemoveGraphFromRot(DWORD pdwRegister) {
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}

// ROT Regisration holder
RotRegistration::RotRegistration(IGraphBuilder* ig) {
	assert(ig!=0);
	HRESULT hr = AddGraphToRot(ig, &_magic);
    if(FAILED(hr)) {
		MessageBox(0L, L"ROT registration failed",L"", MB_OK);
		_magic = 0;
    }
}

RotRegistration::~RotRegistration() {
	 RemoveGraphFromRot(_magic);
}

CComPtr<IPin> DXTools::GetPin(CComPtr<IBaseFilter> filter, PIN_DIRECTION dir) {
    bool found = false;
    CComPtr<IEnumPins> pinEnum;
    CComPtr<IPin> pin;

    // Begin by enumerating all the pins on a filter
	if(FAILED(filter->EnumPins(&pinEnum))) {
		return NULL;
	}

    // Now look for a pin that matches the direction characteristic.
    // When we've found it, we'll return with it.
    while(pinEnum->Next(1, &pin, 0)==S_OK) {
        PIN_DIRECTION cdir;
        pin->QueryDirection(&cdir);

		if(dir==cdir) {
			found = true;
			break;
		}
    }

	return found?pin:NULL;
}

HRESULT DXTools::SaveGraphFile(CComPtr<IGraphBuilder> pGraph, std::wstring wszPath) {
    const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
    HRESULT hr;
    IStorage *pStorage = NULL;

    // First, create a document file that will hold the GRF file
    hr = StgCreateDocfile(wszPath.c_str(),STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,0, &pStorage);
    if(FAILED(hr)) {
        return hr;
    }

    // Next, create a stream to store.
    IStream *pStream;
    hr = pStorage->CreateStream(wszStreamName, STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, 0, &pStream);
    if (FAILED(hr)) {
        pStorage->Release();    
        return hr;
    }

    // The IpersistStream::Save method converts a stream
    // into a persistent object.
    IPersistStream *pPersist = NULL;
    pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
    if (SUCCEEDED(hr))  {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();
    return hr;
}

HRESULT DXTools::LoadGraphFile(CComPtr<IGraphBuilder> pGraph, std::wstring name) {
    IStorage *pStorage = 0;

    if (S_OK != StgIsStorageFile(name.c_str())) {
        return E_FAIL;
    }

    HRESULT hr = StgOpenStorage(name.c_str(), 0,STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE, 0, 0, &pStorage);

    if (FAILED(hr)) {
        return hr;
    }

    IPersistStream *pPersistStream = 0;
    hr = pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersistStream));

    if (SUCCEEDED(hr)) {
        IStream *pStream = 0;
        pStorage->OpenStream(L"ActiveMovieGraph", 0,STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);

        if(SUCCEEDED(hr)) {
            hr = pPersistStream->Load(pStream);
            pStream->Release();
        }
        pPersistStream->Release();
    }
    pStorage->Release();
    return hr;
}

int DXTools::GetDuration(std::wstring file) {
	if(GetFileAttributes(file.c_str())==INVALID_FILE_ATTRIBUTES) {
		return -1;
	}

	// Graph files have infinite length
	if(file.length()>4 && file.substr(file.length()-4, 4)==L".grf") {
		return INT_MAX;
	}

	CComPtr<IGraphBuilder> graph;
	if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&graph))) {
		return -2;
	}

	if(FAILED(graph->RenderFile(file.c_str(), NULL))) {
		return -3;
	}

	CComPtr<IMediaSeeking> ms;
	if(FAILED(graph->QueryInterface(IID_IMediaSeeking, (void**)&ms))) {
		return -4;
	}

	LONGLONG dur = 0;
	ms->GetDuration(&dur);

	return int(dur/10000);
}