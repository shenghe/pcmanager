#include "StdAfx.h"
#include "bkutilityimp.h"
#include "miniutil/bksvcmdl.h"
#include "unknownreport.h"
#include "unkownrescan.h"
#include "log.h"
#include "globalwhitelist.h"

CBkUtilityImp::CBkUtilityImp(void)
{
}

CBkUtilityImp::~CBkUtilityImp(void)
{
}

HRESULT CBkUtilityImp::Initialize( DWORD dwProcId, int nVersion )
{
    HRESULT hr = S_OK;

    if ( BKCOM_EXPORT_MODULE_CBkUtility_VER != nVersion )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    BKSVC_DEBUG_TRACE( L"[CBkUtilityImp] Initialize\n" );
    hr = __super::Initialize( dwProcId, nVersion );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

Exit0:
    return hr;
}

HRESULT CBkUtilityImp::Uninitialize()
{
    HRESULT hr = S_OK;

    BKSVC_DEBUG_TRACE( L"[CBkUtilityImp] Uninitialize\n" );
    hr = __super::Uninitialize();

    return hr;
}

HRESULT CBkUtilityImp::ReportFile()
{
    CUnkownReport::Instance().StartReportUnkownFile();

    return S_OK;
}

HRESULT CBkUtilityImp::SettingChange()
{
    CUnkownRescan::Instance().SettingChange();
    CUnkownReport::Instance().SettingChange();
    return S_OK;
}

HRESULT CBkUtilityImp::QueryLastScanInfo(
    DWORD dwScanMode, 
    BK_SCAN_INFO &ScanInfo
    )
{
    ScanInfo.Reset();

    ScanInfo.ScanMode = dwScanMode;

    return CScanInfoLog::LoadScanInfo( ScanInfo );
}

HRESULT CBkUtilityImp::EnableAvEngine()
{
    return CScanEngineProxy::Instance().EnableAvEngine();
}

HRESULT CBkUtilityImp::DisableAvEngine()
{
    return CScanEngineProxy::Instance().DisableAvEngine();
}

HRESULT CBkUtilityImp::LoadDaemonAvSign()
{
    return CScanEngineProxy::Instance().LoadDaemonAvSign();
}

HRESULT CBkUtilityImp::UnloadDaemonAvSign()
{
    return CScanEngineProxy::Instance().UnloadDaemonAvSign();
}

HRESULT CBkUtilityImp::ReloadAvSign()
{
    return CScanEngineProxy::Instance().ReloadAvSign();
}

HRESULT CBkUtilityImp::WhiteListUpdate()
{
    CGlobalWhiteList::Instance().LoadGlobalWhiteList();
    return S_OK;
}





//////////////////////////////////////////////////////////////////////////

HRESULT CBkPluginReportImp::Initialize( DWORD dwProcId, int nVersion )
{
    HRESULT hr = S_OK;

    if ( BKCOM_EXPORT_MODULE_CBkPluginReport_VER != nVersion )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    hr = __super::Initialize( dwProcId, nVersion );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

Exit0:
    return hr;
}

HRESULT CBkPluginReportImp::Uninitialize()
{
    HRESULT hr = S_OK;

    hr = __super::Uninitialize();

    return hr;
}

HRESULT CBkPluginReportImp::PluginReport(
    CAtlArray<BK_PLUGIN_INFO> &PluginList
    )
{
    for (size_t i = 0; i < PluginList.GetCount(); ++i)
    {
        CUnkownReport::Instance().AddUnkownFile(PluginList[i].strPath, FILE_TRACK_REPORT_NONPE);
    }

    //CUnkownReport::Instance().StartReportUnkownFile();
    return S_OK;
}