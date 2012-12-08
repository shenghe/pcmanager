#include "stdafx.h"
#include <shlwapi.h>
#include <strsafe.h>
#include <process.h>
#include "kclrmonerr.h"
#include "kclrmon.h"
#include "kclrmonsetting.h"

//////////////////////////////////////////////////////////////////////////
// 定时器时间设置
const UINT g_kLackingMonWait = 5 * 60 * 1000;  // 5分钟

//////////////////////////////////////////////////////////////////////////

KClearMonitor::KClearMonitor() 
    : m_hTimerThread(NULL)
    , m_hExitEvent(NULL)
    , m_piCallback(NULL)
    , m_bVistaOrLater(TRUE)
    , m_qwSysDriveFreeSizeInMB(0)
{

}

KClearMonitor::~KClearMonitor()
{

}

//////////////////////////////////////////////////////////////////////////

HRESULT KClearMonitor::Initialize(IKClearMonitorCallback* piCallback)
{
    DWORD dwRetCode;
    CHAR szSysVol[MAX_PATH] = { 0 };
    OSVERSIONINFO osVersion = { 0 };

    dwRetCode = GetSystemDirectoryA(szSysVol, MAX_PATH);
    //ASSERT(dwRetCode);

    szSysVol[3] = _T('\0');
    m_strSysDrive = szSysVol;

    osVersion.dwOSVersionInfoSize = sizeof osVersion;
    GetVersionEx(&osVersion);

    m_bVistaOrLater = osVersion.dwMajorVersion > 5 ? TRUE : FALSE;

    m_piCallback = piCallback;
    return S_OK;
}

HRESULT KClearMonitor::UnInitialize()
{
    return S_OK;
}

HRESULT KClearMonitor::IsSystemDriveLacking(
    /*[IN]*/ int nSysVol,
    /*[OUT]*/ char* szSysVol,
    /*[OUT]*/ BOOL* pbLacking
    )
{
    HRESULT hr = E_FAIL;
    BOOL bRetCode;
    
    if (!pbLacking || !szSysVol)
    {
        hr = E_INVALIDARG;
        goto clean0;
    }

    if (m_strSysDrive.GetLength() + 1 > nSysVol)
    {
        hr = E_INVALIDARG;
        goto clean0;
    }

    strcpy(szSysVol, (const char*)m_strSysDrive);

    bRetCode = IsSystemDriveLacking(*pbLacking);
    if (!bRetCode)
    {
        hr = GetCurError();
        goto clean0;
    }

    hr = S_OK;

clean0:
    return hr;
}

HRESULT KClearMonitor::LaunchKClear(
    /*[IN_OPT]*/ const char* pSubTab
    )
{
    HRESULT hr = E_FAIL;
    CHAR szKSafeApp[MAX_PATH] = { 0 };
    CHAR szCmdline[MAX_PATH * 2] = { 0 };
    DWORD dwRetCode;
    UINT uRetCode;

    dwRetCode = GetModuleFileNameA(NULL, szKSafeApp, MAX_PATH);
    if (!dwRetCode)
    {
        hr = GetCurError();
        goto clean0;
    }

    PathRemoveFileSpecA(szKSafeApp);
    PathAppendA(szKSafeApp, "KSafe.exe");

    if (pSubTab)
    {
        StringCchPrintfA(szCmdline, MAX_PATH * 2, "\"%s\" -do:ui_KClear_%s", szKSafeApp, pSubTab);
    }
    else
    {
        StringCchPrintfA(szCmdline, MAX_PATH * 2, "\"%s\" -do:ui_KClear_ClrRub", szKSafeApp);
    }

    uRetCode = WinExec(szCmdline, SW_HIDE);
    if (uRetCode < 32)
    {
        hr = GetCurError();
        goto clean0;
    }

    hr = S_OK;

clean0:
    return hr;
}

// 磁盘空间不足规则：
// WinXP:
// 1、  系统盘总容量小于 10G，且剩余空间小于500M
// 
// 2、  系统盘总容量小于20G，且剩余空间小于1.2 G
// 
// 3、  剩余空间小于总容量7 %
// 
// 
// Vista/Win7:
// 1、  系统盘总容量低于20G，且剩余空间小于1.2 G
// 
// 2、  系统盘总容量低于30G，且剩余空间小于2 G
// 
// 3、  剩余空间小于总容量 7%

