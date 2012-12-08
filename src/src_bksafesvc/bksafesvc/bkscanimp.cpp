#include "stdafx.h"
#include "bkscanimp.h"
#include "miniutil/bksvcmdl.h"
#include "unknownreport.h"
#include "insrecycle.h"
#include "setting.h"
#include "skylark2/bkunknownreport.h"
#include "globalwhitelist.h"


class CScanWorkProxy: public IScanFileCallBack
{
public:
    CScanWork m_obj;
    CUnkownFile m_ukfile;

public:

    HRESULT AfterScanFile( 
        LPCWSTR lpszFilePath, 
        const Skylark::BKENG_RESULT* pResult, 
        void* pvFlag
        )
    {
        if ( !pResult->bIsVirus )
        {
            if ( pResult->bNeedUpload )
            {
                m_ukfile.AddUnkownFile( lpszFilePath, FILE_TRACK_FILE );
            }
        }

        return S_OK;
    }

    HRESULT BeforeCleanFile( 
        LPCWSTR lpszFilePath, 
        const Skylark::BKENG_RESULT* pResult,
        void* pvFlag,
        UINT64* pulBackId /* = NULL */ 
        )
    {
        if ( pResult->bNeedUpload )
        {
            if ( pulBackId )
            {
                CString strId;

                strId.Format(L"%I64u", *pulBackId );
                m_ukfile.AddUnkownFile( strId, FILE_TRACK_QUARANTINE );
            }
        }

        return S_OK;
    }
};


class   CBkScanImpRes : public CResBase<CScanWorkProxy>
{
public:
    virtual ~CBkScanImpRes(){}
    virtual HRESULT Destroy()
    {
        if ( m_pi )
        {
            m_pi->m_obj.Uninitialize();
            delete m_pi;
            m_pi = NULL;
        }

        return S_OK;
    }
};







CBkScanImp::CBkScanImp(void) :
    m_pScanIns( NULL )
{
}

CBkScanImp::~CBkScanImp(void)
{
}

HRESULT CBkScanImp::Initialize( DWORD dwProcId, int nVersion )
{
    //DWORD dwTickBegin = ::GetTickCount();

    HRESULT hr = S_OK;

    if ( BKCOM_EXPORT_MODULE_CBkScan_VER != nVersion )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    BKSVC_DEBUG_TRACE( L"[CBkScanImp] Initialize\n" );
    hr = __super::Initialize( dwProcId, nVersion );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    {
        CObjGuard   guard( m_InsLock );
        if ( NULL == m_pScanIns )
        {
            m_pScanIns = new CScanWorkProxy;
            if ( NULL == m_pScanIns )
            {
                hr = E_OUTOFMEMORY;
                goto Exit0;
            }
        }
        else
        {
            goto Exit0;
        }
    }

    hr = m_pScanIns->m_obj.Initialize( dwProcId, m_pScanIns );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    CInsRecycle::Instance().RegisterActiveRes();

    m_setting.Reset();
    CSvcSetting::Instance().LoadScanSetting( m_setting );
Exit0:

    //DWORD dwTickEnd = ::GetTickCount();

    //CString strMessage;
    //strMessage.Format(L"--------------------> Initialize %lu", dwTickEnd - dwTickBegin);
    //::OutputDebugString(strMessage);
    return hr;
}

HRESULT CBkScanImp::Uninitialize()
{
    HRESULT hr = S_OK;

    {
        CObjGuard   guard( m_InsLock );
        if ( m_pScanIns )
        {
           CBkScanImpRes* pRes = new CBkScanImpRes();
           if ( pRes )
           {
               hr = pRes->ThrowToRecycle( m_pScanIns );
               if ( SUCCEEDED( hr ) )
               {
                   m_pScanIns = NULL;
                   goto Exit0;
               }
           }
        }
        else
        {
            goto Exit0;
        }
    }

    if ( m_pScanIns )
    {
        m_pScanIns->m_obj.Uninitialize();
        delete m_pScanIns;
        m_pScanIns = NULL;
    }

Exit0:
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] Uninitialize\n" );
    
    CInsRecycle::Instance().UnregisterActiveRes();

    hr = __super::Uninitialize();

    return hr;
}

HRESULT CBkScanImp::QuerySteps( int nScanMode, CAtlArray<int>& steps )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] QuerySteps\n" );
    HRESULT hr = S_OK;
    if ( NULL == m_pScanIns )
    {
        hr = E_NOINTERFACE;
        goto Exit0;
    }

    hr = m_pScanIns->m_obj.QueryScanStep( nScanMode, steps );
Exit0:
    return hr;
}

HRESULT CBkScanImp::Scan( int nScanMode, BOOL bBackGround, CAtlArray<CString>& lpParam )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] Scan\n" );
#ifdef _DEBUG
    for ( size_t i = 0; i < lpParam.GetCount(); i++ )
    {
        BKSVC_DEBUG_TRACE( L"[CBkScanImp] path[%d]=%s\n", i, lpParam[ i ] );
    }
