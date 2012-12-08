#pragma once
#include <Unknwn.h>
#include <libheader/libupdhelper.h>
#include "SoftChecker.h"
#include "SoftManager.h"

class  CSoftMgrUpdateHelper : public IUpdateHelper
{
public:
	CSoftMgrUpdateHelper(void);
	virtual ~CSoftMgrUpdateHelper(void);

	virtual HRESULT STDMETHODCALLTYPE Combine( LPCWSTR lpwszDifflib ) ;
	virtual HRESULT STDMETHODCALLTYPE Notify( LPCWSTR lpwszFileName ) ;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		ATLASSERT(ppvObject);
		if ( IsEqualIID(riid, __uuidof(IUpdateHelper)) )
		{
			this->AddRef();
			*ppvObject = this;

			return S_OK;
		}
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return static_cast<ULONG>(::InterlockedIncrement(&__ref));
	}

	virtual ULONG STDMETHODCALLTYPE Release( void) 
	{
		if(::InterlockedDecrement(&__ref) == 0)
		{
			delete this;
			return 0;
		}
		else
		{
			return static_cast<ULONG>(__ref);
		}
	}

private:
	LONG volatile __ref;
};
