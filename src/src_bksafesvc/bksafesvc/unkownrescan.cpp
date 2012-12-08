#include "stdafx.h"
#include "unknownreport.h"
#include "unkownrescan.h"
#include "setting.h"
#include "common/runtimelog.h"
#include "scom\scom\scomerror.h"
#include "ksbwlogger.h"
#include "kxescan/logic2/KLogic.h"

#ifdef _DEBUG
#define UNKNOWN_RESCAN_WAIT   10000
#else
#define UNKNOWN_RESCAN_WAIT   30000
#endif


#define RESCAN_DELAY_BOOT (5*60*1000)
#define RESCAN_DELAY_RUN  (10*1000)

HRESULT CUnkownRescan::Initialize()
{
    HRESULT hr  = S_OK;

    if ( InterlockedCompareExchange( &m_lInit, 1, 0 ) )
    {
        hr = S_FALSE;
        goto Exit0;
    }

    hr = m_hNotifyStop.Create( NULL, TRUE, FALSE, NULL );
    if ( FAILED(hr) )
    {
        goto Exit0;
    }

    m_rescanLst.RemoveAll();

    m_bStartReScan = TRUE;

    hr = m_reportFileDB.Initialize();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_rescanDB.Initialize();
    if ( FAILED( hr ) )
    {
        goto Exit1;
    }

    m_setting.Reset();
    CSvcSetting::Instance().LoadScanSetting( m_setting );
    goto Exit0;
Exit1:
    m_reportFileDB.Uninitialize();
Exit0:
    if ( FAILED( hr ) )
    {
        m_hNotifyStop.Close();

        InterlockedExchange( &m_lInit, 0 );
    }
    return hr;
}

HRESULT CUnkownRescan::Uninitialize()
{
    HRESULT hr  = S_OK;
    if ( !InterlockedCompareExchange( &m_lInit, 0, 1 ) )
    {
        hr = S_FALSE;
        goto Exit0;
    }

    NotifyStop();

    AWinRunnable::WaitExit(INFINITE);

    if ( m_piScan )
    {
        m_piScan->Uninitialize();
        delete m_piScan;
        m_piScan = NULL;
    }

    m_rescanDB.Uninitialize();
    m_reportFileDB.Uninitialize();
    m_rescanLst.RemoveAll();

    m_hNotifyStop.Close();

Exit0:
    return hr;
}

// 每隔1分钟查一次，持续10次以后，然后变成10分钟一次一直到1小时。
ULONGLONG CUnkownRescan::GetRescanInterval(int nRescanState)
{
    ULONGLONG ulInterval = 0;
    switch( nRescanState )
    {
    case 0:
    case 1:
    case 2:     
    case 3:     
    case 4:     
    case 5:     
    case 6:     
    case 7:     
    case 8:     
    case 9:     
	case 10:
		// T(n) = 30S + 1Min * n *(n+1) / 2
		ulInterval = (30 + nRescanState * (nRescanState + 1) * 30) * CFileTime::Second;
		break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
		// T(n) = T(10) + 10Min * (n - 10) * (n - 9) / 2
		ulInterval = GetRescanInterval(10) + (nRescanState - 10) * (nRescanState - 9) * 300 * CFileTime::Second;
		break;
    default:
        {
            if ( nRescanState < enumFileRescaned )
            {
				// T(n) = T(15) + (n - 15) * 1Hour
				ulInterval = GetRescanInterval(15) + (nRescanState - 15) * CFileTime::Hour;
                break;
            }
            else
            {
                return ULLONG_MAX;
            }
        }
    }

    return ulInterval;
}

BOOL    CUnkownRescan::GetRescanInterval( 
    const CReportFile& rfile, 
    CFileTimeSpan& interval
    )
{
    ULONGLONG   ulInterval = GetRescanInterval(rfile.m_nRescanState);
    if (ULLONG_MAX == ulInterval)
        return FALSE;

    interval.SetTimeSpan( ( LONGLONG )ulInterval );
    return TRUE;
}

