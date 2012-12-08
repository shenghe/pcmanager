///////////////////////////////////////////////////////////////
//
//	Filename: 	KComObject.h
//	Creator:	lichenglin  <lichenglin@kingsoft.net>
//	Date:		2007-8-25   12:59
//	Comment:	
//
///////////////////////////////////////////////////////////////

#ifndef _KComObject_h_
#define _KComObject_h_
#include "KDllModule.h"

//.-----------------------------------------------------------------------------
// KComObject Decleration, 
//.-----------------------------------------------------------------------------

template <class T>
class KComObject : public T
{
public:
	KComObject();
	~KComObject();

	HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject );
	ULONG   STDMETHODCALLTYPE AddRef();
	ULONG   STDMETHODCALLTYPE Release();

private:
	ULONG m_lRefCount;
};





//.-----------------------------------------------------------------------------
// Implemetation
//.-----------------------------------------------------------------------------

template <class T>
KComObject<T>::KComObject()
{
	::KASGetDllModule()->Lock();
	m_lRefCount = 0;
}



template <class T>
KComObject<T>::~KComObject()
{
	::KASGetDllModule()->UnLock();
}


template <class T>
HRESULT KComObject<T>::QueryInterface( REFIID riid, void **ppvObject )
{
	HRESULT hr = _InternalQueryInterface( riid, ppvObject );

	if( SUCCEEDED( hr ) )
	{
		AddRef();
	}

	return hr;
}


template <class T>
ULONG  KComObject<T>::AddRef()
{
	return ++m_lRefCount;
}


template <class T>
ULONG  KComObject<T>::Release()
{
	--m_lRefCount;

	if( m_lRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_lRefCount;
}




//.-----------------------------------------------------------------------------
// Interface Entry Table
//.-----------------------------------------------------------------------------

/*
[接口映射宏:]
KAS_BEGIN_COM_MAP( CComImpl )
	KAS_COM_INTERFACE_ENTRY( IInterface0 )
	KAS_COM_INTERFACE_ENTRY( IInterface1 )
	KAS_COM_INTERFACE_ENTRY( IInterface2 )
KAS_END_COM_MAP()


[宏展开]
HRESULT _InternalQueryInterface( REFIID riid, void **ppvObject )
{
	HRESULT hr = E_FAIL;
	
	if( false )
	{
	}

	else if( riid == IID_IInterface0 || riid == IID_IUnknown )
	{
		*ppvObject = static_cast<IInterface0*>(this);
		hr = S_OK;
	}

	else if( riid == IID_IInterface1 || riid == IID_IUnknown )
	{
		*ppvObject = static_cast<IInterface1*>(this);
		hr = S_OK;
	}

	...

	else
	{
		*ppvObject = NULL;
		hr = E_NOINTERFACE;
	}

	return hr;
}

*/


#define KAS_BEGIN_COM_MAP( CComImpl ) HRESULT _InternalQueryInterface( REFIID riid, void **ppvObject ) { \
	HRESULT hr = E_FAIL; \
	if( false) {}

#define KAS_COM_INTERFACE_ENTRY( I ) 	 else if( riid == __uuidof( I ) || riid == IID_IUnknown ) { \
	*ppvObject = static_cast<I*>(this); \
	hr = S_OK; } 

#define KAS_COM_INTERFACE_ENTRY_IID( I )  else if( riid == IID_##I || riid == IID_IUnknown ) { \
	*ppvObject = static_cast<I*>(this); \
	hr = S_OK; } 

#define KAS_END_COM_MAP()  else { \
	*ppvObject = NULL; \
	hr = E_NOINTERFACE; } \
	return hr; }


#endif // _KComObject_h_