#pragma once
#include <atlbase.h>
#include <Unknwn.h>
#include "kclear/libupdhelper.h"

class  __declspec(uuid("81ab1cb2-e8f2-42e3-93a8-000632ded876")) CCleanMgrUpdateHelper:public IUpdateHelper
{
public:
	CCleanMgrUpdateHelper(void);
	virtual ~CCleanMgrUpdateHelper(void);

	virtual HRESULT STDMETHODCALLTYPE Combine( LPCWSTR lpwszDifflib ) ;
	virtual HRESULT STDMETHODCALLTYPE Notify( LPCWSTR lpwszFileName ) ;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */  void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		ATLASSERT(ppvObject);
		if ( IsEqualIID(riid, __uuidof(IUpdateHelper)) )
		{
			IUpdateHelper *p = this;
			*ppvObject = p;
			return S_OK;
		}
		else if( IsEqualIID(riid, __uuidof(CCleanMgrUpdateHelper)) )
		{

			CCleanMgrUpdateHelper *p = this;
			*ppvObject = p;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return ++__ref;
	}

	virtual ULONG STDMETHODCALLTYPE Release( void) 
	{
		return __ref;
	}

	ULONG __ref;
	
};
