#ifndef NET_CHECK_PUBLIC_INC_
#define NET_CHECK_PUBLIC_INC_

#include <unknwn.h>

//////////////////////////////////////////////////////////////////////////

interface INetCheckEvent
{
	virtual STDMETHODIMP OnNetCheckMsg( ULONG uOrder, const PTCHAR szSender, const PTCHAR szMsg ) = 0;
};


extern const __declspec(selectany) GUID CLSID_INetCheckImp =
{ 0xed33be6d, 0x73a5, 0x452c, { 0x9b, 0xed, 0xea, 0xc4, 0x2, 0xb6, 0x88, 0xa7 } };

[
    uuid(ED33BE6D-73A5-452c-9BED-EAC402B688A7)
]
interface INetCheck : IUnknown	
{
	virtual STDMETHODIMP_(BOOL) Check( INetCheckEvent* checkEvent ) = 0;
	virtual STDMETHODIMP_(BOOL) Fix( INetCheckEvent* checkEvent ) = 0;
};

//////////////////////////////////////////////////////////////////////////

#endif  // !FWPROXY_PUBLIC_INC_

