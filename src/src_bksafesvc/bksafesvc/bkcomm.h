// bkcomm.h : Declaration of the Cbkcomm

#pragma once
#include "resource.h"       // main symbols

#include "_idl_gen/bksafesvc.h"
#include "com_svc_dispatch.h"

#include "bksafesvcmodule.h"
#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// Cbkcomm
class ATL_NO_VTABLE Cbkcomm :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<Cbkcomm, &CLSID_bkcomm>,
// 	public IObjectWithSiteImpl<Cbkcomm>,
 	public IDispatchImpl<Ibkcomm, &IID_Ibkcomm, &LIBID_bksafesvcLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
    Cbkcomm() :
      m_dwProcId( 0 ),
      m_piRoot( NULL )
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BKCOMM)


BEGIN_COM_MAP(Cbkcomm)
	COM_INTERFACE_ENTRY(Ibkcomm)
 	COM_INTERFACE_ENTRY(IDispatch)
// 	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
        BKSVC_DEBUG_TRACE(L"FinalConstruct remains %d\r\n", _AtlModule->OnObjectCreate());
		return OnFinalConstruct();
	}

	void FinalRelease()
	{
        BKSVC_DEBUG_TRACE(L"FinalRelease remains %d\r\n", _AtlModule->OnObjectDestroy());

        return OnFinalRelease();
	}

public:

    STDMETHOD(Initialize)(int nModule, ULONG ulProcId, int nVersion);
    STDMETHOD(Call)(int nMethod, BSTR bstrParams, BSTR* pbstrResponse);
    STDMETHOD(Uninitialize)(void);
private:
    HRESULT OnFinalConstruct();
    void    OnFinalRelease();
protected:
    DWORD       m_dwProcId;
    int         m_nModuleId;
    CBkComRoot* m_piRoot;
};

OBJECT_ENTRY_AUTO(__uuidof(bkcomm), Cbkcomm)