#endif

    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    m_pScanIns->m_obj.SetScanSetting( m_setting );

    hr = m_pScanIns->m_obj.Scan( nScanMode, bBackGround, lpParam );
Exit0:
    return hr;
}

HRESULT CBkScanImp::Scan2( int nScanMode, BOOL bBackGround, CAtlArray<BK_SCAN_ADDITIONAL_PATH>& lpParam )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] Scan2\n" );
#ifdef _DEBUG
    for ( size_t i = 0; i < lpParam.GetCount(); i++ )
    {
        BKSVC_DEBUG_TRACE( L"[CBkScanImp] path[%d]=%s\n", i, lpParam[ i ].strFilePath );
    }
#endif

    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    m_pScanIns->m_obj.SetScanSetting( m_setting );

    hr = m_pScanIns->m_obj.Scan2( nScanMode, bBackGround, lpParam );
Exit0:
    return hr;
}

HRESULT CBkScanImp::Pause()
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] Pause\n" );

    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pScanIns->m_obj.Pause();
Exit0:
    return hr;
}

HRESULT CBkScanImp::Resume()
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] Resume\n" );
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pScanIns->m_obj.Resume();
Exit0:
    return hr;
}

HRESULT CBkScanImp::Stop()
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] Stop\n" );
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pScanIns->m_obj.Stop();
Exit0:
    return hr;
}

HRESULT CBkScanImp::QueryScanInfo( BK_SCAN_INFO& scanInfo )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] QueryScanInfo\n" );
#ifdef _DEBUG
    scanInfo.CantCleanVirus = 10;
    scanInfo.CleanedVirus = 101;
    scanInfo.CleanFailedCount = 2;
    scanInfo.LastScanFileName = L"f:\\virus list\\2009-10-2\\YZ@=U~ZIOT";
    scanInfo.Progress = 100;
    scanInfo.QueryFailedCount = 0;
    scanInfo.RebootCount = 123;
    scanInfo.SafeCount = 1111111;
    scanInfo.ScanFinished = TRUE;
    //scanInfo.
#endif
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pScanIns->m_obj.QueryScanInfo( scanInfo );
Exit0:
    return hr;
}

HRESULT CBkScanImp::QueryCleanInfo( BK_CLEAN_INFO& cleanInfo )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] QueryCleanInfo\n" );
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }
    hr = m_pScanIns->m_obj.QueryCleanInfo( cleanInfo );
Exit0:
    return hr;
}

HRESULT CBkScanImp::QueryFileInfo( int nType, DWORD dwIndex, BK_FILE_INFO& fileInfo )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] QueryFileInfo\n" );
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pScanIns->m_obj.QueryFileInfo( nType, dwIndex, fileInfo );
Exit0:
    return hr;
}

HRESULT CBkScanImp::Clean( CAtlArray<DWORD>& fileIndex )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] Clean\n" );
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pScanIns->m_obj.Clean( fileIndex );
Exit0:
    return hr;
}

HRESULT CBkScanImp::QueryLastScanInfo( int nScanMode, BK_SCAN_INFO& scanInfo )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] QueryLastScanInfo\n" );
#ifdef _DEBUG
    //scanInfo.CantCleanVirus = 10;
    //scanInfo.CleanedVirus = 101;
    //scanInfo.CleanFailedCount = 2;
    //scanInfo.LastScanFileName = L"f:\\virus list\\2009-10-2\\YZ@=U~ZIOT";
    //scanInfo.Progress = 100;
    //scanInfo.QueryFailedCount = 0;
    //scanInfo.RebootCount = 123;
    //scanInfo.SafeCount = 1111111;
    //scanInfo.ScanFinished = TRUE;
#endif
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    scanInfo.ScanMode = nScanMode;
    hr = m_pScanIns->m_obj.QueryLastScanInfo( scanInfo );
Exit0:
    return hr;
}

HRESULT CBkScanImp::SetScanSetting( BK_SCAN_SETTING& setting )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] SetScanSetting\n" );
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    memcpy( &m_setting, &setting, sizeof( BK_SCAN_SETTING ) );
Exit0:
    return hr;
}

HRESULT CBkScanImp::GetScanSetting( BK_SCAN_SETTING& setting )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanImp] GetScanSetting\n" );
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pScanIns->m_obj.GetScanSetting( setting );

    setting.bAutoClean = m_setting.bAutoClean;
Exit0:
    return hr;
}

HRESULT CBkScanImp::ReportFile()
{
    return m_pScanIns->m_ukfile.ReportFile();
}

HRESULT CBkScanImp::AddUnknownFile(LPCWSTR lpszFilePath)
{
	// 整合UnknownFile至一个模块，以方便展现

	m_pScanIns->m_ukfile.AddUnkownFile( lpszFilePath, FILE_TRACK_FILE);
	return S_OK;
}

HRESULT CBkScanImp::Report()
{
	return ReportFile();
}

HRESULT CBkScanImp::IsAutoReport(BOOL& bIsAutoReport)
{
	return CSvcSetting::Instance().GetAutoReport( bIsAutoReport );
}
