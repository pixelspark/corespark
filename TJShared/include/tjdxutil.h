#ifndef _TJDXUTIL_H
#define _TJDXUTIL_H

#include <atlbase.h>
#include <dshow.h>

class EXPORTED RotRegistration {
	public:
		RotRegistration(IGraphBuilder* ig);
		virtual ~RotRegistration();
		
	protected:
		DWORD _magic;
		static HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
		static void RemoveGraphFromRot(DWORD pdwRegister);
};

class EXPORTED DXTools {
	public:
		static CComPtr<IPin> GetPin(CComPtr<IBaseFilter> filter, PIN_DIRECTION dir);
		static HRESULT SaveGraphFile(CComPtr<IGraphBuilder> graph, std::wstring path);
		static HRESULT LoadGraphFile(CComPtr<IGraphBuilder> graph, std::wstring name);
		static int GetDuration(std::wstring file);
};

#endif