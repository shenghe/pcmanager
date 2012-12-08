/*
Copyright (C) Kingsoft Corporation

Creator:
    Leo Jiang <JiangFengbing@kingsoft.net>

Description:
    Interface of IKisServiceEvent

Environment:
    Application
*/

#ifndef KISSVCPUBLIC_INC_
#define KISSVCPUBLIC_INC_

#include <unknwn.h>

//////////////////////////////////////////////////////////////////////////

typedef struct tagKisServiceName {

    WCHAR   wszName[32];

} KISSVC_NAME, *PKISSVC_NAME;

//////////////////////////////////////////////////////////////////////////

[
    uuid("d01225b5-2e6e-4b26-824e-96aa8944b06f")
]
interface IKisServiceEvent : IUnknown
{
    virtual STDMETHODIMP OnInitialize(HINSTANCE hInstance) = 0;

    virtual STDMETHODIMP OnQueryName(KISSVC_NAME* pName) = 0;

    virtual STDMETHODIMP OnStart(void) = 0;

    virtual STDMETHODIMP OnPause(void) = 0;

    virtual STDMETHODIMP OnContinue(void) = 0;

    virtual STDMETHODIMP OnStop(void) = 0;

    virtual STDMETHODIMP OnUnInitialize(void) = 0;

};

//////////////////////////////////////////////////////////////////////////

#endif  // !KISSVCPUBLIC_INC_
