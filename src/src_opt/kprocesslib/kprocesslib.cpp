// kprocesslib.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "kprocesslib/interface.h"
#include "processlibqueryimp.h"
#include "libheader/libupdhelper.h"
#include "com_s/com/comobject.h"
#include "kprocesslibupdate.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

extern "C"
{

	IProcessLibQuery* WINAPI pxGetProcessLibQuery(DWORD nVer)
	{
		if (nVer!=VERSION_KSAFE_PROCESSLIB_INTERFACE)
			return NULL;

		CProcessLibQueryImp* eng = new CProcessLibQueryImp;
		return (IProcessLibQuery*)eng;
	}

	VOID WINAPI pxFreeProcessLibQuery(IProcessLibQuery* pEng)
	{
		CProcessLibQueryImp* query = (CProcessLibQueryImp*)pEng;

		if (query!=NULL)
		{
			delete query;
		}
	}


	HRESULT WINAPI CreateUpdateHelper( REFIID riid, void** ppvObj )
	{
		if (__uuidof(IUpdateHelper) == riid || IID_IUnknown == riid)
		{
			IUpdateHelper *piUpdateer = new KComObject<KProcessLibUpdate>;
			if (NULL == piUpdateer)
				return E_OUTOFMEMORY;

			piUpdateer->AddRef();

			*ppvObj = piUpdateer;

			return S_OK;
		}

		return S_FALSE;
	}

}

