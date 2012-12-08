#include "StdAfx.h"
#include "unknownreport.h"
#include "backupex.h"
#include "setting.h"
#include "skylark2\bkbak.h"
#include "common\runtimelog.h"
#include "bkengex.h"

#ifdef _DEBUG
#define UNKNOWN_REPORT_FIRST_WAIT   300000
#define UNKNOWN_REPORT_NEXT_WAIT    1000
#else
//#define UNKNOWN_REPORT_FIRST_WAIT   1000
#define UNKNOWN_REPORT_FIRST_WAIT   300000
#define UNKNOWN_REPORT_NEXT_WAIT    1000
#endif

NS_SKYLARK_USING

HRESULT CUnkownReport::Initialize()
{
    HRESULT hr  = S_OK;
    CAtlList<CReportFile>   reportList;

    if ( InterlockedCompareExchange( &m_lInit, 1, 0 ) )
    {
        hr = S_FALSE;
        goto Exit0;
    }

    hr = m_reportFileDB.Initialize();
    if (FAILED(hr))
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownReport] failed to create ReportFileDB : 0x%x", hr );
        goto Exit0;
    }

    hr = m_hNotifyStop.Create( NULL, TRUE, FALSE, NULL );
    if ( FAILED(hr) )
    {
        goto Exit0;
    }

    hr = m_hNotifyReport.Create( NULL, TRUE, FALSE, NULL );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    m_submitList.RemoveAll();
    m_reportHighList.RemoveAll();
    m_reportMidList.RemoveAll();
    m_reportLowList.RemoveAll();

    m_reportFileDB.GetUnReportList( reportList );

    POSITION pos = reportList.GetHeadPosition();

    while( pos )
    {
        const CReportFile&  rfile = reportList.GetNext( pos );

        // 将新文件记入数据库
        if ( ( rfile.m_nTrack & FILE_TRACK_QUARANTINE ) )
        {   
            m_reportLowList.AddTail( rfile );
        }
        else
        {
            if ( rfile.m_nFileSize < REPORT_MAX_HIGH_LEVEL_SIZE )
            {
                m_reportHighList.AddTail( rfile );
            }
            else if ( rfile.m_nFileSize < REPORT_MAX_MID_LEVEL_SIZE )
            {
                m_reportMidList.AddTail( rfile );
            }
            else
            {
                m_reportLowList.AddTail( rfile );
            }
        }
    }

    return S_OK;
Exit0:
    m_hNotifyReport.Close();
    m_hNotifyStop.Close();

    InterlockedExchange( &m_lInit, 0 );
    return hr;
}

HRESULT CUnkownReport::Uninitialize()
{
    HRESULT hr  = S_OK;
    if ( !InterlockedCompareExchange( &m_lInit, 0, 1 ) )
    {
        hr = S_FALSE;
        goto Exit0;
    }

    m_submitList.RemoveAll();
    m_reportHighList.RemoveAll();
    m_reportMidList.RemoveAll();
    m_reportLowList.RemoveAll();

    if ( m_hNotifyReport )
    {
        m_hNotifyReport.Close();
    }

    if ( m_hNotifyStop )
    {
        m_hNotifyStop.Set();

        AWinRunnable::WaitExit( INFINITE );

        m_hNotifyStop.Close();
    }

    m_reportFileDB.Uninitialize();

    if ( m_spiBackup )
    {
        m_spiBackup.Release();
    }

    m_spiBakFinder.Release();

    if ( m_spiUploader )
    {
        m_spiUploader->Uninitialize();
        m_spiUploader.Release();
    }
Exit0:
    return hr;
}

BOOL  CUnkownReport::PopHeadSubmitFile( CReportFile& rfile )
{
    CObjGuard   guard( m_submitLock );
    BOOL        bRet = FALSE;;

    if ( m_submitList.GetCount() )
    {
        rfile = m_submitList.RemoveHead();
        bRet = TRUE;
    }

    return bRet;
}

