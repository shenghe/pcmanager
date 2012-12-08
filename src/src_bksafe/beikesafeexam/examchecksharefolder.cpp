#include "stdafx.h"
#include "examchecksharefolder.h"
// #include <Wbemidl.h>
// 
// #pragma comment(lib, "Wbemuuid.lib")
// 
// BOOL _CheckFromWMI()
// {
//     BOOL bRet = FALSE;
// 
//     CComPtr<IWbemLocator> spiLoc;
//     CComPtr<IWbemServices> spiSvc;
//     CComPtr<IEnumWbemClassObject> spiEnumerator;
// 
//     HRESULT hr = ::CoCreateInstance(
//         CLSID_WbemLocator, 
//         0,
//         CLSCTX_INPROC_SERVER,
//         IID_IWbemLocator,
//         (void**) &spiLoc);
//     if (FAILED(hr))
//         goto Exit0;
// 
//     hr = spiLoc->ConnectServer(
//         CComBSTR(L"root\\cimv2"),
//         NULL, 
//         NULL,
//         0,
//         NULL,
//         0, 
//         0,
//         &spiSvc);
//     if (FAILED(hr))
//         goto Exit0;
// 
//     hr = ::CoSetProxyBlanket(
//         spiSvc,
//         RPC_C_AUTHN_WINNT,
//         RPC_C_AUTHZ_NONE,
//         NULL,
//         RPC_C_AUTHN_LEVEL_CALL,
//         RPC_C_IMP_LEVEL_IMPERSONATE,
//         NULL,
//         EOAC_NONE);
//     if (FAILED(hr))
//         goto Exit0;
// 
//     hr = spiSvc->ExecQuery(
//         CComBSTR("WQL"), 
//         CComBSTR("SELECT * FROM Win32_Share"),
//         WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
//         NULL,
//         &spiEnumerator);
//     if (FAILED(hr))
//         goto Exit0;
// 
//     while (TRUE)
//     {
//         CComVariant varName, varPath, varType;
//         CComPtr<IWbemClassObject> spiClsObj;
//         ULONG uReturn = 0;
// 
//         hr = spiEnumerator->Next(WBEM_INFINITE, 1, &spiClsObj, &uReturn);
//         if(FAILED(hr) || 0 == uReturn)
//             break;
// 
//         hr = spiClsObj->Get(CComBSTR("Name"), 0, &varName, NULL, NULL);
//         hr = spiClsObj->Get(CComBSTR("Path"), 0, &varPath, NULL, NULL);
//         hr = spiClsObj->Get(CComBSTR("Type"), 0, &varType, NULL, NULL);
// 
//         DEBUG_TRACE(L" Share Folder: %s, 0x%08X, %s\r\n", varName.bstrVal, varType.uintVal, varPath.bstrVal);
//     }
// 
// 
//     bRet = TRUE;
// 
// Exit0:
// 
//     return bRet;
// }

#include <lm.h>

#pragma comment(lib, "Netapi32.lib")

BOOL _CheckUsingNetShareAPI()
{
    BOOL bHasShareFolder = FALSE;
    PSHARE_INFO_2 pInfoRet = NULL;
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwReadCount = 0, dwTotalCount = 0, dwResume = 0;

    DEBUG_TRACE(L"Share:              Local Path:                   Type:\r\n");
    DEBUG_TRACE(L"------------------------------------------------------------\r\n");
    do
    {
        dwRet = ::NetShareEnum(NULL, 2, (LPBYTE *)&pInfoRet, -1, &dwReadCount, &dwTotalCount, &dwResume);
        if(ERROR_SUCCESS == dwRet || ERROR_MORE_DATA == dwRet)
        {
            for(DWORD i = 0;i < dwReadCount; i++)
            {
                DEBUG_TRACE(L"%-20s%-30s0x%08X\r\n", pInfoRet[i].shi2_netname, pInfoRet[i].shi2_path, pInfoRet[i].shi2_type);

                if (STYPE_DISKTREE == pInfoRet[i].shi2_type)
                {
                    bHasShareFolder = TRUE;

#                   ifndef _DEBUG
                    break;
#                   endif
                }
            }

            ::NetApiBufferFree(pInfoRet);
        }
        else 
            DEBUG_TRACE(L"Error: %ld\r\n", dwRet);

#       ifndef _DEBUG
        if (bHasShareFolder)
            break;
#       endif
    }
    while (ERROR_MORE_DATA == dwRet);

    return bHasShareFolder;
}

BOOL CExamCheckShareFolder::Check()
{
    return _CheckUsingNetShareAPI();
}