BOOL    CUnkownRescan::IsNeedRescan( CReportFile& rfile )
{
    CFileTime   org;
    CFileTimeSpan   interval, timediff;

    if ( rfile.m_nRescanState >= enumFileRescaned )
    {
        CRunTimeLog::WriteLog(
            WINMOD_LLVL_TRACE,
            L"[CUnkownRescan] already rescaned file %s",
            rfile.m_strFilePath);
        return FALSE;
    }


    if ( !GetRescanInterval( rfile, interval ) )
    {
        return FALSE;
    }

    org = rfile.m_ReportTime;

    timediff = CFileTime::GetCurrentTime() - org;

    if ( timediff < interval )
    {
        CRunTimeLog::WriteLog(
            WINMOD_LLVL_TRACE,
            L"[CUnkownRescan] it is not time (%lu unit left) to rescan file %s",
            DWORD(interval.GetTimeSpan() - timediff.GetTimeSpan()),
            rfile.m_strFilePath);
        return FALSE;
    }

    return TRUE;
}

void    CUnkownRescan::LoadRescanList()
{
    HRESULT hr = S_OK;
    CAtlArray<CReportFile>  tmp;

    hr = m_reportFileDB.GetUnRescanList( tmp );
    if ( SUCCEEDED( hr ) )
    {
        for ( size_t i = 0; i < tmp.GetCount(); i++ )
        {
            if ( IsNeedRescan( tmp[ i ] ) )
            {
                CObjGuard   guard( m_Lock );

                CRunTimeLog::WriteLog(
                    WINMOD_LLVL_TRACE,
                    L"[CUnkownRescan] ready to rescan file %s",
                    tmp[ i ].m_strFilePath);
                m_rescanLst.Add( tmp[ i ] );
            }
        }
    }
}

void    CUnkownRescan::UpdateDBState( CReportFile& rfile, const BK_FILE_RESULT& fr )
{
    if ( BkQueryInfoFileTypeUnknown == fr.Type
		|| BkQueryInfoFileTypeNull == fr.Type )
    {
        return;
    }

    if ( SLPTL_LEVEL_MARK___NEVER_BEEN_SCANNED == fr.SecLevel )
    {
        if ( rfile.m_nRescanState > 99 )
        {
            m_reportFileDB.RemoveFileInfo( rfile );

            return;
        }
    }

    m_reportFileDB.UpdateRescanInfo( rfile );

    if ( SLPTL_LEVEL_MARK___NEVER_BEEN_SCANNED != fr.SecLevel )
    {
        m_rescanDB.AddRescanResult( fr, rfile.CommitTime2ULL() );
    }
}

