/*
Copyright (C) Kingsoft Corporation

Creator:
    Leo Jiang <JiangFengbing@kingsoft.net>

Description:
    Interface of IServiceCallClient

Environment:
    Application
*/

#ifndef KISSERVICECALLPUBLIC_INC_
#define KISSERVICECALLPUBLIC_INC_

#include <unknwn.h>

//////////////////////////////////////////////////////////////////////////

[
    uuid("08d9c551-8062-46b8-8b51-1343f13efe3d")
]
interface IServiceCallClient : IUnknown
{
    virtual STDMETHODIMP Initialize(IUnknown* pSetting) = 0;

    virtual STDMETHODIMP UnInitialize(void) = 0;

    virtual STDMETHODIMP IsSystemMode(BOOL* pfSystemMode) = 0;

    virtual STDMETHODIMP GetCurrentSessionId(DWORD* pdwSessionId) = 0;

    virtual STDMETHODIMP CreateSystemProcess(
        const WCHAR cwszAppName[], 
        const WCHAR cwszParamer[]
        ) = 0;

    virtual STDMETHODIMP CreateSystemProcess2(
        DWORD dwSession,
        const WCHAR cwszAppName[],
        const WCHAR cwszParamer[]
        ) = 0;

    virtual STDMETHODIMP CreateSystemProcess3(
        DWORD dwSession,
        const WCHAR cwszDesktop[],
        const WCHAR cwszAppName[],
        const WCHAR cwszParamer[]
        ) = 0;

    virtual STDMETHODIMP CreateUserProcess(
        BOOL* pfRetCode,	// 返回执行结果
        LPCTSTR lpApplicationName, 	// 以下参数依次等同于CreateProcess
        LPTSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles,
        DWORD dwCreationFlags,
        LPVOID lpEnvironment,
        LPCTSTR lpCurrentDirectory,
        LPSTARTUPINFO lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation
        ) = 0;

    virtual STDMETHODIMP IncreasePrivilege(void) = 0;

    virtual STDMETHODIMP DecreasePrivilege(void) = 0;
};

//////////////////////////////////////////////////////////////////////////

#endif  // KISSERVICECALLPUBLIC_INC_
