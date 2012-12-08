#include "StdAfx.h"
#include "bkscanmultifileimp.h"
#include "miniutil/bksvcmdl.h"
#include "setting.h"
#include "unknownreport.h"

CBkScanMultiFileImp::CBkScanMultiFileImp(void)
: m_pFileScan( NULL )
{
}

CBkScanMultiFileImp::~CBkScanMultiFileImp(void)
{
}

HRESULT CBkScanMultiFileImp::Initialize( DWORD dwProcId, int nVersion )
{
    HRESULT hr = S_OK;

    if ( BKCOM_EXPORT_MODULE_CBkScanSingleFile_VER != nVersion )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    BKSVC_DEBUG_TRACE( L"[CBkScanMultiFileImp] Initialize\n" );
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
        m_pFileScan = new CBkMultiFileScan();
        if ( NULL == m_pFileScan )
        {
            hr = E_OUTOFMEMORY;
            goto Exit0;
        }

        hr = m_pFileScan->Initialize();
        if ( FAILED( hr ) )
        {
            goto Exit0;
        }
    }
Exit0:
    return hr;
}

HRESULT CBkScanMultiFileImp::Uninitialize()
{
    HRESULT hr = S_OK;

    if ( m_pFileScan )
    {
        m_pFileScan->Uninitialize();
        delete  m_pFileScan;
        m_pFileScan = NULL;
    }

    BKSVC_DEBUG_TRACE( L"[CBkScanMultiFileImp] Uninitialize\n" );
    hr = __super::Uninitialize();

    return hr;
}

HRESULT CBkScanMultiFileImp::ScanHash( CAtlArray<CString>& hashArray, DWORD dwScanPolicy, CAtlArray<BK_FILE_RESULT>& frArray )
{
    HRESULT hr = S_OK;

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

    hr = m_pFileScan->ScanHash( hashArray, frArray );
Exit0:
    return hr;
}

HRESULT CBkScanMultiFileImp::ScanFile( CAtlArray<CString>& fileArray, DWORD dwScanPolicy, CAtlArray<BK_FILE_RESULT>& frArray )
{
    HRESULT hr = S_OK;
    size_t  nSize = 0;
    BOOL    bAutoReport = FALSE;
    CAtlArray<size_t>   unKnownIndex;

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

    hr = m_pFileScan->Scan2( fileArray, frArray, unKnownIndex );

    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = CSvcSetting::Instance().GetAutoReport( bAutoReport );
	if ( FAILED( hr ) )
	{
		bAutoReport = FALSE;
	}

    if ( bAutoReport )
    {
        nSize = unKnownIndex.GetCount();
        for ( size_t i = 0; i < nSize; i++ )
        {
            CUnkownReport::Instance().AddUnkownFile( 
                frArray[ unKnownIndex[ i ] ].FileName, 
                FILE_TRACK_FILE 
                );
        }
    }
Exit0:
    return hr;
}