DWORD STDMETHODCALLTYPE CUnkownRescan::Run()
{
    HRESULT hr = S_OK;
    DWORD dwWaitTime = GetTickCount();

    if (dwWaitTime >= RESCAN_DELAY_BOOT)
    {
        dwWaitTime = RESCAN_DELAY_RUN; // 服务启动后30秒钟再开始回扫逻辑
    }
    else
    {
        dwWaitTime = (RESCAN_DELAY_BOOT - dwWaitTime); // 开机5分后再开始回扫逻辑
    }

    DWORD   dwRet = WaitForSingleObject( m_hNotifyStop, dwWaitTime );
    if ( WAIT_TIMEOUT != dwRet )
    {
        return 0;
    }

    m_piScan = new CBkMultiFileScan();
    if ( NULL == m_piScan )
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_ERROR, L"[CUnkownRescan] new scan instance failed" );
        return 0;
    }

    hr = m_piScan->Initialize();
    if ( FAILED( hr ) )
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_ERROR, L"[CUnkownRescan] scan instance init failed" );
        delete  m_piScan;
        m_piScan = NULL;
        return 0;
    }

    CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownRescan] scan instance init ok" );

    CKsbwLogger::Instance().Init();

    // 回扫禁止查询联网缓存(by bbcallen 2010.09.01)
    m_setting.nScanPolicy |= SCAN_PLOICY_FOR_RESCAN;
    m_piScan->SetScanSetting( m_setting );

    KLogic logic;
    CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CUnkownRescan] rescan thread start ok" );
    while( 1 )
    {
        HRESULT hr = S_OK;
        CAtlArray<CReportFile>  tmp;
        CAtlArray<CString>  scanlist;
        CAtlArray<BK_FILE_RESULT>   scanResult;

        CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan thread wait 30 seconds" );
        dwRet = WaitForSingleObject( m_hNotifyStop, UNKNOWN_RESCAN_WAIT );   // wait 30 seconds
        //dwRet = WaitForSingleObject( m_hNotifyStop, 1000 );   // wait 1 sec
        if ( WAIT_TIMEOUT != dwRet )
        {
            break;
        }
        else
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] load rescan list" );
            LoadRescanList();

            if ( m_bSettingChange )
            {
                CSvcSetting::Instance().LoadScanSetting( m_setting );
                m_piScan->SetScanSetting( m_setting );
                m_bSettingChange = FALSE;
            }

            {
                CObjGuard   guard( m_Lock );

                tmp.Copy( m_rescanLst );
                m_rescanLst.RemoveAll();
            }

            if ( tmp.GetCount() )
            {
                for ( size_t i = 0; i < tmp.GetCount(); i++ )
                {
                    scanlist.Add( tmp[i].m_strFilePath );
                }

                CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan task begin" );
                hr = m_piScan->Scan(  scanlist, scanResult );
                if ( SUCCEEDED( hr ) )
                {
                    CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan task finished" );
                    for ( size_t i = 0; i < scanResult.GetCount(); i++ )
                    {
                        CReportFile&    rReportFile = tmp[ i ];
                        BK_FILE_RESULT& rFileResult = scanResult[ i ];
                        BOOL bResultTypeRight = TRUE;

                        switch( rFileResult.Type )
                        {
                        case BkQueryInfoFileTypeSafe:
                            {
                                if ( SLPTL_LEVEL_MARK___NEVER_BEEN_SCANNED == rFileResult.SecLevel ||
									SLPTL_LEVEL_MARK___UNRECOGNISED_FILE == rFileResult.SecLevel)
                                {
									rFileResult.SecLevel = SLPTL_LEVEL_MARK___NEVER_BEEN_SCANNED;
                                    CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan %s still unkown", rReportFile.m_strFilePath );
                                    rReportFile.m_nRescanState++;
                                }
                                else
                                {
                                    ULONGLONG ulInterval = GetRescanInterval(rReportFile.m_nRescanState++);
                                    if (ulInterval != ULLONG_MAX)
                                    {
                                        //CKsbwLogger::Instance().SendRescanDelay(scanResult[i].MD5, scanResult[i].SecLevel);
                                    }

                                    
                                    // 对回扫结果为白的文件, 检查是否需要恢复启动项
                                    if ( Skylark::SLLevelIsTrusted(rFileResult.SecLevel)
                                        && logic.SpTestAndStart()
                                        )
                                    {
                                        std::vector<S_HISTORY_ITEM_EX> HisList;
                                        logic.QueryHistoryListEx( INT_MAX, HisList );

                                        std::vector<S_RECOVER_ITEM> recoverList;

                                        // 遍历所有还原项
                                        for ( size_t i = 0; i < HisList.size(); i++ )
                                        {
                                            S_HISTORY_ITEM_EX& rHistoryItemEx = HisList[i];

                                            S_RECOVER_ITEM recoverItem;
                                            recoverItem.tmHistoryTime = rHistoryItemEx.tmHistoryTime;

                                            // 遍历还原项中所有的item
                                            for (size_t k = 0; k < rHistoryItemEx.FixedList.size(); ++k)
                                            {
                                                S_HISTORY_ITEM_INFO& rItemInfo = rHistoryItemEx.FixedList[k];

                                                // 进处理禁止启动的
                                                if (enum_PROCESS_TYPE_DISABLE != rItemInfo.ActionType)
                                                    continue;

                                                // 仅处理可修复的
                                                if (enum_HISTORY_ITEM_STATE_FIXED != rItemInfo.state)
                                                    continue;

                                                // 在item中查找文件路径是否匹配
                                                BOOL bMatchItem = FALSE;
                                                for (size_t m = 0; m < rItemInfo.vecProcessItems.size(); ++m)
                                                {
                                                    S_PROCESS_OBJECT_INFO& rProcObjInfo = rItemInfo.vecProcessItems[m];

                                                    // 仅对比just_record的文件
                                                    if (enum_PROCESS_AT_BOOT_DELETE_FILE == rProcObjInfo.processType)
                                                    {   // 如果包含标记为删除的文件,则跳过不修复此项
                                                        bMatchItem = FALSE;
                                                        break;
                                                    }
                                                    else if (enum_PROCESS_AT_JUST_RECORED == rProcObjInfo.processType)
                                                    {
                                                        WinMod::CWinPath PathFile = rProcObjInfo.strInfo_1.c_str();
                                                        PathFile.ExpandNormalizedPathName();
                                                        PathFile.RemoveUnicodePrefix();
                                                        if (PathFile.m_strPath.IsEmpty() ||
                                                            0 != PathFile.m_strPath.CompareNoCase(rFileResult.FileName))
                                                        {   // 如果路径无效,或者文件名不匹配,则跳过不修复此项
                                                            bMatchItem = FALSE;
                                                            break;
                                                        }

                                                        // 记录日志
                                                        CRunTimeLog::WriteLog(
                                                            WINMOD_LLVL_INFO,
                                                            L"[rescan] match %s",
                                                            rProcObjInfo.strInfo_1.c_str());

                                                        // 标记为匹配
                                                        bMatchItem = TRUE;
                                                    }
                                                }

                                                
                                                // 查找到匹配的项,则记录项目id
                                                if (bMatchItem)
                                                {
                                                    CRunTimeLog::WriteLog(
                                                        WINMOD_LLVL_INFO,
                                                        L"[rescan] decide to recover %s",
                                                        rItemInfo.strItemName.c_str());

                                                    recoverItem.vecItemIDs.push_back(rItemInfo.dwID);
                                                }
                                            }


                                            // 将当前项添加到还原列表
                                            if (!recoverItem.vecItemIDs.empty())
                                            {
                                                CRunTimeLog::WriteLog(
                                                    WINMOD_LLVL_INFO,
                                                    L"[rescan] decide to recover ids above",
                                                    recoverItem.vecItemIDs);

                                                recoverList.push_back(recoverItem);
                                                break;
                                            }
                                        }


                                        // 如果还原列表不为空,则还原之
                                        if (!recoverList.empty())
                                        {
                                            CRunTimeLog::WriteLog(
                                                WINMOD_LLVL_INFO,
                                                L"[rescan] start recover");

                                            logic.StartRecoverEx(recoverList);
                                        }
                                    }



                                    if ( rFileResult.bHashed )
                                    {
                                        CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan %s is safe", rReportFile.m_strFilePath );
                                        rReportFile.m_nRescanState = enumFileRescaned;
                                    }
                                    else
                                    {
                                        if ( WinMod::CWinPathApi::IsFileExisting( rReportFile.m_strFilePath ) )
                                        {
                                            if ( ( rFileResult.SecLevel >= SLTPL_LEVEL_MARK___LOCAL_NON_EXE_FILE ) && ( rFileResult.SecLevel <= SLTPL_LEVEL_MARK___LOCAL_NON_EXE_FILE ) )
                                            {
                                                CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan %s is invalid pe level %d", rReportFile.m_strFilePath, rFileResult.SecLevel );
                                                rReportFile.m_nRescanState = enumFileRescaned;
                                            }
                                            else
                                            {
                                                CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan %s is not hashed, retried", rReportFile.m_strFilePath );
                                                rReportFile.m_nRescanState++;
                                                rFileResult.SecLevel = SLPTL_LEVEL_MARK___NEVER_BEEN_SCANNED;
                                            }
                                        }
                                        else
                                        {
                                            CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan %s is not exists", rReportFile.m_strFilePath );
                                            rReportFile.m_nRescanState = enumFileRescaned;
                                            rFileResult.SecLevel = SLPTL_LEVEL_MARK___LOCAL_FAILED_TO_HASH_FILE;
                                        }
                                    }
                                }
                                break;
                            }
                        case BkQueryInfoFileTypeVirus:
                            {
                                CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] rescan %s is virus, name %s", rReportFile.m_strFilePath, rFileResult.VirusName );
                                rReportFile.m_nRescanState = enumFileRescaned;
                                {
                                    CObjGuard   guard( m_rLock );

                                    if ( m_resultLst.GetCount() >= MAX_RESULT_LIST_NUM )
                                    {
                                        m_resultLst.RemoveHead();
                                    }

                                    m_resultLst.AddTail( rFileResult );
                                }
                                break;
                            }
                        case BkQueryInfoFileTypeUnknown:
                            {
                                CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] upload failed or file was modified", rReportFile.m_strFilePath );

                                if ( rReportFile.m_nRescanState > 5 )
                                {
                                    m_reportFileDB.RemoveFileInfo( rReportFile );
                                    CUnkownReport::Instance().AddUnkownFile( rReportFile.m_strFilePath, rReportFile.m_nTrack );
                                }
                                else
                                {
                                    rReportFile.m_nRescanState++;
                                }
                                break;
                            }
						case BkQueryInfoFileTypeNull:
							{
								CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE, L"[CUnkownRescan] unknown reason return no scan result", rReportFile.m_strFilePath );
								if ( rReportFile.m_nRescanState > 5 )
								{
									m_reportFileDB.RemoveFileInfo( rReportFile );
									CUnkownReport::Instance().AddUnkownFile( rReportFile.m_strFilePath, rReportFile.m_nTrack );
								}
								else
								{
									rReportFile.m_nRescanState++;
								}
								break;
							}
                        default:
                            {
                                bResultTypeRight = FALSE;
                                break;
                            }
                        }

                        if ((rReportFile.m_nTrack & FILE_TRACK_MEMORY)     ||    
                            (rReportFile.m_nTrack & FILE_TRACK_AUTORUN)    ||
                            (rReportFile.m_nTrack & FILE_TRACK_FILE)       ||     
                            (rReportFile.m_nTrack & FILE_TRACK_QUARANTINE))
                        {
                            if (0 != rReportFile.ReportTime2ULL() && enumFileRescaned == rReportFile.m_nRescanState)
                            {
                                CFileTime ftCurrent = CFileTime::GetCurrentTime();
                                CFileTimeSpan uDiff = ftCurrent - rReportFile.m_ReportTime;

                                if (uDiff.GetTimeSpan() < 0)
                                    uDiff = 0;

                                DWORD dwDelay = uDiff.GetTimeSpan() / CFileTime::Millisecond;

                                CKsbwLogger::Instance().SendRescanDelay(
                                    rFileResult.MD5,
                                    rFileResult.SecLevel,
                                    dwDelay);
                            }
                        }

                        if(bResultTypeRight)
                        {
                            UpdateDBState( rReportFile, rFileResult );
                        }
                        
                    }
                }
            }
        }
    }

    CKsbwLogger::Instance().Uninit();
    CRunTimeLog::WriteLog(WINMOD_LLVL_INFO, L"[CUnkownRescan] rescan thread exit" );
    if ( m_piScan )
    {
        m_piScan->Uninitialize();
        delete m_piScan;
        m_piScan = NULL;
    }
    return 0;
}

void    CUnkownRescan::GetRescanResult( CAtlArray<BK_FILE_RESULT>& result )
{
    CObjGuard   guard( m_rLock );
    CRunTimeLog::WriteLog(WINMOD_LLVL_TRACE,L"[CUnkownRescan] tray GetRescanResult");

    while ( m_resultLst.GetCount() )
    {
        const BK_FILE_RESULT& headResult = m_resultLst.GetHead();
        CRunTimeLog::WriteLog(
            WINMOD_LLVL_TRACE,
            L"[CUnkownRescan] pop result t:%lu, s:%lu, f:%s",
            headResult.Type,
            headResult.Status,
            headResult.FileName);

        result.Add( m_resultLst.RemoveHead() );
    }
}
