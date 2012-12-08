#include "StdAfx.h"
#include "common/utility.h"
#include "common/runtimelog.h"
#include "reportfiledb.h"

HRESULT  CReportFileDB::Initialize()
{
    HRESULT hr = S_OK;

    if ( InterlockedCompareExchange( &m_lInit, 1, 0 ) )
    {
        hr = S_FALSE;
        goto Exit0;
    }

    hr = CAppPath::Instance().GetLeidianAppPath( m_modpath.m_strPath );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = CAppPath::Instance().GetLeidianLogPath( m_dbpath.m_strPath );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    m_modpath.Append( BKDB_DLL );
    m_dbpath.Append( REPORT_FILE_DATABASE);

    hr = m_dbmod.LoadLib( m_modpath );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_dbmod.BKDbCreateObject( 
        __uuidof( Skylark::ISQLiteComDatabase3 ), 
        ( void** )&m_piSlDB 
        );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_piSlDB->Open( m_dbpath );
    if ( FAILED( hr ) )
    {
        ::SetFileAttributes( m_dbpath, 0 );
        ::DeleteFile( m_dbpath );
        hr = m_piSlDB->Open( m_dbpath );
    }

    if ( FAILED( hr ) )
    {
        goto Exit1;
    }

    if ( !m_piSlDB->TableExists( REPORT_FILE_TABLENAME ) )
    {
        hr = m_piSlDB->ExecuteUpdate( REPORT_FILE_CREATE_TABLE );
        if ( FAILED( hr ) )
        {
            m_piSlDB->GetErrorMsg();
            m_piSlDB->Close();
            goto Exit1;
        }
    }

    goto Exit0;
Exit1:
    m_piSlDB->Release();
    m_piSlDB = NULL;
Exit0:
    if ( FAILED( hr ) )
    {
        InterlockedExchange( &m_lInit, 0 );
    }
    return hr;
}

HRESULT  CReportFileDB::Uninitialize()
{
    HRESULT hr = S_OK;

    if ( !InterlockedCompareExchange( &m_lInit, 0, 1 ) )
    {
        hr = S_FALSE;
        goto Exit0;
    }

    if ( m_piSlDB )
    {
        m_piSlDB->Close();
        m_piSlDB->Release();
        m_piSlDB = NULL;
    }

    m_dbmod.FreeLib();
Exit0:
    return hr;
}

HRESULT CReportFileDB::QueryFileInfo( 
    const CString& strFile, 
    BOOL& bFind, 
    CReportFile& rfile
    )
{
    HRESULT hr = S_OK;
    CComPtr<Skylark::ISQLiteComStatement3>  piStm;
    CComPtr<Skylark::ISQLiteComResultSet3>  piRtm;

    if ( NULL == m_piSlDB )
    {
        hr = E_NOINTERFACE;
    }

    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_piSlDB->PrepareStatement( REPORT_FILE_QUERY_FILE, &piStm );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = piStm->Bind( 1, strFile );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = piStm->ExecuteQuery( &piRtm );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    if ( piRtm->IsEof() )
    {
        hr = S_FALSE;
        bFind = FALSE;
        goto Exit0;
    }

    rfile.m_strFilePath = piRtm->GetAsString( 0 );
    rfile.m_nFileSize = piRtm->GetInt( 1 );
    rfile.m_nReportState = piRtm->GetInt( 2 );
    rfile.m_nTrack = piRtm->GetInt( 3 );
    rfile.SetCreateTime( piRtm->GetInt64( 4 ) );
    rfile.SetReportTime( piRtm->GetInt64( 5 ) );
    rfile.m_nRescanState = piRtm->GetInt( 6 );

    bFind = TRUE;


Exit0:
    return hr;
}

HRESULT CReportFileDB::AddFileInfo( CReportFile& rfile )
{
    HRESULT hr = S_OK;
    CComPtr<Skylark::ISQLiteComStatement3>  piStm;

    if ( NULL == m_piSlDB )
    {
        hr = E_NOINTERFACE;
        goto Exit0;
    }

    hr = m_piSlDB->PrepareStatement( REPORT_FILE_ADD_REPORTFILE, &piStm );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    piStm->Bind( 1, rfile.m_strFilePath );
    piStm->Bind( 2, rfile.m_nFileSize );
    piStm->Bind( 3, rfile.m_nReportState );
    piStm->Bind( 4, rfile.m_nTrack);
    piStm->Bind( 5, rfile.CreateTime2ULL() );
    piStm->Bind( 6, rfile.CommitTime2ULL() );
    piStm->Bind( 7, rfile.ReportTime2ULL() );
    piStm->Bind( 8, rfile.m_nRescanState );

    hr = piStm->ExecuteUpdate();
Exit0:
    return hr;
}

