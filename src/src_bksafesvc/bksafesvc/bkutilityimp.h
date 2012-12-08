#pragma once
#include "comproxy/com_utility_interface.h"
#include "com_svc_dispatch.h"
#include "comproxy/com_utility_xdx.h"


class CBkUtilityImp
    :   public CBkComRoot,
        public IBkUtility
{
public:
    CBkUtilityImp(void);
    ~CBkUtilityImp(void);
public:
    HRESULT Initialize( DWORD dwProcId, int nVersion );
    HRESULT Uninitialize();

    COM_METHOD_DISPATCH_BEGIN()
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkUtility, ReportFile )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkUtility, SettingChange )
        COM_METHOD_DISPATCH_IN_ARG_1_OUT( CBkUtility, QueryLastScanInfo, dwScanMode, BK_SCAN_INFO )

        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkUtility, EnableAvEngine )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkUtility, DisableAvEngine )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkUtility, LoadDaemonAvSign )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkUtility, UnloadDaemonAvSign )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkUtility, ReloadAvSign )

        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkUtility, WhiteListUpdate )
    COM_METHOD_DISPATCH_END
public:
    virtual HRESULT ReportFile();
    virtual HRESULT SettingChange();
    virtual HRESULT QueryLastScanInfo( DWORD dwScanMode, BK_SCAN_INFO& ScanInfo );

    virtual HRESULT EnableAvEngine();
    virtual HRESULT DisableAvEngine();
    virtual HRESULT LoadDaemonAvSign();
    virtual HRESULT UnloadDaemonAvSign();
    virtual HRESULT ReloadAvSign();
    virtual HRESULT WhiteListUpdate();
};

class CBkPluginReportImp
    :   public  CBkComRoot,
        public  IBkPluginReport
{
public:
    CBkPluginReportImp(void){}
    ~CBkPluginReportImp(void){}
public:
    HRESULT Initialize( DWORD dwProcId, int nVersion );
    HRESULT Uninitialize();

    COM_METHOD_DISPATCH_BEGIN()
        COM_METHOD_DISPATCH_IN_ARG_1_NO_OUT( CBkPluginReport, PluginReport, PluginList )
    COM_METHOD_DISPATCH_END
public:
    virtual HRESULT PluginReport( CAtlArray<BK_PLUGIN_INFO>& PluginList );
};