BOOL KClearMonitor::IsSystemDriveLacking(BOOL& bLacking)
{
    BOOL retval = FALSE;
    DWORD dwSectorsPerCluster;
    DWORD dwBytesPerSector;
    DWORD dwNumberOfFreeClusters;
    DWORD dwTotalNumberOfClusters;
    BOOL bRetCode;
    ULONGLONG qwTotalSize;
    ULONGLONG qwFreeSize;

    bLacking = FALSE;
    
    bRetCode = GetDiskFreeSpaceA(m_strSysDrive,
                                 &dwSectorsPerCluster,
                                 &dwBytesPerSector,
                                 &dwNumberOfFreeClusters,
                                 &dwTotalNumberOfClusters);
    if (!bRetCode)
        goto clean0;

    qwTotalSize = (ULONGLONG)dwSectorsPerCluster * (ULONGLONG)dwBytesPerSector * (ULONGLONG)dwTotalNumberOfClusters;
    qwFreeSize = (ULONGLONG)dwSectorsPerCluster * (ULONGLONG)dwBytesPerSector * (ULONGLONG)dwNumberOfFreeClusters;

    m_qwSysDriveFreeSizeInMB = qwFreeSize / (1024 * 1024);

    if (m_bVistaOrLater)
    {
        if (qwTotalSize < GigaByte(20))
        {
            if (qwFreeSize < GigaByte(1.2))
                bLacking = TRUE; 
        }
        else
        {
            if (qwTotalSize < GigaByte(30))
            {
                if (qwFreeSize < GigaByte(2))
                    bLacking = TRUE;
            }
            else
            {
                if (qwFreeSize < (double)0.07 * qwTotalSize)
                    bLacking = TRUE;
            }
        }
    }
    else
    {
        if (qwTotalSize < GigaByte(10))
        {
            if (qwFreeSize < GigaByte(0.5))
                bLacking = TRUE; 
        }
        else
        {
            if (qwTotalSize < GigaByte(20))
            {
                if (qwFreeSize < GigaByte(1.2))
                    bLacking = TRUE;
            }
            else
            {
                if (qwFreeSize < (double)0.07 * qwTotalSize)
                    bLacking = TRUE;
            }
        }
    }

    retval = TRUE;

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

HRESULT KClearMonitor::StartMonitor()
{
    HRESULT hr = E_FAIL;

    if (m_hTimerThread)
    {
        hr = S_OK;
        goto clean0;
    }

    m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hTimerThread = (HANDLE)_beginthreadex(NULL, 0, TimerThreadProc, this, 0, NULL);
    if (!m_hTimerThread)
    {
        hr = GetCurError();
        goto clean0;
    }

    hr = S_OK;

clean0:
    return hr;
}

HRESULT KClearMonitor::StopMonitor()
{
    if (m_hTimerThread)
    {
        SetEvent(m_hExitEvent);
        WaitForSingleObject(m_hTimerThread, INFINITE);
        CloseHandle(m_hTimerThread);
        m_hTimerThread = NULL;
    }

    if (m_hExitEvent)
    {
        CloseHandle(m_hExitEvent);
        m_hExitEvent = NULL;
    }

    return S_OK;
}

UINT KClearMonitor::TimerThreadProc(void* pParam)
{
    KClearMonitor* pThis = (KClearMonitor*)pParam;
    DWORD dwWait;
    HRESULT hr;
    KComObject<KClearMonitorSetting> setting;
    BOOL bEnableMon = TRUE;

    if (!pThis)
        goto clean0;

    if (!pThis->m_piCallback)
        goto clean0;

    for (;;)
    {
        BOOL bLacking = FALSE;

        dwWait = WaitForSingleObject(pThis->m_hExitEvent, g_kLackingMonWait);
        if (WAIT_OBJECT_0 == dwWait)
            break;

        setting.GetEnableMointor(&bEnableMon);
        if (!bEnableMon)
            continue;

        hr = pThis->IsSystemDriveLacking(bLacking);
        if (FAILED(hr))
            continue;

        if (bLacking)
        {
            // 通知回调函数
            pThis->m_piCallback->OnSystemDriveLacking();
            goto clean0;    // 不再监控
        }
    }

clean0:
    _endthreadex(0);
    return 0;
}

//////////////////////////////////////////////////////////////////////////

ULONGLONG KClearMonitor::GigaByte(double size)
{
    return (ULONGLONG)(size * ((ULONGLONG)1024 * (ULONGLONG)1024 * (ULONGLONG)1024));
}

//////////////////////////////////////////////////////////////////////////

HRESULT KClearMonitor::GetSystemDriveFreeSize(
    ULONGLONG* pqwSizeInMB
    )
{
    HRESULT hr = E_FAIL;

    if (!pqwSizeInMB)
    {
        hr = E_INVALIDARG;
        goto clean0;
    }

    if (!m_qwSysDriveFreeSizeInMB)
        goto clean0;

    *pqwSizeInMB = m_qwSysDriveFreeSizeInMB;
    hr = S_OK;

clean0:
    return hr;
}

//////////////////////////////////////////////////////////////////////////
