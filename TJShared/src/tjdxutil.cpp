#include "../include/tjcore.h"
#include <dshow.h>
using namespace tj::shared;

int DXTools::GetDuration(const std::wstring& file) {
	if(GetFileAttributes(file.c_str())==INVALID_FILE_ATTRIBUTES) {
		return -1;
	}

	// Graph files have infinite length
	if(file.length()>4 && file.substr(file.length()-4, 4)==L".grf") {
		return INT_MAX;
	}

	IGraphBuilder* graph = 0;
	if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&graph))) {
		return -2;
	}

	if(FAILED(graph->RenderFile(file.c_str(), NULL))) {
		graph->Release();
		return -3;
	}

	IMediaSeeking* ms = 0;
	if(FAILED(graph->QueryInterface(IID_IMediaSeeking, (void**)&ms))) {
		graph->Release();
		return -4;
	}

	LONGLONG dur = 0;
	ms->GetDuration(&dur);
	graph->Release();
	ms->Release(); // needed?

	return int(dur/10000);
}