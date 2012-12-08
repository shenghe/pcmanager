///////////////////////////////////////////////////////////////
//
//	Filename: 	KClassFactory.h
//	Creator:	lichenglin  <lichenglin@kingsoft.net>
//	Date:		2007-8-25   21:45
//	Comment:	Use: KComObject<KClassFactory<Cxxx> >
//
///////////////////////////////////////////////////////////////

#ifndef _KClassFactory_h_
#define _KClassFactory_h_
#include "KComObject.h"

//.-----------------------------------------------------------------------------
// KClassFactory Decleration
//.-----------------------------------------------------------------------------

template <class T>
class KClassFactory : public IClassFactory
{
public:
	virtual HRESULT STDMETHODCALLTYPE CreateInstance( 
		/* [unique][in] */  IUnknown* pUnkOuter,
		/* [in] */          REFIID    riid,
		/* [iid_is][out] */ void**    ppvObject
		);

	virtual HRESULT STDMETHODCALLTYPE LockServer( 
		/* [in] */ BOOL fLock
		);

	KAS_BEGIN_COM_MAP( KClassFactory )
		KAS_COM_INTERFACE_ENTRY( IClassFactory )
	KAS_END_COM_MAP()

private:
};


//.-----------------------------------------------------------------------------
// Implemetation
//.-----------------------------------------------------------------------------

template <class T>
HRESULT KClassFactory<T>::CreateInstance( 
	/* [unique][in] */  IUnknown* pUnkOuter,
	/* [in] */          REFIID    riid,
	/* [iid_is][out] */ void**    ppvObject
	)
{
	if( pUnkOuter != NULL )
		return CLASS_E_NOAGGREGATION;

	T* pObj = new T;
	if( pObj == NULL )
		return E_OUTOFMEMORY;

	pObj->AddRef();
	HRESULT hr = pObj->QueryInterface( riid, ppvObject );
	pObj->Release();

	return hr;
}


template <class T>
HRESULT KClassFactory<T>::LockServer( /* [in] */ BOOL fLock )
{
	fLock ? KASGetDllModule()->Lock() : KASGetDllModule()->UnLock();
	return S_OK;
}



#endif // _KClassFactory_h_