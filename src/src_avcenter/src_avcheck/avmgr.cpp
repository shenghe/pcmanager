// avproductmgr.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "com_s\com\comobject.h"
#include "avmgr\avmgrinterface.h"
#include "avproductmgr.h"


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

extern "C"	HRESULT WINAPI CreateAVProductMgr( REFIID riid, void** ppvObj )
{
	if (__uuidof(IAVProductMgr) == riid || IID_IUnknown == riid)
	{
		IAVProductMgr *pIAVProductMgr = new KComObject<CAVProductMgr>;
		if (NULL == pIAVProductMgr)
			return E_OUTOFMEMORY;
	
		*ppvObj = pIAVProductMgr;

		return S_OK;
	}

	return S_FALSE;
}

extern "C"	VOID WINAPI FreeAVProductMgr(IAVProductMgr* pIAVProductMgr)
{
	CAVProductMgr* pAVProductMgr = (CAVProductMgr*)pIAVProductMgr;

	if (pAVProductMgr!=NULL)
		delete pAVProductMgr;
}


#ifdef _MANAGED
#pragma managed(pop)
#endif