BOOL  CUnkownReport::CheckInReportList( CReportFile& rfile )
{
    CObjGuard   guard( m_reportLock );

    // 顺序查找
    POSITION pos = m_reportHighList.GetHeadPosition();
    for (NULL; pos; m_reportHighList.GetNext(pos))
    {
        const CReportFile& rFindFile = m_reportHighList.GetAt(pos);
        if (0 == rFindFile.m_strFilePath.CompareNoCase(rfile.m_strFilePath))
            return TRUE;
    }

    pos = m_reportMidList.GetHeadPosition();
    for (NULL; pos; m_reportMidList.GetNext(pos))
    {
        const CReportFile& rFindFile = m_reportMidList.GetAt(pos);
        if (0 == rFindFile.m_strFilePath.CompareNoCase(rfile.m_strFilePath))
            return TRUE;
    }

    pos = m_reportLowList.GetHeadPosition();
    for (NULL; pos; m_reportLowList.GetNext(pos))
    {
        const CReportFile& rFindFile = m_reportLowList.GetAt(pos);
        if (0 == rFindFile.m_strFilePath.CompareNoCase(rfile.m_strFilePath))
            return TRUE;
    }


    return FALSE;
}

BOOL  CUnkownReport::GetBackupFileAttributes( 
    CString& strId, 
    Skylark::BKBAK_BACKUP_INFO& info, 
    BSTR*   bstrOrgPath
    )
{
    Skylark::BKBAK_BACKUP_ID    backid;
    BSTR                        bstrPath;

    Skylark::BKENG_INIT( &backid );
    Skylark::BKENG_INIT( &info );
    backid.uBackupID = _wtoi64( strId );

    if ( m_spiBakFinder )
    {
        if ( SUCCEEDED( m_spiBakFinder->GetBackupInfo( &backid, &info, &bstrPath ) ) )
        {
            ::SysFreeString( bstrPath );
            bstrPath = NULL;

            return TRUE;
        }
    }

    return FALSE;
}