HRESULT CReportFileDB::RemoveFileInfo( CReportFile& rfile )
{
    HRESULT hr = S_OK;
    CComPtr<Skylark::ISQLiteComStatement3>  piStm;

    if ( NULL == m_piSlDB )
    {
        hr = E_NOINTERFACE;
        goto Exit0;
    }

    hr = m_piSlDB->PrepareStatement( REPORT_FILE_DEL_REPORTFILE, &piStm );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    piStm->Bind( 1, rfile.m_strFilePath );

    hr = piStm->ExecuteUpdate();
Exit0:
    return hr;
}

HRESULT CReportFileDB::UpdateRescanInfo( CReportFile& rfile )
{
    HRESULT hr = S_OK;
    CComPtr<Skylark::ISQLiteComStatement3>  piStm;

    if ( NULL == m_piSlDB )
    {
        hr = E_NOINTERFACE;
        goto Exit0;
    }

    hr = m_piSlDB->PrepareStatement( REPORT_FILE_UPLOAD_RESCAN, &piStm );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    piStm->Bind( 1, rfile.m_nRescanState );
    piStm->Bind( 2, rfile.m_strFilePath );

    hr = piStm->ExecuteUpdate();
Exit0:
    return hr;
}

HRESULT CReportFileDB::GetUnReportList( CAtlList<CReportFile>& rfArray )
{
    HRESULT hr = S_OK;
    CComPtr<Skylark::ISQLiteComResultSet3>  piRtm;
    CReportFile rfile;

    if ( NULL == m_piSlDB )
    {
        hr = E_NOINTERFACE;
        goto Exit0;
    }

    hr = m_piSlDB->ExecuteQuery( REPORT_FILE_LOAD_REPORTLIST, &piRtm );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    rfArray.RemoveAll();

    while( !piRtm->IsEof() )
    {
        rfile.m_strFilePath = piRtm->GetAsString( 0 );
        rfile.m_nFileSize = piRtm->GetInt( 1 );
        rfile.m_nReportState = piRtm->GetInt( 2 );
        rfile.m_nTrack = piRtm->GetInt( 3 );
        rfile.SetCreateTime( piRtm->GetInt64( 4 ) );
        rfile.SetCommitTime( piRtm->GetInt64( 5 ) );
        rfile.SetReportTime( piRtm->GetInt64( 6 ) );
        rfile.m_nRescanState = piRtm->GetInt( 7 );

        rfArray.AddTail( rfile );

        piRtm->NextRow();
    }
Exit0:
    return hr;
}

HRESULT CReportFileDB::GetUnRescanList( CAtlArray<CReportFile>& rfArray )
{
    HRESULT hr = S_OK;
    CComPtr<Skylark::ISQLiteComResultSet3>  piRtm;
    CReportFile rfile;

    if ( NULL == m_piSlDB )
    {
        hr = E_NOINTERFACE;
        goto Exit0;
    }

    hr = m_piSlDB->ExecuteQuery( REPORT_FILE_LOAD_RESCANLIST, &piRtm );
    if ( FAILED( hr ) )
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[GetUnRescanList] query rfl.db failed %08x", hr );
        goto Exit0;
    }

    rfArray.RemoveAll();

    while( !piRtm->IsEof() )
    {
        rfile.m_strFilePath = piRtm->GetAsString( 0 );
        rfile.m_nFileSize = piRtm->GetInt( 1 );
        rfile.m_nReportState = piRtm->GetInt( 2 );
        rfile.m_nTrack = piRtm->GetInt( 3 );
        rfile.SetCreateTime( piRtm->GetInt64( 4 ) );
        rfile.SetCommitTime( piRtm->GetInt64( 5 ) );
        rfile.SetReportTime( piRtm->GetInt64( 6 ) );
        rfile.m_nRescanState = piRtm->GetInt( 7 );

        if ( FILE_TRACK_FILE & rfile.m_nTrack)
        {
            rfArray.Add( rfile );
        }

        piRtm->NextRow();
    }
Exit0:
    return hr;
}