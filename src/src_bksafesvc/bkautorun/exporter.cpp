#include "stdafx.h"
#include "bkautorunimp.h"

HRESULT WINAPI BkCreateAutorunHandle( IAutorunHandle **ppiAutorunHandle )
{
	if (NULL == ppiAutorunHandle)
		return E_INVALIDARG;

	*ppiAutorunHandle = new KComObject<CAutorunHandle>;
	if (NULL == *ppiAutorunHandle)
		return E_OUTOFMEMORY;

	(*ppiAutorunHandle)->AddRef();

	return S_OK;
}