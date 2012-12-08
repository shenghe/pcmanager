#include "StdAfx.h"
#include "ImplUpdateHelper.h"
#include <libheader/libheader.h>

CImplUpdateHelper::CImplUpdateHelper(void)
{
	m_nRefCount = 0;
	AddRef();
}

CImplUpdateHelper::~CImplUpdateHelper(void)
{

}

// 
HRESULT STDMETHODCALLTYPE CImplUpdateHelper::QueryInterface(REFIID riid, void**ppvObject)
{
	if ( IsEqualIID(riid, __uuidof(IUnknown)) )
	{
		*ppvObject = static_cast<IUnknown*>(this);
		return S_OK;
	}
	else if ( IsEqualIID(riid, __uuidof(IUpdateHelper)) )
	{
		*ppvObject = static_cast<IUpdateHelper*>(this);
		return S_OK;
	}
	else if ( IsEqualIID(riid, __uuidof(IUpdateHelper2)) )
	{
		*ppvObject = static_cast<IUpdateHelper2*>(this);
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CImplUpdateHelper::AddRef( void)
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG STDMETHODCALLTYPE CImplUpdateHelper::Release( void)
{
	LONG nCount = InterlockedDecrement(&m_nRefCount);
	if ( nCount == 0 )
	{
		delete this;
	}
	return nCount;
}

// 
HRESULT STDMETHODCALLTYPE CImplUpdateHelper::Combine( LPCWSTR lpwszDifflib )
{
	return m_update.Combine( lpwszDifflib );
}

HRESULT STDMETHODCALLTYPE CImplUpdateHelper::Notify( LPCWSTR lpwszFileName )
{
	return E_NOTIMPL;	
}

HRESULT STDMETHODCALLTYPE CImplUpdateHelper::Combine2( LPCWSTR szBaselib, LPCWSTR lpwszDifflib )
{
	return m_update.CombineFile( szBaselib, lpwszDifflib );
}