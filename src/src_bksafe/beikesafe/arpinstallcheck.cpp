/************************************************************************
* @file      : arpinstallcheck.cpp
* @author    : gaoyi <gaoyi@kingsoft.com>
* @date      : 2011/1/25 17:10:33
* @brief     : 
*
* $Id: $
/************************************************************************/

#include "stdafx.h"
#include "arpinstallcheck.h"

// -------------------------------------------------------------------------
#define NDIS_ANTIARP_DRIVER       L"KNdisFlt"

BOOL IskArpInstalled()
{
    HRESULT hrResult = E_FAIL;
    BOOL bSvrRuning	= FALSE;
    BOOL bRet = FALSE;

    SC_HANDLE hSvrMgr = NULL;
    SC_HANDLE hSvr = NULL;

    ::SetLastError(0);

    do 
    {
        hSvrMgr = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if ( !hSvrMgr )
        {
            hrResult = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        hSvr = ::OpenService(hSvrMgr, NDIS_ANTIARP_DRIVER, SERVICE_QUERY_STATUS);
        if ( !hSvr )
        {
            hrResult = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if ( hSvr )
        {
            SERVICE_STATUS svrstatus = { 0 };
            if ( !::QueryServiceStatus(hSvr, &svrstatus) )
            {
                hrResult = HRESULT_FROM_WIN32(GetLastError());
                break;
            }

            bRet = (svrstatus.dwCurrentState == SERVICE_RUNNING);
        }

        hrResult = S_OK;

    } while (FALSE);

    if ( hSvr )
    {
        ::CloseServiceHandle(hSvr);
        hSvr = NULL;
    }
    if ( hSvrMgr )
    {
        ::CloseServiceHandle(hSvrMgr);
        hSvrMgr = NULL;
    }

    return bRet;
}



// -------------------------------------------------------------------------
// $Log: $
