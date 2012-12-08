#include "stdafx.h"
#include "kclrmonerr.h"
#include "kclrmonsetting.h"
#include <atltime.h>


//////////////////////////////////////////////////////////////////////////

KClearMonitorSetting::KClearMonitorSetting()
{
    CHAR szCfgPath[MAX_PATH] = { 0 };

    GetModuleFileNameA(NULL, szCfgPath, MAX_PATH);
    PathRemoveFileSpecA(szCfgPath);
    PathAppendA(szCfgPath, "cfg\\kclear.ini");

    m_strCfgFile = szCfgPath;
}

KClearMonitorSetting::~KClearMonitorSetting()
{

}

//////////////////////////////////////////////////////////////////////////

HRESULT KClearMonitorSetting::GetEnableMointor(
    BOOL* pbEnable
    )
{
    HRESULT hr = E_FAIL;
    DWORD dwRetCode;
    CHAR szValue[256] = { 0 };

    dwRetCode = GetPrivateProfileStringA("kclear",
                                         "enable_monitor",
                                         "true",
                                         szValue,
                                         256,
                                         m_strCfgFile);
    if (!dwRetCode)
    {
        *pbEnable = TRUE;
    }
    else
    {
        *pbEnable = stricmp(szValue, "true") ? FALSE : TRUE;
    }

    hr = S_OK;

    return hr;
}

HRESULT KClearMonitorSetting::SetEnableMointor(
    BOOL bEnable
    )
{
    HRESULT hr = E_FAIL;
    BOOL bRetCode;

    bRetCode = WritePrivateProfileStringA("kclear",
                                          "enable_monitor",
                                          bEnable ? "true" : "false",
                                          m_strCfgFile);
    if (!bRetCode)
    {
        hr = GetCurError();
        goto clean0;
    }

    hr = S_OK;

clean0:
    return hr;
}

HRESULT KClearMonitorSetting::IsNotifyCoolDown(
    BOOL* pbCoolDown
    )
{
    HRESULT hr = E_FAIL;
    CTime* pLastHotTime = NULL;
    CTime nowTime;
    CHAR szValue[256] = { 0 };
    DWORD dwRetCode;
    int nYear, nMonth, nDay, nHour, nMinute, nSecond;
    CTimeSpan timeSpan;

    if (!pbCoolDown)
    {
        hr = E_INVALIDARG;
        goto clean0;
    }

    *pbCoolDown = TRUE;

    dwRetCode = GetPrivateProfileStringA("kclear",
                                         "last_notify",
                                         "2011-1-1 0:0:0",
                                         szValue,
                                         256,
                                         m_strCfgFile);
    if (!dwRetCode)
        goto clean0;
    
    if (!strlen(szValue))
        goto clean0;

    sscanf(szValue, "%d-%d-%d %d:%d:%d", &nYear, &nMonth, &nDay, &nHour, &nMinute, &nSecond);

    if (nYear < 1970 || nYear > 3000)
        goto clean0;

    pLastHotTime = new CTime(nYear, nMonth, nDay, nHour, nMinute, nSecond);
    nowTime = CTime::GetCurrentTime();

    if (nowTime.GetYear() < 1970 || nowTime.GetYear() > 3000)
        goto clean0;

    timeSpan = nowTime - *pLastHotTime;
    if (timeSpan.GetDays() >= 1)
    {
        *pbCoolDown = TRUE;
    }
    else
    {
        *pbCoolDown = FALSE;
    }

    hr = S_OK;

clean0:
    if (pLastHotTime)
    {
        delete pLastHotTime;
        pLastHotTime = NULL;
    }

    return hr;
}

HRESULT KClearMonitorSetting::RecordLastNotify(
    )
{
    HRESULT hr = E_FAIL;
    CTime nowTime;
    char szTime[128] = { 0 };
    BOOL bRetCode;

    nowTime = CTime::GetCurrentTime();
    if (nowTime.GetYear() < 1970 || nowTime.GetYear() > 3000)
        goto clean0;

    sprintf(szTime, "%d-%d-%d %d:%d:%d", 
        nowTime.GetYear(),
        nowTime.GetMonth(),
        nowTime.GetDay(),
        nowTime.GetHour(),
        nowTime.GetMinute(),
        nowTime.GetSecond()
        );

    bRetCode = WritePrivateProfileStringA("kclear",
                                          "last_notify",
                                          szTime,
                                          m_strCfgFile);
    if (!bRetCode)
    {
        hr = GetCurError();
        goto clean0;
    }

    hr = S_OK;

clean0:
    return hr;
}

//////////////////////////////////////////////////////////////////////////
