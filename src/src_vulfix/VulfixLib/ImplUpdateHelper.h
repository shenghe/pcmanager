#pragma once
#include <libheader/libupdhelper.h>
#include "LeakDBUpdate.h"


class CImplUpdateHelper : public IUpdateHelper, public IUpdateHelper2
{
public:
	CImplUpdateHelper(void);
	~CImplUpdateHelper(void);
		
	// IUnkown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void**ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef( void);
	virtual ULONG STDMETHODCALLTYPE Release( void);
	
	// IUpdateHelper
	virtual HRESULT STDMETHODCALLTYPE Combine( LPCWSTR lpwszDifflib );
	virtual HRESULT STDMETHODCALLTYPE Notify( LPCWSTR lpwszFileName );

	// IUpdateHelper2
	virtual HRESULT STDMETHODCALLTYPE Combine2( LPCWSTR szBaselib, LPCWSTR lpwszDifflib );

protected:
	CLeakDBUpdate m_update;
	
private:
	LONG m_nRefCount;
};
