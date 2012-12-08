#pragma once
#include "winmod/winpath.h"
#include "skylark2/bkdb_mod.h"

#define BKDB_DLL                    L"ksafedb.dll"
#define REPORT_FILE_DATABASE        L"rfl2.dat"
#define REPORT_FILE_TABLENAME       L"rfl"
#define REPORT_FILE_CREATE_TABLE    L"create table rfl ( file_path varchar not null, file_size integer not null, report_state integer not null, file_track integer not null, create_time integer not null, commit_time integer not null, report_time integer not null, rescan_state integer not null, primary key ( file_path ) )"
#define REPORT_FILE_QUERY_FILE      L"select * from rfl where file_path=?"
#define REPORT_FILE_LOAD_REPORTLIST L"select * from rfl where report_state<>1"
#define REPORT_FILE_ADD_REPORTFILE  L"insert or replace into rfl ( file_path, file_size, report_state, file_track, create_time, commit_time, report_time, rescan_state ) values(?,?,?,?,?,?,?,?)"
#define REPORT_FILE_DEL_REPORTFILE  L"delete from rfl where file_path=?"
#define REPORT_FILE_LOAD_RESCANLIST L"select * from rfl where file_track<>8 and report_state=1 and rescan_state<>100"
#define REPORT_FILE_UPLOAD_RESCAN   L"update rfl set rescan_state=? where file_path=?"

typedef enum
{
    enumFileNotReport = 0,
    enumFileReported  = 1,
    enumFileRetried   = 2,
};

#define FILE_TRACK_MEMORY               0x0001
#define FILE_TRACK_AUTORUN              0x0002
#define FILE_TRACK_FILE                 0x0004      ///< 上传文件
#define FILE_TRACK_QUARANTINE           0x0008      ///< 上传被隔离的文件
#define FILE_TRACK_REPORT_NONPE         0x0010      ///< 上传报告,上传完成后会删除报告

typedef enum
{
    enumFileNeedRescan  = 0,
    enumFileRescaned    = 100,
};

class   CReportFile
{
public:
    CReportFile()
    {
        m_nFileSize = 0;
        m_nReportState = enumFileNotReport;
        m_nTrack = 0;
        memset( &m_createTime, 0, sizeof( FILETIME ) );
        memset( &m_ReportTime, 0, sizeof( FILETIME ) );
        m_nRescanState = enumFileNeedRescan;
        m_ullRescanInterval = 0;
        m_nRetry = 0;
    }
public:
    CString     m_strFilePath;
    int         m_nFileSize;
    int         m_nReportState;
    int         m_nTrack;
    FILETIME    m_createTime;
    FILETIME    m_commitTime;
    FILETIME    m_ReportTime;
    int         m_nRescanState;
    ULONGLONG   m_ullRescanInterval;
    int         m_nRetry;
public:
    HRESULT SetCreateTime( ULONGLONG ullTime )
    {
        return SetTime( m_createTime, ullTime );
    }

    HRESULT SetCreateTime( const FILETIME& ft )
    {
        m_createTime.dwHighDateTime = ft.dwHighDateTime;
        m_createTime.dwLowDateTime  = ft.dwLowDateTime;
        return S_OK;
    }

    HRESULT SetReportTime( ULONGLONG ullTime )
    {
        return SetTime( m_ReportTime, ullTime );
    }

    HRESULT SetReportTime( const FILETIME& ft )
    {
        m_ReportTime.dwHighDateTime = ft.dwHighDateTime;
        m_ReportTime.dwLowDateTime  = ft.dwLowDateTime;
        return S_OK;
    }

    HRESULT SetCommitTime( ULONGLONG ullTime )
    {
        return SetTime( m_commitTime, ullTime );
    }

    HRESULT SetCommitTime( const FILETIME& ft )
    {
        m_commitTime.dwHighDateTime = ft.dwHighDateTime;
        m_commitTime.dwLowDateTime  = ft.dwLowDateTime;
        return S_OK;
    }

    ULONGLONG CreateTime2ULL()
    {
        return Time2ULL( m_createTime );
    }

    ULONGLONG ReportTime2ULL()
    {
        return Time2ULL( m_ReportTime );
    }

    ULONGLONG CommitTime2ULL()
    {
        return Time2ULL( m_commitTime );
    }
private:
    HRESULT SetTime( FILETIME& ft, ULONGLONG ullTime )
    {
        HRESULT         hr = S_OK;
        ULARGE_INTEGER  ut;

        if ( 0L == ullTime )
        {
            return S_OK;
        }

        ut.QuadPart = ullTime;

        ft.dwLowDateTime = ut.LowPart;
        ft.dwHighDateTime = ut.HighPart;

        return hr;
    }

    ULONGLONG Time2ULL( const FILETIME& ft )
    {
        ULARGE_INTEGER  ut;

        ut.LowPart = ft.dwLowDateTime;
        ut.HighPart = ft.dwHighDateTime;

        return ut.QuadPart;
    }
};

class CReportFileDB
{
public:
    CReportFileDB(void) : 
      m_lInit( 0 ),
      m_piSlDB( NULL )
    {

    }
    ~CReportFileDB(void)
    {
        Uninitialize();
    }

public:
    HRESULT  Initialize();

    HRESULT  Uninitialize();

    HRESULT QueryFileInfo( 
        const CString& strFile, 
        BOOL& bFind, 
        CReportFile& rfile
        );

    HRESULT AddFileInfo( CReportFile& rfile );

    HRESULT RemoveFileInfo( CReportFile& rfile );

    HRESULT UpdateRescanInfo( CReportFile& rfile );

    HRESULT GetUnReportList( CAtlList<CReportFile>& rfArray );

    HRESULT GetUnRescanList( CAtlArray<CReportFile>& rfArray );
public:
private:
    LONG                m_lInit;
    WinMod::CWinPath    m_modpath;
    WinMod::CWinPath    m_dbpath;
    Skylark::CWinModule_bkdb        m_dbmod;
    Skylark::ISQLiteComDatabase3*   m_piSlDB;
};
