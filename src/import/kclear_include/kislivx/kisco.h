//kisco.h
//
#pragma once
#ifndef __KISUP_KISCO_H__
#define __KISUP_KISCO_H__
#include "uplive_common.h"
#include <unknwn.h>

NS_BEGIN(KISUP)
NS_BEGIN(Kis)

template<typename InterfaceType>
class IKisCoObjectT : public IUnknown
{
public:
	IKisCoObjectT() : ulRefCnt(0) {}
	virtual ~IKisCoObjectT(){}

public:
	ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return ++ulRefCnt;
	}
	ULONG STDMETHODCALLTYPE Release( void)
	{
		ulRefCnt--;
		if (0 == ulRefCnt)
		{
			delete this;
			return 0;
		}
		return ulRefCnt;
	}
	HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject)
	{
		HRESULT hResult = E_NOINTERFACE;

		if (NULL != ppvObject)
		{
			*ppvObject = NULL;
			if (__uuidof(IUnknown) == riid || __uuidof(InterfaceType) == riid)
			{
				this->AddRef();
				*ppvObject = static_cast<void*>(this);
				hResult = S_OK;
			}
			else
			{
				hResult = E_NOINTERFACE;
			}
		}
		else
			hResult = E_INVALIDARG;

		return hResult;
	}
private:
	ULONG ulRefCnt;
};

////////////////////////////////////////////////////////////////////////////
//
namespace KisCo
{
//public:
	HRESULT BindKisCoObject(REFIID riid);

	template<typename InterfaceType, typename ImplType>
	static HRESULT NewKisCoObject(REFIID riid, LPVOID * ppv)
	{
		HRESULT hResult = E_NOINTERFACE;
		ImplType * pObjectImp = NULL;
		InterfaceType * pIType = NULL;

		if (__uuidof(InterfaceType) == riid)
		{
			pObjectImp = new ImplType;
			if (NULL != pObjectImp)
			{
				pIType = pObjectImp;
				pIType->AddRef();
				*ppv = dynamic_cast<IUnknown*>(pObjectImp);
				hResult = S_OK;
			}
			else
				hResult = E_OUTOFMEMORY;
		}

		return hResult;
	}
};

NS_END(Kis)
NS_END(KISUP)




#endif	//__KISUP_KISCO_H__