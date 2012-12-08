#include "stdafx.h"
#include "Download.h"
#include "KFileDownload.h"

HRESULT WINAPI CreateDownloadObject( REFIID riid, void** ppvObj )
{
	ATLASSERT(ppvObj);
	if ( IsEqualIID(riid, __uuidof(IDownload)) )
	{
		IDownload *p = new KFileDownload;
		*ppvObj = p;
		return S_OK;
	}
	return E_NOINTERFACE;
}