BOOL  CUnkownReport::CheckExpired( CReportFile& rfile )
{
    WIN32_FILE_ATTRIBUTE_DATA fdata;

    if ( ( rfile.m_nTrack & FILE_TRACK_QUARANTINE ) )
    {   // 隔离的文件总是不过期
        Skylark::BKBAK_BACKUP_INFO info;

        if ( GetBackupFileAttributes( rfile.m_strFilePath, info ) )
        {
            if( rfile.m_nFileSize == ( int )info.nFileSizeLow )
            {
                if ( ( rfile.m_createTime.dwHighDateTime == info.ftLastWriteTime.dwHighDateTime ) 
                    && ( rfile.m_createTime.dwLowDateTime == info.ftLastWriteTime.dwLowDateTime ) )
                {
                    return FALSE;
                }
            }

            return TRUE;
        }

        return FALSE;
    }
    else if ( GetFileAttributesEx( rfile.m_strFilePath, GetFileExInfoStandard, &fdata ) )
    {
        if ( rfile.m_nFileSize == fdata.nFileSizeLow )
        {
            if ( ( rfile.m_createTime.dwHighDateTime == fdata.ftLastWriteTime.dwHighDateTime ) 
                && ( rfile.m_createTime.dwLowDateTime == fdata.ftLastWriteTime.dwLowDateTime ) )
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

// 这个函数很重要！千万不能删
BOOL  CUnkownReport::CheckInDB( CReportFile& rfile )
{
    BOOL    bRet = FALSE;

    m_reportFileDB.QueryFileInfo( 
        rfile.m_strFilePath, 
        bRet, 
        rfile
        );
    if ( bRet )
    {
        if ( CheckExpired( rfile ) )
        {
            m_reportFileDB.RemoveFileInfo( rfile );
            bRet = FALSE;
        }
    }

    return bRet;
}

void  CUnkownReport::ProcessNewFile()
{
    WIN32_FILE_ATTRIBUTE_DATA   fdata;

    while( 1 )
    {
        CReportFile rfile;
        HANDLE  hFile = INVALID_HANDLE_VALUE;

        if ( PopHeadSubmitFile( rfile ) )
        {
            if ( CheckInReportList( rfile ) )
            {
                continue;
            }

            if ( CheckInDB( rfile ) )
            {
                if ( enumFileReported == rfile.m_nReportState )
                {
                    continue;
                }
            }
        }
        else
        {
            return;
        }

        {
            CObjGuard   guard( m_reportLock );

            // 将新文件记入数据库
            if ( ( rfile.m_nTrack & FILE_TRACK_QUARANTINE ) )
            {   
                Skylark::BKBAK_BACKUP_INFO  info;

                if ( GetBackupFileAttributes( rfile.m_strFilePath, info ) )
                {
                    rfile.SetCreateTime( info.uCRC32 );
                    rfile.m_nFileSize = ( int )info.nFileSizeLow;
                }
                else
                {
                    rfile.SetCreateTime( 0 );
                    rfile.m_nFileSize = 0;
                }
                
                rfile.m_nReportState = enumFileNotReport;
                rfile.m_nRescanState = enumFileRescaned;    // 隔离的文件不需要rescan

                m_reportFileDB.AddFileInfo( rfile );
                m_reportLowList.AddTail( rfile );
            }
            else if ( GetFileAttributesEx( rfile.m_strFilePath, GetFileExInfoStandard, &fdata ) )
            {
                rfile.SetCreateTime( fdata.ftLastWriteTime );
                rfile.m_nFileSize = fdata.nFileSizeLow;
                rfile.m_nReportState = enumFileNotReport;
                rfile.m_nRescanState = enumFileNeedRescan;

                m_reportFileDB.AddFileInfo( rfile );
                if ( rfile.m_nFileSize < REPORT_MAX_HIGH_LEVEL_SIZE )
                {
                    m_reportHighList.AddTail( rfile );
                }
                else if ( rfile.m_nFileSize < REPORT_MAX_MID_LEVEL_SIZE )
                {
                    m_reportMidList.AddTail( rfile );
                }
                else
                {
                    m_reportLowList.AddTail( rfile );
                }
            }
        }
    }
}

BOOL  CUnkownReport::GetReportFile( 
    CReportFile& rfile, 
    CAtlList<CReportFile> **pReportList 
    )
{
    CObjGuard   guard( m_reportLock );

    BOOL    bRet = TRUE;
    if ( m_reportHighList.GetCount() )
    {
        rfile = m_reportHighList.RemoveHead();
        if ( pReportList )
        {
            *pReportList = &m_reportHighList;
        }
    }
    else if ( m_reportMidList.GetCount() )
    {
        rfile = m_reportMidList.RemoveHead();
        if ( pReportList )
        {
            *pReportList = &m_reportMidList;
        }
    }
    else if ( m_reportLowList.GetCount() )
    {
        rfile = m_reportLowList.RemoveHead();
        if ( pReportList )
        {
            *pReportList = &m_reportLowList;
        }
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

DWORD STDMETHODCALLTYPE CUnkownReport::Run()
{
    HRESULT hr = S_OK;
    HANDLE  hEvent[ 2 ] = { NULL, NULL };

    WaitForSingleObject( g_hThread, INFINITE );

    if ( !g_bInitFalg )
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_ERROR, L"[CUnkownReport] Load Engine Failed" );
        goto Exit0;
    }

    hr = CScanEngineProxy::Instance().BKEngCreateEngine( 
        __uuidof(Skylark::IBKEngUploader), 
        ( void** )&m_spiUploader 
        );
    if ( FAILED( hr ) )
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_ERROR, L"[CUnkownReport] Upload interface create failed" );
        goto Exit0;
    }

    hr = m_spiUploader->Initialize();
    if ( FAILED( hr ) )
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_ERROR, L"[CUnkownReport] Upload interface initialize failed" );
        goto Exit0;
    }


    CBkBackupProxy::Instance().BKBackupCreateObject( 
        __uuidof( Skylark::IBKFileBackupFinder ), 
        ( void** )&m_spiBakFinder 
        );

    //if (WAIT_TIMEOUT != ::WaitForSingleObject(m_hNotifyStop, UNKNOWN_REPORT_FIRST_WAIT))
    //    goto Exit0;
    hEvent[ 0 ] = m_hNotifyStop;
    hEvent[ 1 ] = m_hNotifyReport;

    if ( WAIT_OBJECT_0 == ::WaitForMultipleObjects( 2, hEvent, FALSE, UNKNOWN_REPORT_FIRST_WAIT ) )
    {
        goto Exit0;
    }

    //BOOL bAutoReport = FALSE;
    //CSvcSetting::Instance().GetAutoReport( bAutoReport );


    if (!m_spiUploader)
        goto Exit0;


    CRunTimeLog::WriteLog(WINMOD_LLVL_INFO, L"report thread start ok" );


    while( 1 )
    {
        CReportFile rfile;
        CAtlList<CReportFile>   *pReportList = NULL;

        if (WAIT_TIMEOUT != ::WaitForSingleObject(m_hNotifyStop, UNKNOWN_REPORT_NEXT_WAIT))
            goto Exit0;

        if ( !GetReportFile( rfile, &pReportList ) )
        {
            continue;
        }

        hr = S_OK;

        if ( ( rfile.m_nTrack & FILE_TRACK_QUARANTINE ) )
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownReport] report quarantine %s", rfile.m_strFilePath );

            if ( m_spiBackup == ( Skylark::IBKFileBackup* )NULL )
            {
                hr = CBkBackupProxy::Instance().BKBackupCreateObject(
                    __uuidof(Skylark::IBKFileBackup),
                    ( void** )&m_spiBackup);
            }
            if ( SUCCEEDED( hr ) )
            {
                Skylark::BKBAK_BACKUP_ID  backupID;
                Skylark::BKENG_INIT( &backupID );
                backupID.uBackupID = _wtoi64(rfile.m_strFilePath);


                Skylark::BKENG_UPLOAD_PARAM  uploadParam;
                Skylark::BKENG_INIT( &uploadParam );
                hr = m_spiBackup->UploadBackupFile(
                    &backupID,
                    m_spiUploader,
                    &uploadParam,
                    static_cast<Skylark::IBKProgress*>(this));
                if ( SUCCEEDED( hr ) )
                {
                    CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownReport] succeeded to report quarantine %s", rfile.m_strFilePath );

                    // 备份文件不需要重扫
                    // 上传成功,从数据库删除
                    //m_reportFileDB.RemoveFileInfo(rfile);
                    rfile.m_nReportState = enumFileReported;
                    m_reportFileDB.AddFileInfo( rfile );
                }
                else
                {
                    CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownReport] failed to report quarantine %s : 0x%x", rfile.m_strFilePath, hr );

                    // 上传不成功
					if ( AtlHresultFromWin32(ERROR_FILE_NOT_FOUND) == hr )
					{// 隔离区中已不存在
						m_reportFileDB.RemoveFileInfo( rfile );
					}
					else
					{
						// 因为已经入库,所以下次重试
						if ( rfile.m_nRetry < REPORT_MAX_RETRY_TIMES )
						{
							rfile.m_nRetry++;
							pReportList->AddTail( rfile );
						}
						else
						{
							rfile.m_nReportState = enumFileRetried;

							m_reportFileDB.AddFileInfo( rfile );
						}
					}
                }
            }
            else
            {
                CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownReport] failed to create IBKFileBackup : 0x%x", hr );

                // 创建隔离组件不成功
                // 因为已经入库,所以下次重试
            }
        }
        else
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownReport] report file %s", rfile.m_strFilePath );


            BOOL bIsReportFile = (rfile.m_nTrack & FILE_TRACK_REPORT_NONPE);


            Skylark::BKENG_UPLOAD_PARAM  uploadParam;
            Skylark::BKENG_INIT( &uploadParam );
            uploadParam.bUploadNonPEFile = bIsReportFile;
            hr = m_spiUploader->Upload( 
                rfile.m_strFilePath, 
                static_cast<Skylark::IBKProgress*>(this), 
                &uploadParam);
            if ( SUCCEEDED( hr ) )
            {
                CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownReport] succeeded to report file %s", rfile.m_strFilePath );
                WIN32_FILE_ATTRIBUTE_DATA   fdata;

                // 上传成功,更新数据库,用于重扫
                if ( GetFileAttributesEx( rfile.m_strFilePath, GetFileExInfoStandard, &fdata ) )
                {
                    rfile.SetCreateTime( fdata.ftLastWriteTime );
                    GetSystemTimeAsFileTime( &rfile.m_ReportTime );
                    rfile.m_nReportState = enumFileReported;

                    m_reportFileDB.AddFileInfo( rfile );
                }
            }
            else
            {
                CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownReport] failed to report file %s : 0x%x", rfile.m_strFilePath, hr );

                // 上传不成功
                if ( WinMod::CWinPathApi::IsFileExisting( rfile.m_strFilePath ) )
                {
                    // 因为已经入库,所以下次重试
                    if ( rfile.m_nRetry < REPORT_MAX_RETRY_TIMES )
                    {
                        rfile.m_nRetry++;
                        pReportList->AddTail( rfile );
                    }
                    else
                    {
                        rfile.m_nReportState = enumFileRetried;

                        m_reportFileDB.AddFileInfo( rfile );
                    }
                }
                else
                {
                    m_reportFileDB.RemoveFileInfo( rfile );
                }
            }


            // 有隐患
            if (bIsReportFile)
                ::DeleteFile(rfile.m_strFilePath);
        }
    }


Exit0:

    if ( m_spiUploader )
    {
        m_spiUploader->Uninitialize();
        m_spiUploader.Release();
    }


    CRunTimeLog::WriteLog(WINMOD_LLVL_INFO, L"[CUnkownReport] report thread exit" );
    return 0;
}