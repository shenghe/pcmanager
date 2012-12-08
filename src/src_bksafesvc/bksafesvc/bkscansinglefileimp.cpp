#include "StdAfx.h"
#include "bkscansinglefileimp.h"
#include "miniutil/bksvcmdl.h"
#include "setting.h"
#include "unknownreport.h"

CBkScanSingleFileImp::CBkScanSingleFileImp(void)  :
    m_pFileScan( NULL ),
    m_bOnline( FALSE )
{

}

CBkScanSingleFileImp::~CBkScanSingleFileImp(void)
{
}

HRESULT CBkScanSingleFileImp::Initialize( DWORD dwProcId, int nVersion )
{
    HRESULT hr = S_OK;

    if ( BKCOM_EXPORT_MODULE_CBkScanSingleFile_VER != nVersion )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    BKSVC_DEBUG_TRACE( L"[CBkScanSingleFileImp] Initialize\n" );
    hr = __super::Initialize( dwProcId, nVersion );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    if ( m_pFileScan )
    {
        m_pFileScan->Uninitialize();
        delete  m_pFileScan;
        m_pFileScan = NULL;
    }

    if ( NULL == m_pFileScan )
    {
        m_pFileScan = new CBkFileScan();
        if ( NULL == m_pFileScan )
        {
            hr = E_OUTOFMEMORY;
            goto Exit0;
        }

        hr = m_pFileScan->Initialize( m_dwProcId );
        if ( FAILED( hr ) )
        {
            goto Exit0;
        }
    }
Exit0:
    return hr;
}

HRESULT CBkScanSingleFileImp::Uninitialize()
{
    HRESULT hr = S_OK;

    if ( m_pFileScan )
    {
        m_pFileScan->Uninitialize();
        delete  m_pFileScan;
        m_pFileScan = NULL;
    }

    BKSVC_DEBUG_TRACE( L"[CBkScanSingleFileImp] Uninitialize\n" );
    hr = __super::Uninitialize();

    return hr;
}

HRESULT CBkScanSingleFileImp::ScanFile( CString& strFile, DWORD dwScanMask, DWORD dwScanPolicy, BK_FILE_RESULT& fr )
{
    BKSVC_DEBUG_TRACE( L"[CBkScanSingleFileImp] ScanFile : %s\n", strFile );
    HRESULT hr = S_OK;
	BOOL    bAutoReport = FALSE;

    if ( ( dwScanPolicy & SCAN_POLICY_CLOUD ) == SCAN_POLICY_CLOUD )
    {
        m_bOnline = TRUE;
    }

    if ( NULL == m_pFileScan )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    m_setting.Reset();
    m_setting.bAutoClean = FALSE;
    m_setting.nScanPolicy = dwScanPolicy;
    m_setting.bScanArchive = TRUE;
    m_setting.nScanFileType = FALSE;
    m_setting.CleanFailedOp = CleanFailedDelete;

    hr = m_pFileScan->SetScanSetting( m_setting );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pFileScan->Scan( strFile, dwScanMask, fr );

	hr = CSvcSetting::Instance().GetAutoReport( bAutoReport );
	if ( FAILED( hr ) )
	{
		bAutoReport = FALSE;
	}

	if ( bAutoReport && ( BkQueryInfoFileTypeUnknown == fr.Type ) )
	{
		CUnkownReport::Instance().AddUnkownFile( 
			fr.FileName, 
			FILE_TRACK_FILE 
			);
	}
Exit0:
    return hr;
}

HRESULT CBkScanSingleFileImp::CleanFile( CString& strFile, DWORD dwScanMask, DWORD dwScanPolicy, BK_FILE_RESULT& fr )
{
    HRESULT hr = S_OK;

    if ( ( dwScanPolicy & SCAN_POLICY_CLOUD ) == SCAN_POLICY_CLOUD )
    {
        m_bOnline = TRUE;
    }

    if ( NULL == m_pFileScan )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    m_setting.Reset();
    m_setting.bAutoClean = TRUE;
    m_setting.nScanPolicy = dwScanPolicy;
    m_setting.bScanArchive = TRUE;
    m_setting.nScanFileType = FALSE;
    m_setting.CleanFailedOp = CleanFailedDelete;

    hr = m_pFileScan->SetScanSetting( m_setting );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pFileScan->Scan( strFile, dwScanMask, fr );
Exit0:
    return hr;
}

HRESULT CBkScanSingleFileImp::ForceClean( CString& strFile, BK_FORCE_CLEAN_DETAIL& detail, BK_FILE_RESULT& fr )
{
    HRESULT hr = S_OK;

	//assert( Skylark::SLLevelIsBlackKnown( fr.SecLevel ) );

	if ( NULL == m_pFileScan )
	{
		hr = E_NOTIMPL;
		goto Exit0;
	}

	m_setting.Reset();
	m_setting.bAutoClean	= TRUE;
	m_setting.nScanPolicy	= detail.dwScanPolicy;
	m_setting.bScanArchive	= TRUE;
	m_setting.nScanFileType = FALSE;
	m_setting.CleanFailedOp = CleanFailedDelete;

	hr = m_pFileScan->SetScanSetting( m_setting );
	if ( FAILED( hr ) )
	{
		goto Exit0;
	}

	hr = m_pFileScan->ForceClean( strFile, detail, fr );
Exit0:

    return hr;
}

//HRESULT CBkScanSingleFileImp::SetScanSetting( BK_SCAN_SETTING& setting )
//{
//    HRESULT hr = S_OK;
//
//    memcpy( &m_setting, &setting, sizeof( BK_SCAN_SETTING ) );
//
//    if ( ( m_setting.nScanPolicy & SCAN_POLICY_CLOUD ) == SCAN_POLICY_CLOUD )
//    {
//        m_bOnline = TRUE; 
//    }
//Exit0:
//    return hr;
//}
//
//HRESULT CBkScanSingleFileImp::GetScanSetting( BK_SCAN_SETTING& setting )
//{
//    HRESULT hr = S_OK;
//    if ( NULL == m_pFileScan )
//    {
//        goto Exit0;
//    }
//
//    hr = m_pFileScan->GetScanSetting( setting );
//Exit0:
//    return hr;
//}