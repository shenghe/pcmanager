#pragma once
#include "winmod/winrunnable.h"
#include "common/runtimelog.h"
#include "common/rescanstat.h"
#include "reportfiledb.h"
#include "scanwork.h"

#define MAX_RESULT_LIST_NUM 1000

class CUnkownRescan
    :  public WinMod::AWinRunnable
{
public:
    CUnkownRescan(void)
        : m_hNotifyStop( NULL ),
        m_bStartReScan( TRUE ),
        m_piScan( NULL ),
        m_bSettingChange( FALSE )
    {
        m_setting.Reset();
        m_setting.nScanPolicy = SCAN_POLICY_CLOUD;
    }

    ~CUnkownRescan(void)
    {
    }

    static CUnkownRescan& Instance()
    {
        static CUnkownRescan    _ins;

        return _ins;
    }

    HRESULT Initialize();
    HRESULT Uninitialize();

    void SettingChange()
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] notify setting change" );
        m_bSettingChange = TRUE;
    }

    void NotifyStop()
    {
        if ( m_hNotifyStop )
        {
            if ( m_piScan )
            {
                m_piScan->Stop();
            }

            m_hNotifyStop.Set();
        }
    }

    void GetRescanResult( CAtlArray<BK_FILE_RESULT>& result );
private:

    virtual DWORD STDMETHODCALLTYPE Run();


    void  LoadRescanList();

    BOOL  IsNeedRescan( CReportFile& rfile );

    ULONGLONG GetRescanInterval(int nRescanState);
    BOOL  GetRescanInterval( const CReportFile& rfile, CFileTimeSpan& interval );

    void  UpdateDBState( CReportFile& rfile, const BK_FILE_RESULT& fr ); 
private:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

    CObjLock    m_Lock;
    CObjLock    m_rLock;
    CAtlArray<CReportFile>      m_rescanLst;
    CAtlList<BK_FILE_RESULT>    m_resultLst;
    ATL::CEvent         m_hNotifyStop;
    LONG                m_lInit;
    CBkMultiFileScan*   m_piScan;
    CReportFileDB       m_reportFileDB;
    CRescanDB           m_rescanDB;
private:
    BOOL                m_bStartReScan;
    BOOL                m_bSettingChange;
    BK_SCAN_SETTING     m_setting;
};
