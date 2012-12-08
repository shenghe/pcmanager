#include "stdafx.h"
#include "common.h"
#include "scanwork.h"
#include "scanpolicy.h"
#include "autorunex.h"
#include "bklogex.h"
#include "log.h"
#include "globalwhitelist.h"
#include "reportfiledb.h"
#include "unknownreport.h"
#include "statistics/virscanstatistics.h"
#include "skylark2/slhashutil.h"
#include "skylark2/skylarkpath.h"
#include "skylark2/filepathtruncater.h"


CScanWork::CScanWork()
{
    m_ExitCleanEvent.Create( NULL, TRUE, FALSE, NULL );
    m_hCleanThread = NULL;
    m_ExitEnumFileEvent.Create( NULL, TRUE, FALSE, NULL );
    m_hEnumFileThread = NULL;
    m_Setting.Reset();
    m_pSvcCallback = NULL;
    m_bScanEnableEntry = TRUE;
    m_bClean = FALSE;
    m_bScan2 = FALSE;
}


CScanWork::~CScanWork()
{
    if ( NULL != m_hEnumFileThread )
    {
        ::CloseHandle( m_hEnumFileThread );
        m_hEnumFileThread = NULL;
    }
    if ( NULL != m_hCleanThread )
    {
        ::CloseHandle( m_hCleanThread );
        m_hCleanThread = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CScanWork::_ResetForScan()
{
    {
        CObjGuard Lock(m_LockData);
        m_CleanInfo.Reset();
        m_VirusFileArray.RemoveAll();
        m_UnknownFileArray.RemoveAll();
    } 
    //扫描时间
    m_StartTimeForScanTime = 0;
    m_BeginPauseTime = 0;
    m_PauseTime = 0;
    //扫描模式
    m_ScanMode = BkScanModeNull;
    //自定义参数
    m_CustomParam.RemoveAll();
    //清除索引
    m_CleanIndexArray.RemoveAll();
    //进度
    m_bCommitFinish = FALSE;
    m_ScanProgress.Reset();
    //枚举线程
    if ( NULL != m_hEnumFileThread )
    {
        ::CloseHandle( m_hEnumFileThread );
        m_hEnumFileThread = NULL;
    };
    m_ExitEnumFileEvent.Reset();
    //扫描上下文
    m_ScanContext.Reset();
    //扫描步骤
    m_ScanStep.Reset();

    m_whiteMap.RemoveAll();

    return S_OK;
}



HRESULT  CScanWork::_ResetForClean()
{
    if ( NULL != m_hCleanThread )
    {
        ::CloseHandle ( m_hCleanThread );
        m_hCleanThread = NULL;
    }
    m_ExitCleanEvent.Reset();
    //清除索引
    m_CleanIndexArray.RemoveAll();

    m_LockData.Lock();
    m_CleanInfo.ScanInfo.Progress = 0;
    m_LockData.Unlock();

    return S_OK;
}

HRESULT CScanWork::Initialize( DWORD CliendPid, IScanFileCallBack* pSvcCallback )
{
    // 检测引擎是否加载
    DWORD dwRet = WaitForSingleObject( g_hThread, INFINITE );
    if ( WAIT_FAILED == dwRet )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
    if ( !g_bInitFalg )
    {
        //引擎初始化失败
        return E_FAIL;
    }

    if ( NULL == pSvcCallback )
    {
        return E_INVALIDARG;
    }

    _ResetForScan();
    _ResetForClean();
    m_pSvcCallback = pSvcCallback;
    m_ScanContext.SetCallback( this );

    HRESULT hr = E_FAIL;

    m_ScanLogWriter.Initialize();
    m_BackupVirusFile.Initialize();
    do 
    {
        hr = m_ScanEngEx.Initialize();
        if ( FAILED(hr) )
        {
            break;
        }
        hr = m_CleanVirusOperation.Initialize();
        if ( FAILED(hr) )
        {
            break;
        }
        hr = S_OK;
    } while (false);
    if ( FAILED(hr) )
    {
        m_CleanVirusOperation.Uninitialize();
        m_ScanEngEx.Uninitialize();
        m_BackupVirusFile.Uninitialize();
        m_ScanLogWriter.Uninitialize();
    }

    return hr;
}


HRESULT CScanWork::Uninitialize()
{
    _Resume();
    if ( m_bClean )
    {
        _ExitCleanVirusThread();
    }
    else
    {
        _ExitEnumFileThread();
        _StopScan();
        //_StopScan没有等待，所以这里要等待。
        m_LockOther.Lock();
        BOOL bFinished = m_bScanEnableEntry;
        m_LockOther.Unlock();
        if ( FALSE == bFinished )
        {
            m_ScanEngEx.WaitForFinish( INFINITE );
        }
    }

    //记录扫描日志
    _RecordScanLog();

    m_CleanVirusOperation.Uninitialize();
    m_ScanEngEx.Uninitialize();
    m_BackupVirusFile.Uninitialize();
    m_ScanLogWriter.Uninitialize();

    return S_OK;
}


HRESULT CScanWork::Scan(
                        int ScanMod,
                        BOOL bBackGround,
                        const CAtlArray<CString>& CustomParam 
                        )
{
    if ( (ScanMod <= BkScanModeNull) 
        || (BkScanModeNum <= ScanMod) )
    {
        return E_INVALIDARG;
    }




    //消重
    {
        CObjGuard Lock( m_LockOther );
        m_bScanEnableEntry;
        if ( FALSE == m_bScanEnableEntry )
        {
            return E_FAIL;
        }
        else
        {
            m_bScanEnableEntry = FALSE;
        }
    }
    m_bClean = FALSE;
    _ResetForScan();

    HRESULT hr = E_FAIL;
    hr = m_ScanEngEx.ReCreateScanEng();
    if ( FAILED(hr) )
    {
        return hr;
    }
    hr = m_ScanEngEx.SetScanEngSetting( m_Setting );
    if ( FAILED(hr) )
    {
        return hr;
    }
    //重置数据


    m_ScanMode = ScanMod;
    {
        CObjGuard Lock( m_LockData );
        ::GetLocalTime( &m_CleanInfo.ScanInfo.StartTime );
        m_CleanInfo.ScanInfo.ScanMode = ScanMod;
        m_CleanInfo.ScanInfo.ScanFinished = FALSE;
        m_CleanInfo.ScanInfo.ScanState = BkScanStateScan;
    }
    m_StartTimeForScanTime = ::GetTickCount();    
    m_CustomParam.Copy( CustomParam ); 
    m_bScan2 = FALSE;

    return _ScanWork();
}

HRESULT CScanWork::Scan2( 
    int ScanMod, 
    BOOL bBackGround, 
    const CAtlArray<BK_SCAN_ADDITIONAL_PATH>& CustomParam
    )
{
    if ( (ScanMod <= BkScanModeNull) 
        || (BkScanModeNum <= ScanMod) )
    {
        return E_INVALIDARG;
    }

    //消重
    {
        CObjGuard Lock( m_LockOther );
        m_bScanEnableEntry;
        if ( FALSE == m_bScanEnableEntry )
        {
            return E_FAIL;
        }
        else
        {
            m_bScanEnableEntry = FALSE;
        }
    }
    m_bClean = FALSE;
    _ResetForScan();

    HRESULT hr = E_FAIL;
    hr = m_ScanEngEx.ReCreateScanEng();
    if ( FAILED(hr) )
    {
        return hr;
    }
    hr = m_ScanEngEx.SetScanEngSetting( m_Setting );
    if ( FAILED(hr) )
    {
        return hr;
    }
    //重置数据

    m_ScanMode = ScanMod;
    {
        CObjGuard Lock( m_LockData );
        ::GetLocalTime( &m_CleanInfo.ScanInfo.StartTime );
        m_CleanInfo.ScanInfo.ScanMode = ScanMod;
        m_CleanInfo.ScanInfo.ScanFinished = FALSE;
        m_CleanInfo.ScanInfo.ScanState = BkScanStateScan;
    }
    m_StartTimeForScanTime = ::GetTickCount();    
    m_CustomParam2.Copy( CustomParam ); 
    m_bScan2 = TRUE;

    return _ScanWork();
}


/**
*@brief     暂停查杀
*/
HRESULT  CScanWork::Pause(
                          )
{
    {
        CObjGuard Lock( m_LockOther );
        if ( m_bScanEnableEntry )
        {
            return E_FAIL;
        }
    }


    HRESULT hr = E_FAIL;
    hr = m_ScanEngEx.Pause();
    if ( FAILED(hr) )
    {
        return hr;
    }


    //设置暂停开始时间
    m_BeginPauseTime = ::GetTickCount();
    //设置暂停状态
    m_LockData.Lock();
    m_CleanInfo.ScanInfo.ScanState = BkScanStatePause;
    m_LockData.Unlock();

    return S_OK;
}

/**
*@brief     暂停查杀
*/
HRESULT CScanWork::Resume(
                          )
{
    {
        CObjGuard Lock( m_LockOther );
        if ( m_bScanEnableEntry )
        {
            return E_FAIL;
        }
    }

    HRESULT hr = _Resume();
    if ( FAILED(hr) )
    {
        return hr;
    }

    //暂停时间


    //取消暂停状态
    m_LockData.Lock();
    m_CleanInfo.ScanInfo.ScanState = BkScanStateScan;
    m_LockData.Unlock();

    return S_OK;
}

/**
*@brief     停止查杀
*/
HRESULT CScanWork::Stop(
                        )
{
    {
        CObjGuard Lock( m_LockOther );
        if ( m_bScanEnableEntry )
        {
            return S_OK;
        }
    }

    _Resume();
    m_LockData.Lock();
    m_CleanInfo.ScanInfo.ScanState = BkScanStateForceStop;
    m_LockData.Unlock();
    if ( m_bClean )
    {
        m_ExitCleanEvent.Set();
    }
    else
    {
        m_ExitEnumFileEvent.Set();
        _StopScan();
    }

    return S_OK;
}


HRESULT CScanWork::QueryScanStep( int ScanMode, CAtlArray<int>& ScanStepArray )
{
    if ( ScanMode <= BkScanModeNull
        || BkScanModeNum <= ScanMode )
    {
        return E_FAIL;
    }

    return m_ScanStep.QueryScanStep( ScanMode, ScanStepArray );
}


/**
*@brief     查询查杀信息
*@param     ScanInfo 返回查杀信息
*/
HRESULT CScanWork::QueryScanInfo(
                                 BK_SCAN_INFO& ScanInfo 
                                 )
{
    _SetScanProgress();
    _SetScanTime();
    {
        CObjGuard Lock( m_LockData );
        ScanInfo = m_CleanInfo.ScanInfo;
    }

    return S_OK;
}

/**
*@brief		查询清除信息
*@param     CleanInfo 返回清除信息
*/
HRESULT CScanWork::QueryCleanInfo(
                                  BK_CLEAN_INFO& CleanInfo
                                  )
{       
    CObjGuard Lock( m_LockData );
    DWORD Num = (DWORD)m_CleanIndexArray.GetCount();
    if ( 0 != Num )
    {
        DWORD Progress = (m_CleanInfo.OperatedVirus * 100) / Num;
        m_CleanInfo.ScanInfo.Progress = (Progress>100) ? 100 : Progress;
    }
    CleanInfo = m_CleanInfo;

    return S_OK;
}

/**
*@brief		查询文件信息
*@param     Type     文件类型（未知、病毒）
*@param     Index    文件索引
*@param     FileInfo 返回文件信息
*/
HRESULT CScanWork::QueryFileInfo(
                                 int       Type,
                                 DWORD       Index,
                                 BK_FILE_INFO& FileInfo
                                 )
{
    if ( (Type <= BkQueryInfoFileTypeNull) 
        || (BkQueryInfoFileTypeNum <= Type) )
    {
        return E_INVALIDARG;
    }


    CObjGuard Lock( m_LockData );

    BK_FILE_RESULT_EX* pFileResultEx = NULL;
    if ( BkQueryInfoFileTypeUnknown == Type )
    {
        if ( Index >= m_UnknownFileArray.GetCount() )
        {
            return E_INVALIDARG;
        }
        pFileResultEx = &(m_UnknownFileArray[Index]);
        FileInfo.FileResult.VirusName   = L"";
        FileInfo.FileResult.VirusType   = BkVirusTypeNull;
    }
    else if ( BkQueryInfoFileTypeVirus == Type )
    {
        if ( Index >= m_VirusFileArray.GetCount() )
        {
            return E_INVALIDARG;
        }
        pFileResultEx = &(m_VirusFileArray[Index]);
        FileInfo.FileResult.VirusName   = pFileResultEx->VirusName;
        FileInfo.FileResult.VirusType   = pFileResultEx->VirusType;
    }
    else
    {
        _ASSERT(0);
        return E_INVALIDARG;
    }
    FileInfo.Index                  = Index; 
    FileInfo.FileResult.SecLevel               = pFileResultEx->uSecLevel;
    FileInfo.FileResult.CRC         = pFileResultEx->CRC;
    FileInfo.FileResult.FileName    = pFileResultEx->FileName;
    CMD5::GetStrMd5( pFileResultEx->MD5, FileInfo.FileResult.MD5 );
    FileInfo.FileResult.Status      = pFileResultEx->Status;
    FileInfo.FileResult.Track       = pFileResultEx->Track;
    FileInfo.FileResult.Type        = pFileResultEx->Type;

    return S_OK;
}

/**
*@brief		清除文件
*@param     IndexArr	病毒类型文件索引数组
*/
HRESULT CScanWork::Clean(
                         const  CAtlArray<DWORD>& fileIndex
                         )
{
    if ( fileIndex.IsEmpty() )
    {
        return E_INVALIDARG;
    }




    //消重
    {
        CObjGuard Lock( m_LockOther );
        if ( FALSE == m_bScanEnableEntry )
        {
            return E_FAIL;
        }
        else
        {
            m_bScanEnableEntry = FALSE;
        }
    }
    m_bClean = TRUE;
    _ResetForClean();

    HRESULT hr = E_FAIL;

    size_t Num = fileIndex.GetCount();
    size_t i = 0;
    for (; i<Num; ++i )
    {
        if ( fileIndex[i] >= m_VirusFileArray.GetCount() )
        {
            return E_INVALIDARG;
        }
    }
    m_CleanIndexArray.Copy( fileIndex );

    BK_SCAN_SETTING CleanSetting;
    CleanSetting = m_Setting;
    CleanSetting.bAutoClean = TRUE;
    hr = m_CleanVirusOperation.SetScanSetting( CleanSetting );
    if ( FAILED(hr) )
    {
        return hr;
    }


    m_LockData.Lock();
    m_CleanInfo.CleanFinished = FALSE;
    m_CleanInfo.OperatedVirus = 0;
    m_CleanInfo.LastCleanFileName = L"";
    m_LockData.Unlock();

    assert(!m_hCleanThread);
    m_hCleanThread = (HANDLE)::_beginthreadex( NULL, 0, CleanVirusFileProc, this, 0, NULL );
    if ( NULL == m_hCleanThread )
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }
    return S_OK;
}


/**
*@brief		查询上次扫描信息
*@param		ScanInfo    查杀信息
*/
HRESULT  CScanWork::QueryLastScanInfo(
                                      BK_SCAN_INFO&	ScanInfo
                                      )
{
    HRESULT RetResult	= E_FAIL;

    RetResult = CScanInfoLog::LoadScanInfo( ScanInfo );

    return RetResult;
}

/**
*@brief		设置扫描设置
*@param		
*/
HRESULT  CScanWork::SetScanSetting(
                                   const BK_SCAN_SETTING&  Setting
                                   )
{
    if ( (Setting.CleanFailedOp <= CleanFailedNull) 
        || (CleanFailedNum <= Setting.CleanFailedOp)
        || (Setting.nScanFileType <=  BkScanFileTypeNull)
        || (BkScanFileTypeNum <= Setting.nScanFileType)
        || Setting.dwSize != sizeof(Setting) )
    {
        return E_INVALIDARG;
    }
    {
        CObjGuard Lock( m_LockOther );
        if ( FALSE == m_bScanEnableEntry )
        {
            //scan 过程中不能设置
            return E_FAIL;
        }
    }

    m_Setting = Setting;

    return S_OK;
}


/**
*@brief		获取扫描选项
*@param		Setting 扫描选项
*/
HRESULT  CScanWork::GetScanSetting(
                                   /* [out ] */BK_SCAN_SETTING&  Setting 
                                   )
{
    Setting = m_Setting;
    return S_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID  CScanWork::_ExitCleanVirusThread()
{
    if ( NULL != m_hCleanThread )
    {
        m_ExitCleanEvent.Set();
        if ( WAIT_OBJECT_0 != ::WaitForSingleObject(m_hCleanThread, INFINITE) )
        {
            ::TerminateThread( m_hCleanThread, 0 );
        }
        ::CloseHandle( m_hCleanThread );
        m_hCleanThread = NULL;
    }
}

VOID  CScanWork::_ExitEnumFileThread()
{
    if ( NULL != m_hEnumFileThread )
    {
        m_ExitEnumFileEvent.Set();
        if ( WAIT_OBJECT_0 != ::WaitForSingleObject(m_hEnumFileThread, INFINITE) )
        {
            ::TerminateThread( m_hEnumFileThread, 0 );
        }
        ::CloseHandle( m_hEnumFileThread );
        m_hEnumFileThread = NULL;
    }
}

VOID   CScanWork::_StopScan()
{
    m_ScanEngEx.NotifyStop();
    //这里没有等待结束，因为，上面调用stop接口。必须马上返回，不能等待。
    //上面判断结束标志是通过，scaninf中的结束标志字段。
}

HRESULT        CScanWork::_Resume()
{
    HRESULT hr = E_FAIL;
    hr = m_ScanEngEx.Resume();
    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( m_BeginPauseTime )
    {
        //暂停时间
        DWORD TmpTime = ::GetTickCount();
        if ( TmpTime < m_PauseTime )
        {
            m_BeginPauseTime = TmpTime;
            TmpTime = ::GetTickCount();
        }
        m_PauseTime += TmpTime - m_BeginPauseTime;
        m_BeginPauseTime = 0;
    }

    return hr;
}


HRESULT CScanWork::_CleanWork()
{
    size_t Num = m_CleanIndexArray.GetCount();
    size_t j = 0;


    for( ; j<Num; ++j )
    {
        UINT64  ulbackid;
        Skylark::BKENG_RESULT ScanResult;
        BKENG_INIT( &ScanResult );
        ScanResult.bHashed = m_VirusFileArray[m_CleanIndexArray[j]].bHashed;
        ScanResult.bIsVirus = TRUE;
        ::CopyMemory( ScanResult.byMD5, m_VirusFileArray[m_CleanIndexArray[j]].MD5, sizeof(m_VirusFileArray[m_CleanIndexArray[j]].MD5) );

        wcsncpy( ScanResult.szVirusName, (LPCWSTR)(m_VirusFileArray[m_CleanIndexArray[j]].VirusName), 64-1 );
        ScanResult.szVirusName[64-1] = NULL;

        ScanResult.uCleanResult = m_VirusFileArray[m_CleanIndexArray[j]].uCleanResult;
        ScanResult.uCRC32 = m_VirusFileArray[m_CleanIndexArray[j]].CRC;
        ScanResult.uScanResult = m_VirusFileArray[m_CleanIndexArray[j]].uScanResult;
        ScanResult.uSecLevel  = m_VirusFileArray[m_CleanIndexArray[j]].uSecLevel;
        ScanResult.bNeedUpload = m_VirusFileArray[m_CleanIndexArray[j]].bNeedUpload;


        //清除病毒前的处理
        if ( BkFileTrackMemory == m_VirusFileArray[m_CleanIndexArray[j]].Track
            || BkFileTrackAutorun == m_VirusFileArray[m_CleanIndexArray[j]].Track )
        {
            //需要清除的文件，可能及是进程，也是注册表项。
            m_ScanContext.OperateCleanInfo( BkScanStepMemory, m_VirusFileArray[m_CleanIndexArray[j]].FileName );
        }
        if ( ScanResult.bIsVirus && m_Setting.bQuarantineVirus )
        {
            if (SLPTL_LEVEL_MARK___LOCAL_HEURIST == ScanResult.uSecLevel)
            {
                // 本地启发式扫描的不删除,所以也不会备份
            }
            else if (SLPTL_LEVEL_MARK___BLACK_HEURIST_LOW_RISK == ScanResult.uSecLevel)
            {
                // 云端启发式的低风险不删除,所以也不会备份
            }
            else
            {
                //添加到隔离区
                m_BackupVirusFile.BackupFile( m_VirusFileArray[m_CleanIndexArray[j]].FileName, &ScanResult, ulbackid );
                if ( NULL != m_pSvcCallback )
                {
                    if ( ( BkScanModeOnlyCustomScan == m_ScanMode ) || ( BkScanModeFastScan == m_ScanMode ) || ( BkScanModeFullScan == m_ScanMode ) )
                    {
                        m_pSvcCallback->BeforeCleanFile( 
                            m_VirusFileArray[m_CleanIndexArray[j]].FileName, 
                            &ScanResult, 
                            NULL, 
                            &ulbackid 
                            );
                    }
                }
            }
        }

        //判断回调是否处理过
        Skylark::BKENG_RESULT CleanResult;
        BKENG_INIT( &CleanResult );
        DWORD dwScanMask =
            BKENG_SCAN_MASK_USE_LOCAL_HEURIST            |
            BKENG_SCAN_MASK_USE_CLOUD_HEURIST_LOW_RISK   |
            BKENG_SCAN_MASK_USE_CLOUD_HEURIST_HIGH_RISK  |
            BKENG_SCAN_MASK_FROM_AUTORUNS;
        m_CleanVirusOperation.HintClean( m_VirusFileArray[m_CleanIndexArray[j]].FileName, dwScanMask, &ScanResult, &CleanResult );

        //清除病毒后的处理
        if ( BkFileTrackMemory == m_VirusFileArray[m_CleanIndexArray[j]].Track
            || BkFileTrackAutorun == m_VirusFileArray[m_CleanIndexArray[j]].Track )
        {
            //需要清除的文件，可能及是进程，也是注册表项。
            m_ScanContext.OperateCleanInfo( BkScanStepMemory, m_VirusFileArray[m_CleanIndexArray[j]].FileName );
            if ( (BKENG_CLEAN_RESULT_DELETE == CleanResult.uCleanResult) 
                || (BKENG_CLEAN_RESULT_DELETE_NEED_REBOOT == CleanResult.uCleanResult)
                || (BKENG_CLEAN_RESULT_DISABLE_AUTORUN == CleanResult.uCleanResult))
            {
                //autorun启动项，并且删除文件。这是进行Autorun注册表clean。
                m_ScanContext.OperateCleanInfo( BkScanStepAutorun, m_VirusFileArray[m_CleanIndexArray[j]].FileName );
            }
        }


        //统计结果
        m_LockData.Lock();
        UINT32 uOldCleanResult = m_VirusFileArray[m_CleanIndexArray[j]].uCleanResult;
        // 这里有统计计数的问题，下面这一行将NO_OP转化为已删除，但是在更新计数
        // _UpdateCleanInfo里面却是还是使用的这个uOldCleanResult变量，依然为NO_OP，统计将不计算为已删除
        // 因此会出现用户反馈统计计数不一样的问题，现在我在引擎中掩盖了一部分NO_OP，但是依然存在
        // 这样的问题
        m_VirusFileArray[m_CleanIndexArray[j]].Status = COther::GetCleanResult( CleanResult.uCleanResult );
        m_VirusFileArray[m_CleanIndexArray[j]].uCleanResult = CleanResult.uCleanResult;
        m_LockData.Unlock();
        _UpdateCleanInfo( m_VirusFileArray[m_CleanIndexArray[j]].FileName, &CleanResult, NULL, uOldCleanResult );


        //判断clean是否要停止。
        DWORD Result = ::WaitForSingleObject(m_ExitCleanEvent, 0);
        if( WAIT_OBJECT_0 == Result
            || WAIT_FAILED  == Result )
        {
            //等待成功，或失败，退出。超时继续clean。
            break;
        }
    }
    m_LockData.Lock();
    DWORD Progress = (m_CleanInfo.OperatedVirus * 100) / Num;
    m_CleanInfo.ScanInfo.Progress = (Progress>100) ? 100 : Progress;
    m_LockData.Unlock();

    m_CleanIndexArray.RemoveAll();

    m_LockData.Lock();
    m_CleanInfo.CleanFinished = TRUE;
    m_LockData.Unlock();

    m_LockOther.Lock();
    m_bScanEnableEntry = TRUE;
    m_LockOther.Unlock();

    return S_OK;
}



HRESULT CScanWork::_ScanWork()
{
    CWhiteList  whitelist;

    if ( SUCCEEDED( whitelist.Initialize() ) )
    {
        CAtlArray<CWhiteListItem>   itemArray;

        if ( SUCCEEDED( whitelist.LoadWhiteList( itemArray ) ) )
        {
            for ( size_t i = 0; i < itemArray.GetCount(); i++ )
            {
                WinMod::CWinPath path = itemArray[ i ].m_filepath;

                path.ExpandNormalizedPathName();
                m_whiteMap[ path ] = itemArray[ i ].m_nType;
            }
        }

        whitelist.Uninitialize();
    }
    
    assert(!m_hEnumFileThread);
    m_hEnumFileThread = (HANDLE)::_beginthreadex( NULL, 0, EnumScanFileProc, this, 0, NULL );
    if ( NULL == m_hEnumFileThread )
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    HRESULT hr = m_ScanEngEx.AsyncScan( this );
    if ( FAILED(hr) )
    {
        _ExitEnumFileThread();
        return hr;
    }
    return hr;
}






DWORD CScanWork::_EnumFileWork()
{    
    DWORD dwRet = 0;
    CAtlMap<CString,BOOL> CustomParamMap;
    size_t i = 0;

    if ( m_bScan2 )
    {
        size_t Num = m_CustomParam2.GetCount();

        for (; i<Num; ++i )
        {
            if ( FALSE == m_CustomParam2[i].strFilePath.IsEmpty() )
            {
                Skylark::CWinPath NormalizePath = m_CustomParam2[i].strFilePath;
                NormalizePath.ExpandNormalizedPathName();
                CustomParamMap.SetAt( NormalizePath.m_strPath, m_CustomParam2[i].bRecursive );
            }
        }
    }
    else
    {
        size_t Num = m_CustomParam.GetCount();

        for (; i<Num; ++i )
        {
            if ( FALSE == m_CustomParam[i].IsEmpty() )
            {
                Skylark::CWinPath NormalizePath = m_CustomParam[i];
                NormalizePath.ExpandNormalizedPathName();
                CustomParamMap.SetAt( NormalizePath.m_strPath, TRUE );
            }
        }
    }

    switch ( m_ScanMode )
    {
    case (BkScanModeCheckScan):
        {
            m_ScanContext.SetScanMode( BkScanModeCheckScan );
            CCheckScanPolicy CheckScanPolicy;
            CheckScanPolicy.StartScan( &m_ScanContext );
        }
        break;

    case (BkScanModeBootSystemScan):
        {
            m_ScanContext.SetScanMode( BkScanModeBootSystemScan );
            CCheckScanPolicy BootSystemScanPolicy;
            BootSystemScanPolicy.StartScan( &m_ScanContext );
        }
        break;

    case (BkScanModeIeFix):
        {
            m_ScanContext.SetScanMode( BkScanModeIeFix );
            CIeFixScanPolicy IeFixScanPolicy;
            IeFixScanPolicy.StartScan( &m_ScanContext );
        }
        break;

    case (BkScanModeOnlyMemoryScan):
        {
            m_ScanContext.SetScanMode( BkScanModeOnlyMemoryScan );
            COnlyMemoryScanPolicy OnlyMemoryPolicy;
            OnlyMemoryPolicy.StartScan( &m_ScanContext );
        }
        break;

    case (BkScanModeFastScan):
        {
            m_ScanContext.SetScanMode( BkScanModeFastScan );
            CFastScanPolicy FastScanPolicy;
            FastScanPolicy.StartScan( &m_ScanContext, CustomParamMap );
        }
        break;

    case (BkScanModeCustomScan):
        {
            m_ScanContext.SetScanMode( BkScanModeCustomScan );
            CAtlMap<CString,BOOL> CustomParamMap;
            CCustomScanPolicy CustomScanPolicy;
            CustomScanPolicy.StartScan( &m_ScanContext, CustomParamMap );
        }
        break;

    case (BkScanModeFullScan):
        {
            m_ScanContext.SetScanMode( BkScanModeFullScan );
            CFullDiskScanPolicy FullDisScanPolicy;
            FullDisScanPolicy.StartScan( &m_ScanContext );
        }
        break;

    case (BKScanModeMoveDiskScan):
        {
            m_ScanContext.SetScanMode( BKScanModeMoveDiskScan );
            CMoveDiskScanPolicy MoveDisScanPolicy;
            MoveDisScanPolicy.StartScan( &m_ScanContext );
        }
        break;

    case (BkScanModeRightScan):
        {
            m_ScanContext.SetScanMode( BkScanModeRightScan );
            CRightScanPolicy RightScanPolicy;
            RightScanPolicy.StartScan( &m_ScanContext, CustomParamMap );
        }
        break;

    case (BkScanModeOnlyCustomScan):
        {
            m_ScanContext.SetScanMode( BkScanModeOnlyCustomScan );
            COnlyCustomScanPolicy OnleyCustomScanPolicy;

            OnleyCustomScanPolicy.StartScan( &m_ScanContext, CustomParamMap );
        }
        break;
    case BkScanModeCacheMemory:
        {
            m_ScanContext.SetScanMode( BkScanModeCacheMemory );
            CCheckScanPolicy cachememory;
            cachememory.StartScanForCacheMemory( &m_ScanContext );
        }
    case BkScanModeCacheAutorun:
        {
            m_ScanContext.SetScanMode( BkScanModeCacheAutorun );
            CCheckScanPolicy cacheautorun;
            cacheautorun.StartScanForCacheAutoRun( &m_ScanContext );
        }
    default:
        {
            _ASSERT(0);
        }
        break;
    }

    m_ScanEngEx.Commit();
    m_bCommitFinish = TRUE;

    return dwRet;
}



////////////////////////////////////////////////////////////////////////
/**
* @brief        扫描文件前调用
* @param[in]    lpszFilePath    原始文件路径
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/
HRESULT STDMETHODCALLTYPE CScanWork::BeforeScanFile(
    /* [in ] */ LPCWSTR             lpszFilePath,
    /* [in ] */ void*               pvFlag)
{
    return S_OK;
}



/**
* @brief        扫描文件后调用
* @param[in]    lpszFilePath    原始文件路径
* @param[in]    pResult         当前文件的扫描结果
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/
HRESULT STDMETHODCALLTYPE CScanWork::AfterScanFile(
    /* [in ] */ LPCWSTR             lpszFilePath,
    /* [in ] */ const Skylark::BKENG_RESULT* pResult,
    /* [in ] */ void*               pvFlag)
{
    if ( sizeof(Skylark::BKENG_RESULT) != pResult->uSize )
    {
        _ASSERT( 0 );
    }


    if (SLPTL_LEVEL_MARK___LOCAL_HEURIST == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_HEURIST_WITH_CLOUD_LOW_RISK == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_WIN_TRUST_HEURIST == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___BLACK_HEURIST_HIGH_RISK == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___BLACK_HEURIST_LOW_RISK == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_NO_NEED_SRV_HEURIST_HIGH_RISK == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_NO_NEED_SRV_HEURIST_LOW_RISK == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_WIN_TRUST_CLOUD_HIGH_RISK == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_WIN_TRUST_CLOUD_LOW_RISK == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_WIN_TRUST_BOTH_HEURIST == pResult->uSecLevel)
    {
        CString strTruncatePath;
        Skylark::CFilePathTruncater::Instance().TruncateFilePath(lpszFilePath, strTruncatePath);

        CString strMD5;
        Skylark::CHexUtilW::HexFromBuffer(pResult->byMD5, 16, strMD5);
        CVirScanStc::Instance().ReportHeuristTrojans(strTruncatePath, strMD5, pResult->uSecLevel, pResult->szVirusName);
    }

    if (
        SLPTL_LEVEL_MARK___LOCAL_FAKE_SYS_FILE == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_WIN_TRUST_FAKE_SYS == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_FAKE_SYS_GRAY_TRUST == pResult->uSecLevel
        || SLPTL_LEVEL_MARK___LOCAL_FAKE_SYS_GRAY_UNTRUST == pResult->uSecLevel)
    {
        CString strMD5;
        Skylark::CHexUtilW::HexFromBuffer(pResult->byMD5, 16, strMD5);
        CVirScanStc::Instance().ReportSfmTrojans(CWinPathApi::FindFileName(lpszFilePath), strMD5, pResult->uSecLevel);
    }


    _UpdateScanResult( lpszFilePath, pResult, pvFlag );
    _UpdateScanInfo( lpszFilePath, pResult, pvFlag );


    if ( m_pSvcCallback )
    {
        if ( ( BkScanModeOnlyCustomScan == m_ScanMode ) 
            || ( BkScanModeFastScan == m_ScanMode ) 
            || ( BkScanModeFullScan == m_ScanMode ) )
        {
            m_pSvcCallback->AfterScanFile( 
                lpszFilePath, 
                pResult, 
                pvFlag
                );
        }
    }
    return S_OK;
}


/**
* @brief        清除文件前调用 (禁用自动清除时,不会调用此回调)
* @param[in]    lpszFilePath    原始文件路径
* @param[in]    pResult         当前文件的扫描结果
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/
//这个函数，只有在用户设置了自动清除时才会被调用。
HRESULT STDMETHODCALLTYPE CScanWork::BeforeCleanFile(
    /* [in ] */ LPCWSTR             lpszFilePath,
    /* [in ] */ const Skylark::BKENG_RESULT* pResult,
    /* [in ] */ void*               pvFlag)
{
    UINT64  ulbackid;
    if ( sizeof(Skylark::BKENG_RESULT) != pResult->uSize )
    {
        _ASSERT( 0 );
    }

    int ScanStep = (INT)(INT_PTR)pvFlag;
    if ( BkScanStepMemory == ScanStep
        || BkScanStepAutorun == ScanStep )
    {
        //需要清除的文件，可能及是进程，也是注册表项。
        m_ScanContext.OperateCleanInfo( BkScanStepMemory, lpszFilePath );
    }

    if ( pResult->bIsVirus && m_Setting.bQuarantineVirus )
    {
        if (SLPTL_LEVEL_MARK___LOCAL_HEURIST == pResult->uSecLevel)
        {
            // 本地启发式扫描的不删除,所以也不会备份
        }
        else if (SLPTL_LEVEL_MARK___BLACK_HEURIST_LOW_RISK == pResult->uSecLevel)
        {
            // 云端启发式的低风险不删除,所以也不会备份
        }
        else
        {
            //添加到隔离区
            m_BackupVirusFile.BackupFile( lpszFilePath, pResult, ulbackid, pvFlag );
            if ( m_pSvcCallback )
            {
                if ( ( BkScanModeOnlyCustomScan == m_ScanMode )
                    || ( BkScanModeFastScan == m_ScanMode )
                    || ( BkScanModeFullScan == m_ScanMode ) )
                {
                    m_pSvcCallback->BeforeCleanFile( 
                        lpszFilePath, 
                        pResult, 
                        pvFlag, 
                        &ulbackid 
                        );
                }
            }
        }
    }


    return S_OK;
}


/**
* @brief        清除文件后调用 (禁用自动清除时,不会调用此回调)
* @param[in]    lpszFilePath    原始文件路径
* @param[in]    pResult         当前文件的清除结果
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/
//这个函数，只有在用户设置了自动清除时才会被调用。
HRESULT STDMETHODCALLTYPE CScanWork::AfterCleanFile(
    /* [in ] */ LPCWSTR             lpszFilePath,
    /* [in ] */ const Skylark::BKENG_RESULT* pResult,
    /* [in ] */ void*               pvFlag)
{
    if ( sizeof(Skylark::BKENG_RESULT) != pResult->uSize )
    {
        _ASSERT( 0 );
    }

    int ScanStep = (INT)(INT_PTR)pvFlag;
    if ( BkScanStepMemory == ScanStep
        || BkScanStepAutorun == ScanStep )
    {
        //需要清除的文件，可能及是进程，也是注册表项。

        m_ScanContext.OperateCleanInfo( BkScanStepMemory, lpszFilePath );

        if ( (BKENG_CLEAN_RESULT_DELETE == pResult->uCleanResult) 
            || (BKENG_CLEAN_RESULT_DELETE_NEED_REBOOT == pResult->uCleanResult)
            || (BKENG_CLEAN_RESULT_DISABLE_AUTORUN == pResult->uCleanResult))
        {
            //autorun启动项，并且删除文件。这是进行Autorun注册表clean。
            m_ScanContext.OperateCleanInfo( BkScanStepAutorun, lpszFilePath );
        }
    }

    //注意这函数的调用顺序，现记录结果在添加计数。
    _UpdateCleanResult( lpszFilePath, pResult, pvFlag );
    _UpdateCleanInfo( lpszFilePath, pResult, NULL );



    return S_OK;
}


/**
* @brief        扫描过程中返回细节 (如壳信息,压缩包中的病毒等...)
* @param[in]    lpszVirtualPath 扫描对象的虚拟路径
* @param[in]    pResult         扫描对象的扫描结果
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/ 
HRESULT STDMETHODCALLTYPE CScanWork::OnScanDetail(
    /* [in ] */ LPCWSTR             lpszVirtualPath,
    /* [in ] */ const Skylark::BKENG_RESULT* pResult,
    /* [in ] */ void*               pvFlag)
{
    if ( sizeof(Skylark::BKENG_RESULT) != pResult->uSize )
    {
        _ASSERT( 0 );
    }
    return S_OK;
}


/**
* @brief        空闲时候的回调
* @param[in]    lpszFilePath    当前正在扫描的文件 (如果当前没有正在扫描的文件,这里可能为NULL)
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/ 
HRESULT STDMETHODCALLTYPE CScanWork::OnIdle()
{
    return S_OK;
}

/**
* @brief        结束时回调
* @param[in]    hrFinishCode    扫描返回的错误码
*/ 
HRESULT STDMETHODCALLTYPE CScanWork::OnFinish(
    HRESULT             hrFinishCode)
{


    //只有在scan时，才更新信息。
    _SetScanTime();
    _SetScanProgress();

    m_LockData.Lock();
    if ( BkScanStateForceStop != m_CleanInfo.ScanInfo.ScanState )
    {
        //当stop时，ScanState 会赋值为BkScanStateForceStop状态。这里就不能在赋值了。
        m_CleanInfo.ScanInfo.ScanState = BkScanStateStop;
    }
    m_CleanInfo.ScanInfo.ScanFinished = TRUE;
    m_LockData.Unlock();

    //只有在scan时才记录扫描日志
    _SaveLastScanInfo();

    m_LockOther.Lock();
    m_bScanEnableEntry = TRUE;
    m_LockOther.Unlock();


    // 减少工作集大小,任务管理器中看到的内存占用将减小
    ::SetProcessWorkingSetSize(::GetCurrentProcess(), -1, -1);

    return S_OK;
}


HRESULT CScanWork::OperateFile( const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask, void* pvFlag,  PVOID pParam )
{  
    if ( FALSE == WinMod::CWinPathApi::IsFileExisting(FilePath) )
    {
        return E_FAIL;
    }

    HRESULT hr = E_FAIL;

    if ( m_whiteMap.Lookup( FilePath ) )
    {
        return S_OK;
    }

    m_ScanStep.IncScanFileNum( m_ScanMode, (INT)(INT_PTR)pvFlag );

    Skylark::BKENG_SCAN_PARAM ScanParam;
    Skylark::BKENG_INIT(&ScanParam);
    ScanParam.pFindData  = pFindData;
    ScanParam.dwScanMask = dwScanMask;
    ScanParam.pvFlag     = pvFlag;
    hr = m_ScanEngEx.AppendFileEx( FilePath, &ScanParam );
    if ( FAILED(hr) )
    {
        m_ScanStep.DecScanFileNum( m_ScanMode, (INT)(INT_PTR)pvFlag );
        return hr;
    }
    //之所以没要在，appendfile 成功后添加文件是，因为原因。
    //appendfile之后，还没有执行IncScanFileNum时，扫描回调就来了这时，扫描的文件个数就大于了添加到引擎的文件个数。
    //这时扫描步骤，取得的就不对了。
    //appendfile 之前就，增加引用计数。

    return hr;
}

BOOL    CScanWork::IsExitEnumWork()
{ 
    DWORD Result = ::WaitForSingleObject(m_ExitEnumFileEvent, 0);
    if ( (WAIT_FAILED == Result)
        || (WAIT_OBJECT_0 == Result) )
    {
        //等待成功，或函数失败。返回TRUE。
        return TRUE;
    }
    return FALSE;
}


VOID CScanWork::_SetScanTime()
{
    DWORD TickCount = ::GetTickCount();
    if ( TickCount < m_StartTimeForScanTime )
    {
        m_StartTimeForScanTime = TickCount;
        TickCount = ::GetTickCount();
    }

    {
        CObjGuard Lock( m_LockData );
        if ( (0 == m_BeginPauseTime) 
            && (FALSE == m_CleanInfo.ScanInfo.ScanFinished) )
        {
            //扫描没有结束，而且没有处于暂停状态时，记录时间。
            m_CleanInfo.ScanInfo.ScanTime = TickCount - m_StartTimeForScanTime - m_PauseTime;
        }
    }
}


VOID    CScanWork::_SetScanProgress()
{
    m_LockData.Lock();
    DWORD ScanProgress = m_CleanInfo.ScanInfo.Progress;
    DWORD FileTotalNum = m_ScanStep.GetTotalFileNum();
    DWORD ScanFileNum = m_CleanInfo.ScanInfo.ScanFileCount;
    m_LockData.Unlock();



    DWORD Progress = m_ScanProgress.GetScanProgress(
        m_ScanMode,
        ScanFileNum,
        FileTotalNum,
        ScanProgress,
        m_bCommitFinish);

    {
        CObjGuard Lock( m_LockData );
        if ( Progress > m_CleanInfo.ScanInfo.Progress )
        {
            m_CleanInfo.ScanInfo.Progress = Progress;
        }
    }
}



VOID CScanWork::_SaveLastScanInfo()
{    
    BK_SCAN_INFO ScanInfo;
    {
        CObjGuard Lock( m_LockData );

        ScanInfo = m_CleanInfo.ScanInfo;
    }
    CScanInfoLog::SaveScanInfo( ScanInfo );
}



BOOL CScanWork::_IsFindFile( CAtlArray<BK_FILE_RESULT_EX>& FileArray, LPCWSTR pszFilePath )
{
    size_t Num = FileArray.GetCount();
    size_t i    = 0;
    for (; i<Num; ++i )
    {
        if ( 0 == FileArray[i].FileName.CompareNoCase(pszFilePath) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

VOID CScanWork::_UpdateScanResult( LPCWSTR  lpszFilePath, const Skylark::BKENG_RESULT* pResult, void* pvFlag )
{
    BK_FILE_RESULT_EX FileResultEx;
    InitFileResultEx( FileResultEx );
    BOOL bFindFile = FALSE;

    if ( pResult->bIsVirus || pResult->bNeedUpload )
    {
        FileResultEx.FileName       = lpszFilePath;
        FileResultEx.Track          = COther::TranslateScanStepToTrack( (INT)( INT_PTR )pvFlag );
        FileResultEx.Status	        = COther::GetScanResult( pResult->uScanResult );
        FileResultEx.CRC            = pResult->uCRC32;
        ::CopyMemory( FileResultEx.MD5, pResult->byMD5, sizeof(pResult->byMD5) );
        FileResultEx.bHashed        = pResult->bHashed;
        FileResultEx.uCleanResult   = pResult->uCleanResult;
        FileResultEx.uScanResult    = pResult->uScanResult;
        FileResultEx.uSecLevel      = pResult->uSecLevel;
        FileResultEx.bNeedUpload    = pResult->bNeedUpload;
    }

    if ( pResult->bIsVirus )
    {
        //消重
        m_LockData.Lock();
        bFindFile = _IsFindFile( m_VirusFileArray, lpszFilePath );
        m_LockData.Unlock();
        if ( bFindFile )
        {
            return;
        }
        FileResultEx.Type           = BkQueryInfoFileTypeVirus;
        FileResultEx.VirusName      = pResult->szVirusName;
        FileResultEx.VirusType      = COther::GetVirusType( pResult->uScanResult );

        m_LockData.Lock();
        m_VirusFileArray.Add( FileResultEx );
        m_LockData.Unlock();
    }

    if ( pResult->bNeedUpload )
    {
        //消重
        m_LockData.Lock();
        bFindFile = _IsFindFile( m_UnknownFileArray, lpszFilePath );
        m_LockData.Unlock();
        if ( bFindFile )
        {
            return;
        }

        if( !pResult->bIsVirus )
        {
            FileResultEx.Type           = BkQueryInfoFileTypeUnknown;
            FileResultEx.VirusName      = L"";
            FileResultEx.VirusType      = BkVirusTypeNull;
        }

        m_LockData.Lock();
        m_UnknownFileArray.Add( FileResultEx );
        m_LockData.Unlock();
    }
}

VOID CScanWork::_UpdateScanInfo( LPCWSTR  lpszFilePath, const Skylark::BKENG_RESULT* pResult, void* pvFlag )
{
    CObjGuard	Lock( m_LockData );

    m_CleanInfo.ScanInfo.LastScanFileName = lpszFilePath;
    m_CleanInfo.ScanInfo.ScanFileCount += 1;
    m_CleanInfo.ScanInfo.Step = m_ScanStep.GetScanStep( m_ScanMode, m_CleanInfo.ScanInfo.ScanFileCount, (INT)( INT_PTR )pvFlag );
    if ( pResult->bIsVirus )
    {
        if ( FALSE == m_Setting.bAutoClean )
        {
            //不是自动清除时添加，病毒计数。
            m_CleanInfo.ScanInfo.VirusCount = (DWORD)m_VirusFileArray.GetCount();
        }
        else
        {
            //是自动清除，病毒计数在清除后添加。
            //为了保证上面得病毒状态的正确性。
        }
        if ( BKENG_SCAN_RESULT_VIRUS_NO_OP == pResult->uScanResult )
        {
            m_CleanInfo.ScanInfo.CantCleanVirus += 1;
        }
    }

    if ( pResult->bNeedUpload )  
    {
        //未知
        m_CleanInfo.ScanInfo.UnkownCount = (DWORD)m_UnknownFileArray.GetCount();
    }
    else
    {
        if ( BKENG_SCAN_RESULT_FAILED_TO_SCAN == pResult->uScanResult )
        {
            //扫描失败
            m_CleanInfo.ScanInfo.QueryFailedCount += 1;
        }
        m_CleanInfo.ScanInfo.SafeCount        += 1;
        m_CleanInfo.ScanInfo.TrustCount       += 1;
    }
}




VOID CScanWork::_UpdateCleanResult( LPCWSTR  lpszFilePath, const Skylark::BKENG_RESULT* pResult, void* pvFlag )
{
    CObjGuard	Lock( m_LockData );

    if (  pResult->bIsVirus )
    {
        size_t Num = m_VirusFileArray.GetCount();
        size_t i   = 0;
        for (; i<Num; ++i )
        {
            if ( 0 == m_VirusFileArray[i].FileName.CompareNoCase(lpszFilePath) )
            {
                m_VirusFileArray[i].Status = COther::GetCleanResult( pResult->uCleanResult );
                m_VirusFileArray[i].uCleanResult = pResult->uCleanResult;
                break;   
            }
        }
    }
}


VOID CScanWork::_UpdateCleanInfo( LPCWSTR  lpszFilePath, const Skylark::BKENG_RESULT* pResult, void* pvFlag, UINT32 uOldCleanResult )
{
    CObjGuard	Lock( m_LockData );

    if ( m_Setting.bAutoClean )
    {
        //自动清除时，这里添加病毒计数。
        m_CleanInfo.ScanInfo.VirusCount = (DWORD)m_VirusFileArray.GetCount();
    }
    m_CleanInfo.LastCleanFileName = lpszFilePath;
    m_CleanInfo.OperatedVirus += 1;
    if ( BKENG_CLEAN_RESULT_NO_OP == pResult->uCleanResult )
    {
        _ASSERT(0);//目前没有返回这个结果。
    }
    else if ( BKENG_CLEAN_RESULT_DELETE == pResult->uCleanResult
        || BKENG_CLEAN_RESULT_DISINFECT == pResult->uCleanResult
        || BKENG_CLEAN_RESULT_DISABLE_AUTORUN == pResult->uCleanResult)
    {
        //清除
        m_CleanInfo.ScanInfo.CleanedVirus += 1;
    }
    else if ( BKENG_CLEAN_RESULT_FAILED_TO_DELETE == pResult->uCleanResult
        || BKENG_CLEAN_RESULT_FAILED_TO_DISINFECT == pResult->uCleanResult )
    {
        m_CleanInfo.ScanInfo.CleanFailedCount += 1;
    }
    else if ( BKENG_CLEAN_RESULT_DELETE_NEED_REBOOT == pResult->uCleanResult
        || BKENG_CLEAN_RESULT_DISINFECT_NEED_REBOOT == pResult->uCleanResult )
    {
        m_CleanInfo.ScanInfo.RebootCount += 1;
    }
    else
    {
        _ASSERT(0);
    }



    // 如果该对象曾经被清除过,那么这里需要修正计数
    switch (uOldCleanResult)
    {
    case BKENG_CLEAN_RESULT_DELETE:
    case BKENG_CLEAN_RESULT_DISINFECT:
    case BKENG_CLEAN_RESULT_DISABLE_AUTORUN:
        m_CleanInfo.ScanInfo.CleanedVirus -= 1;
        m_CleanInfo.OperatedVirus -= 1;
        break;
    case BKENG_CLEAN_RESULT_FAILED_TO_DELETE:
    case BKENG_CLEAN_RESULT_FAILED_TO_DISINFECT:
        m_CleanInfo.ScanInfo.CleanFailedCount -= 1;
        m_CleanInfo.OperatedVirus -= 1;
        break;
    case BKENG_CLEAN_RESULT_DELETE_NEED_REBOOT:
    case BKENG_CLEAN_RESULT_DISINFECT_NEED_REBOOT:
        m_CleanInfo.ScanInfo.RebootCount -= 1;
        m_CleanInfo.OperatedVirus -= 1;
        break;
    }


}


VOID       CScanWork::_RecordScanLog()
{
    size_t Num = m_VirusFileArray.GetCount();
    size_t i = 0;
    for(; i<Num; ++i )
    {
        Skylark::BKENG_SCAN_LOG ScanLog;
        COther::TranslateFileResultExToScanLog( m_VirusFileArray[i], ScanLog );
        m_ScanLogWriter.WriteLog( ScanLog );
    }
    //保存日志。
    m_ScanLogWriter.SaveToFile();
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

CBkMultiFileScan::CBkMultiFileScan()
{
    m_Setting.Reset();
}



CBkMultiFileScan::~CBkMultiFileScan()
{

}



HRESULT  CBkMultiFileScan::Initialize()
{
    // 检测引擎是否加载
    DWORD dwRet = WaitForSingleObject( g_hThread, INFINITE );
    if ( WAIT_FAILED == dwRet )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
    if ( !g_bInitFalg )
    {
        //引擎初始化失败
        return E_FAIL;
    }

    HRESULT hr = m_EngineLiteEx.Initialize();
    if ( FAILED( hr ) )
    {
        return hr;
    }

    hr = m_ScanEngEx.Initialize();
    if ( FAILED(hr) )
    {
        m_EngineLiteEx.Uninitialize();
        return hr;
    }

    m_BackupVirusFile.Initialize();
    m_ScanLogWriter.Initialize();
    return S_OK;
}



HRESULT  CBkMultiFileScan::Uninitialize()
{
    m_ScanLogWriter.Uninitialize();
    m_BackupVirusFile.Uninitialize();
    m_EngineLiteEx.Uninitialize();
    m_ScanEngEx.Uninitialize();
    return S_OK;
}


HRESULT CBkMultiFileScan::Stop()
{
    m_ScanEngEx.NotifyStop();
    m_ScanEngEx.WaitForFinish(INFINITE);
    return S_OK;
}


HRESULT  CBkMultiFileScan::Scan( 
                                const CAtlArray<CString>& FilePathArray,
                                CAtlArray<BK_FILE_RESULT>& FileResultArray
                                )
{
    HRESULT hr = E_FAIL;




    hr = m_ScanEngEx.ReCreateScanEng();
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_ScanEngEx.SetScanEngSetting( m_Setting );
    if ( FAILED(hr) )
    {
        return hr;
    }
    _Reset( FilePathArray );

    //添加文件
    size_t Num = m_FileResultArray.GetCount();
    size_t i = 0;
    for ( i; i<Num; ++i )
    {
        hr = m_ScanEngEx.AppendFile( m_FileResultArray[i].FileName );
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    //提交
    hr = m_ScanEngEx.Commit();
    if ( FAILED(hr) )
    {
        return hr;
    }

    //扫描
    hr = m_ScanEngEx.AsyncScan( this );
    if ( FAILED(hr) )
    {
        return hr;
    }

    //等待
    m_ScanEngEx.WaitForFinish(INFINITE);

    //路径去前缀
    size_t NumI = m_FileResultArray.GetCount();
    size_t IndexI = 0;
    for ( IndexI; IndexI<NumI; ++IndexI )
    {
        size_t NumJ = FilePathArray.GetCount();
        size_t IndexJ = 0;
        for ( IndexJ; IndexJ<NumJ; ++IndexJ )
        {
            CString TmpFilePath = FilePathArray[IndexJ];
            TmpFilePath.MakeLower();
            if ( -1 != m_FileResultArray[IndexI].FileName.Find(TmpFilePath) )
            {
                m_FileResultArray[IndexI].FileName = FilePathArray[IndexJ];
                break;
            }
        }
    }
    FileResultArray.Copy( m_FileResultArray );

    hr = S_OK;
    return hr;
}

HRESULT  CBkMultiFileScan::Scan2( 
                                const CAtlArray<CString>& FilePathArray,
                                CAtlArray<BK_FILE_RESULT>& FileResultArray,
                                CAtlArray<size_t>& unKnownIndex
                                )
{
    HRESULT hr = E_FAIL;

    hr = m_ScanEngEx.ReCreateScanEng();
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_ScanEngEx.SetScanEngSetting( m_Setting );
    if ( FAILED(hr) )
    {
        return hr;
    }

    _Reset( FilePathArray );

    //添加文件
    size_t Num = m_FileResultArray.GetCount();
    size_t i = 0;
    for ( i; i<Num; ++i )
    {
        hr = m_ScanEngEx.AppendFile( m_FileResultArray[i].FileName );
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    //提交
    hr = m_ScanEngEx.Commit();
    if ( FAILED(hr) )
    {
        return hr;
    }

    //扫描
    hr = m_ScanEngEx.AsyncScan( this );
    if ( FAILED(hr) )
    {
        return hr;
    }

    //等待
    m_ScanEngEx.WaitForFinish(INFINITE);

    //路径去前缀
    //刘硕
    //这里要调整，result数组里不填充展开的路径
    size_t NumI = m_FileResultArray.GetCount();
    size_t IndexI = 0;
    for ( IndexI; IndexI<NumI; ++IndexI )
    {
        size_t NumJ = FilePathArray.GetCount();
        size_t IndexJ = 0;
        for ( IndexJ; IndexJ<NumJ; ++IndexJ )
        {
            CString TmpFilePath = FilePathArray[IndexJ];
            TmpFilePath.MakeLower();
            if ( -1 != m_FileResultArray[IndexI].FileName.Find(TmpFilePath) )
            {
                m_FileResultArray[IndexI].FileName = FilePathArray[IndexJ];
                break;
            }
        }
    }
    FileResultArray.Copy( m_FileResultArray );
    unKnownIndex.Copy( m_unKnownIndex );
    hr = S_OK;
    return hr;
}

HRESULT CBkMultiFileScan::ScanHash( 
    const CAtlArray<CString>& hashArray, 
    CAtlArray<BK_FILE_RESULT>& frArray 
    )
{
    HRESULT hr = E_FAIL;

    hr = m_EngineLiteEx.SetScanSetting( m_Setting );
    if ( FAILED(hr) )
    {
        return hr;
    }

    for ( size_t i = 0; i < hashArray.GetCount(); i++ )
    {
        Skylark::BKENG_HASH_SCAN_PARAM    HashScanParam;
        Skylark::BKENG_RESULT KengResult;
        BK_FILE_RESULT        ScanResult;

        Skylark::BKENG_INIT( &HashScanParam );

        HashScanParam.bUseHex = TRUE;

        if ( hashArray[ i ].GetLength() < 40 )
        {
            continue;
        }

        memcpy( HashScanParam.szHex, hashArray[ i ], 40 * sizeof( WCHAR ) );

        InitFileResult( ScanResult );
        hr = m_EngineLiteEx.ScanHash( &HashScanParam, &KengResult );
        if ( FAILED(hr) )
        {
            continue;
        }

        ScanResult.FileName = L""; 
        if ( m_Setting.bAutoClean )
        {
            ScanResult.Status	 = COther::GetCleanResult( KengResult.uCleanResult );
        }
        else
        {
            ScanResult.Status	 = COther::GetScanResult( KengResult.uScanResult );
        }

        CMD5::GetStrMd5( KengResult.byMD5, ScanResult.MD5 );
        ScanResult.CRC = KengResult.uCRC32;

        if ( KengResult.bIsVirus )
        {
            ScanResult.Type      = BkQueryInfoFileTypeVirus;
            ScanResult.VirusName = KengResult.szVirusName;
            ScanResult.VirusType = COther::GetVirusType( KengResult.uScanResult );
        }
        else
        {
            ScanResult.Type      = BkQueryInfoFileTypeSafe;
        }

        if ( KengResult.bNeedUpload )
        {
            ScanResult.Type      = BkQueryInfoFileTypeUnknown;
        }

        frArray.Add( ScanResult );
    }

    return hr;
}

HRESULT  CBkMultiFileScan::SetScanSetting( 
    /* [in ] */const BK_SCAN_SETTING& Setting
    )
{
    if ( (Setting.CleanFailedOp <= CleanFailedNull) 
        || (CleanFailedNum <= Setting.CleanFailedOp)
        || (Setting.nScanFileType <=  BkScanFileTypeNull)
        || (BkScanFileTypeNum <= Setting.nScanFileType)
        || Setting.dwSize != sizeof(Setting) )
    {
        return E_INVALIDARG;
    }

    m_Setting = Setting;
    return S_OK;
}



HRESULT  CBkMultiFileScan::GetScanSetting(
    /* [out ] */BK_SCAN_SETTING& Setting 
    )
{
    Setting = m_Setting;
    return S_OK;
}


/**
* @brief        扫描文件前调用
* @param[in]    lpszFilePath    原始文件路径
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/
HRESULT STDMETHODCALLTYPE CBkMultiFileScan::BeforeScanFile(
    /* [in ] */ LPCWSTR             lpszFilePath,
    /* [in ] */ void*               pvFlag)
{
    return S_OK;
}


/**
* @brief        扫描文件后调用
* @param[in]    lpszFilePath    原始文件路径
* @param[in]    pResult         当前文件的扫描结果
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/
HRESULT STDMETHODCALLTYPE CBkMultiFileScan::AfterScanFile(
    /* [in ] */ LPCWSTR             lpszFilePath,
    /* [in ] */ const Skylark::BKENG_RESULT* pResult,
    /* [in ] */ void*               pvFlag)
{
    if ( sizeof(Skylark::BKENG_RESULT) != pResult->uSize )
    {
        _ASSERT(0);
    }


    size_t Num = m_FileResultArray.GetCount();
    size_t i   = 0;
    for (; i<Num; ++i )
    {
        if ( 0 == m_FileResultArray[i].FileName.CompareNoCase(lpszFilePath) )
        {
            m_FileResultArray[i].SecLevel = pResult->uSecLevel;
            m_FileResultArray[i].Status	 = COther::GetScanResult( pResult->uScanResult );
            m_FileResultArray[i].CRC       = pResult->uCRC32;
            CMD5::GetStrMd5( pResult->byMD5, m_FileResultArray[i].MD5 );

            if ( pResult->bNeedUpload )
            {
				// 快扫不过卫士云时，pResult->bNeedUpload 会一直为真，
				// 因此，这里把这种情况也归为安全，以让回扫流程能
				// 在毒霸云上顺利进行下去（如回扫数据库能够刷新等）
                m_FileResultArray[i].Type		= BkQueryInfoFileTypeSafe;
                m_unKnownIndex.Add( i );
            }
            else
            {
                if ( pResult->bIsVirus )
                {
                    m_FileResultArray[i].Type      = BkQueryInfoFileTypeVirus;
                    m_FileResultArray[i].VirusName = pResult->szVirusName;
                    m_FileResultArray[i].VirusType = COther::GetVirusType( pResult->uScanResult );
                }
                else
                {
                    m_FileResultArray[i].Type      = BkQueryInfoFileTypeSafe;
                }
            }

            m_FileResultArray[ i ].bHashed = pResult->bHashed;
            break;
        }
    }

    if ( pResult->bIsVirus 
        && (FALSE == m_Setting.bAutoClean) )
    {
        m_ScanLogWriter.WriteResultAsLog( lpszFilePath, pResult );
        m_ScanLogWriter.SaveToFile();
    }


    return S_OK;
}


/**
* @brief        清除文件前调用 (禁用自动清除时,不会调用此回调)
* @param[in]    lpszFilePath    原始文件路径
* @param[in]    pResult         当前文件的扫描结果
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/
HRESULT STDMETHODCALLTYPE CBkMultiFileScan::BeforeCleanFile(
    /* [in ] */ LPCWSTR             lpszFilePath,
    /* [in ] */ const Skylark::BKENG_RESULT* pResult,
    /* [in ] */ void*               pvFlag)
{
    UINT64  ulbackid;
    if ( sizeof(Skylark::BKENG_RESULT) != pResult->uSize )
    {
        _ASSERT(0);
    }

    if ( pResult->bIsVirus && m_Setting.bAutoClean )
    {
        m_ScanLogWriter.WriteResultAsLog( lpszFilePath, pResult );
        m_ScanLogWriter.SaveToFile();
    }
    if ( pResult->bIsVirus && m_Setting.bQuarantineVirus )
    {
        if (SLPTL_LEVEL_MARK___LOCAL_HEURIST == pResult->uSecLevel)
        {
            // 本地启发式扫描的不删除,所以也不会备份
        }
        else if (SLPTL_LEVEL_MARK___BLACK_HEURIST_LOW_RISK == pResult->uSecLevel)
        {
            // 云端启发式的低风险不删除,所以也不会备份
        }
        else
        {
            //添加到隔离区
            m_BackupVirusFile.BackupFile( lpszFilePath, pResult, ulbackid, pvFlag );
        }
    }

    return S_OK;
}


/**
* @brief        清除文件后调用 (禁用自动清除时,不会调用此回调)
* @param[in]    lpszFilePath    原始文件路径
* @param[in]    pResult         当前文件的清除结果
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/
HRESULT STDMETHODCALLTYPE CBkMultiFileScan::AfterCleanFile(
    /* [in ] */ LPCWSTR             lpszFilePath,
    /* [in ] */ const Skylark::BKENG_RESULT* pResult,
    /* [in ] */ void*               pvFlag)
{
    if ( sizeof(Skylark::BKENG_RESULT) != pResult->uSize )
    {
        _ASSERT(0);
    }

    size_t Num = m_FileResultArray.GetCount();
    size_t i   = 0;
    for (; i<Num; ++i )
    {
        if ( 0 == m_FileResultArray[i].FileName.CompareNoCase(lpszFilePath) )
        {
            m_FileResultArray[i].Status = COther::GetCleanResult( pResult->uCleanResult );
            break;
        }
    }
    return S_OK;
}


/**
* @brief        扫描过程中返回细节 (如壳信息,压缩包中的病毒等...)
* @param[in]    lpszVirtualPath 扫描对象的虚拟路径
* @param[in]    pResult         扫描对象的扫描结果
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/ 
HRESULT STDMETHODCALLTYPE CBkMultiFileScan::OnScanDetail(
    /* [in ] */ LPCWSTR             lpszVirtualPath,
    /* [in ] */ const Skylark::BKENG_RESULT* pResult,
    /* [in ] */ void*               pvFlag)
{
    if ( sizeof(Skylark::BKENG_RESULT) != pResult->uSize )
    {
        _ASSERT(0);
    }
    return S_OK;
}


/**
* @brief        空闲时候的回调
* @param[in]    lpszFilePath    当前正在扫描的文件 (如果当前没有正在扫描的文件,这里可能为NULL)
* @param[in]    pvFlag          提交文件时传入的回调令牌
* @return       返回E_FAIL或其他错误会导致扫描中断
*/ 
HRESULT STDMETHODCALLTYPE CBkMultiFileScan::OnIdle( )
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CBkMultiFileScan::OnFinish(
    HRESULT             hrFinishCode)
{
    // 减少工作集大小,任务管理器中看到的内存占用将减小
    ::SetProcessWorkingSetSize(::GetCurrentProcess(), -1, -1);

    return S_OK;
}

VOID CBkMultiFileScan::_Reset( const CAtlArray<CString>& FilePathArray )
{
    m_FileResultArray.RemoveAll();
    m_unKnownIndex.RemoveAll();
    size_t Num = FilePathArray.GetCount();
    size_t i = 0;
    for ( i; i<Num; ++i )
    {
        BK_FILE_RESULT		       FileResult;
        InitFileResult( FileResult );
        Skylark::CWinPath FilePath = FilePathArray[i];
        FilePath.ExpandNormalizedPathName();
        FileResult.FileName = FilePath.m_strPath;
        m_FileResultArray.Add( FileResult );
    }
} 



HRESULT  CBkFileScan::Initialize( DWORD CliendPid )
{
    // 检测引擎是否加载
    DWORD dwRet = WaitForSingleObject( g_hThread, INFINITE );
    if ( WAIT_FAILED == dwRet )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
    if ( !g_bInitFalg )
    {
        //引擎初始化失败
        return E_FAIL;
    }

    HRESULT hr = E_FAIL;

    hr = m_EngineLiteEx.Initialize();
    if ( FAILED(hr) )
    {
        return hr;
    }

    m_BackupVirusFile.Initialize();
    m_ScanLogWriter.Initialize();
    return  hr;
}


HRESULT  CBkFileScan::Uninitialize()
{
    m_ScanLogWriter.Uninitialize();
    m_BackupVirusFile.Uninitialize();
    m_EngineLiteEx.Uninitialize();

    return S_OK;
}


HRESULT  CBkFileScan::Scan(
                           CString& FilePath,
                           DWORD            dwScanMask,
                           BK_FILE_RESULT&   ScanResult
                           )
{
    HRESULT hr = E_FAIL;

    if ( CGlobalWhiteList::Instance().Lookup( FilePath ) )
    {
        CGlobalWhiteList::Instance().BuildScanResult( ScanResult, FilePath );
        return S_OK;
    }

    hr = m_EngineLiteEx.SetScanSetting( m_Setting );
    if ( FAILED(hr) )
    {
        return hr;
    }
    Skylark::BKENG_RESULT KengResult;
    hr = m_EngineLiteEx.Scan( FilePath, dwScanMask, &KengResult, this, NULL );
    if ( FAILED(hr) )
    {
        return hr;
    }

    InitFileResult( ScanResult );
    ScanResult.SecLevel = KengResult.uSecLevel;
    ScanResult.FileName = FilePath; 
    if ( m_Setting.bAutoClean )
    {
        ScanResult.Status	 = COther::GetCleanResult( KengResult.uCleanResult );
    }
    else
    {
        ScanResult.Status	 = COther::GetScanResult( KengResult.uScanResult );
    }

    CMD5::GetStrMd5( KengResult.byMD5, ScanResult.MD5 );
    ScanResult.CRC = KengResult.uCRC32;

    if ( KengResult.bIsVirus )
    {
        ScanResult.Type      = BkQueryInfoFileTypeVirus;
        ScanResult.VirusName = KengResult.szVirusName;
        ScanResult.VirusType = COther::GetVirusType( KengResult.uScanResult );
    }
    else
    {
        ScanResult.Type      = BkQueryInfoFileTypeSafe;
    }

    if ( KengResult.bNeedUpload )
    {
        ScanResult.Type      = BkQueryInfoFileTypeUnknown;
    }

    return hr;
}


HRESULT	 CBkFileScan::ForceClean( CString& FilePath, BK_FORCE_CLEAN_DETAIL& detail, BK_FILE_RESULT& ScanResult )
{
	//ATLASSERT( Skylark::SLLevelIsBlackKnown( detail.dwSecLevel ) );

	HRESULT hr = E_FAIL;
	if (!CWinPathApi::IsFileExisting( FilePath ))
	{   // 如果文件不存在,这里返回已删除
		ScanResult.Status = BkFileStateDeleteSuccess;
		return S_OK;
	}

	hr = m_EngineLiteEx.SetScanSetting( m_Setting );
	if ( FAILED(hr) )
	{
		return hr;
	}


	///> 清除前的处理
	if ( FILE_TRACK_REPORT_NONPE == detail.dwTrack 
		&& detail.bNeedUpload )
	{// 非PE文件先复制一份到recycle目录，再行上传
		ReportFile( FilePath, FILE_TRACK_REPORT_NONPE, 0 );
	}


	m_ubackid = 0;


	Skylark::BKENG_RESULT EngScanResult;
	BKENG_INIT( &EngScanResult );
	EngScanResult.bIsVirus;
	EngScanResult.uSecLevel = detail.dwSecLevel;
	CString strVirusName;
	TranslateVirusName( detail.strVirusName, strVirusName );
	if ( !strVirusName.IsEmpty() )
	{
		StrCpyNW( EngScanResult.szVirusName, strVirusName, _countof( EngScanResult.szVirusName ) );
		EngScanResult.szVirusName[_countof( EngScanResult.szVirusName ) - 1] = L'\0';
	}
	EngScanResult.uScanResult = BKENG_SCAN_RESULT_VIRUS_NEED_DELETE;
	

	Skylark::BKENG_RESULT EngCleanResult;
	BKENG_INIT( &EngCleanResult );
    DWORD dwScanMask = 
        BKENG_SCAN_MASK_USE_LOCAL_HEURIST            |
        BKENG_SCAN_MASK_USE_CLOUD_HEURIST_LOW_RISK   |
        BKENG_SCAN_MASK_USE_CLOUD_HEURIST_HIGH_RISK  |
        BKENG_SCAN_MASK_FROM_AUTORUNS;
	hr = m_EngineLiteEx.HintClean( FilePath, dwScanMask, &EngScanResult, &EngCleanResult, this, NULL );
	if ( FAILED( hr ) )
	{
		return hr;
	}

	ScanResult.FileName = FilePath;
	ScanResult.Track	= detail.dwTrack;
	TransformResult( EngCleanResult, ScanResult );


	///> 清除后的处理 
	if ( FILE_TRACK_QUARANTINE & detail.dwTrack
		&& detail.bNeedUpload )
	{// 从隔离区上报的，这里添加上传
		ReportFile( FilePath, FILE_TRACK_QUARANTINE, m_ubackid );
	}


	m_ubackid = 0;

	return hr;
}



HRESULT  CBkFileScan::GetScanSetting( BK_SCAN_SETTING& Setting )
{
    Setting = m_Setting;
    return S_OK;
}



HRESULT  CBkFileScan::SetScanSetting(const BK_SCAN_SETTING& Setting )
{
    if ( (Setting.CleanFailedOp <= CleanFailedNull) 
        || (CleanFailedNum <= Setting.CleanFailedOp)
        || (Setting.nScanFileType <=  BkScanFileTypeNull)
        || (BkScanFileTypeNum <= Setting.nScanFileType)
        || Setting.dwSize != sizeof(Setting) )
    {
        return E_INVALIDARG;
    }

    m_Setting = Setting;
    return S_OK;
}

HRESULT CBkFileScan::ReportFile( 
	CString& FilePath, 
	DWORD  dwTrack,
	UINT64 ubackid
	)
{
	if ( FilePath.IsEmpty() )
	{
		return E_INVALIDARG;
	}

	switch ( dwTrack )
	{
	case FILE_TRACK_REPORT_NONPE:
		{
			CString strRefer;
			WinMod::CWinPath pathReport;
			HRESULT hr = GenerateRecycleFilePath( FilePath, pathReport.m_strPath );

			::CopyFile( FilePath, pathReport.m_strPath, TRUE );

			if ( pathReport.IsExisting() )
			{
				CUnkownReport::Instance().AddUnkownFile( 
					pathReport.m_strPath, 
					dwTrack 
					);
			}
			else
			{
				return E_FAIL;
			}
		}
		break;
	case FILE_TRACK_QUARANTINE:
		{
			if ( 0 == ubackid )
			{
				return E_INVALIDARG;
			}

			BOOL bAutoReport = FALSE;
			HRESULT hr = CSvcSetting::Instance().GetAutoReport( bAutoReport );
			if ( FAILED( hr ) )
			{
				bAutoReport = FALSE;
			}

			if ( bAutoReport )
			{
				CString strId;
				strId.Format(L"%I64u", ubackid );
				CUnkownReport::Instance().AddUnkownFile( 
					strId, 
					dwTrack 
					);
			}
		}
		break;
	default:
		return E_FAIL;
		break;
	}

	return S_OK;
}

BOOL CBkFileScan::TranslateVirusName( LPCWSTR lpVirusName, CString& strTranslateVirusName )
{
	CString strVirusName = lpVirusName;
	if ( 0 == strVirusName.CompareNoCase( _T("{Risk_Lnk_Exec}") ) )
	{
		strTranslateVirusName = L"heur:trojan.lnk.exploit";
		return TRUE;
	}

	strTranslateVirusName = lpVirusName;
	return TRUE;
}

HRESULT CBkFileScan::GenerateRecycleFilePath( LPCWSTR lpszRecycleFile, CString& strRecycleFilePath )
{
	if (!lpszRecycleFile || !*lpszRecycleFile)
		return E_INVALIDARG;

	WinMod::CWinPath pathTempFile;

	// 获取recycle目录
	HRESULT hr = Skylark::CSkylarkPath::GetRecyclePath(lpszRecycleFile, pathTempFile.m_strPath);
	if (FAILED(hr))
		return hr;



	// 生成guid字符串
	GUID guid;
	hr = ::CoCreateGuid(&guid);
	if (FAILED(hr))
		return hr;


	CString strGuid;

	CString::XCHAR szFormat[] = {'%', '0', '2', 'x', '\0'};

	const BYTE *buf = (const BYTE*)&guid;
	DWORD dwBufLen = sizeof(guid);
	strGuid.Empty();
	for (DWORD i = 0; i < dwBufLen; ++i)
	{
		strGuid.AppendFormat(szFormat, buf[i]);
	}


	// 生成回收文件名
	CString strFileSpec = WinMod::CWinPathApi::FindFileName(lpszRecycleFile);


	// 截断文件名
	if (strFileSpec.GetLength() > 16)
		strFileSpec.Truncate(16);


	strFileSpec.Append(strGuid);


	// 生成回收文件路径
	pathTempFile.Append(strFileSpec);

	strRecycleFilePath = pathTempFile.m_strPath;
	return S_OK;
}

void CBkFileScan::TransformResult(const Skylark::BKENG_RESULT& bkResult, BK_FILE_RESULT& FileResult)
{
	FileResult.bHashed	= bkResult.bHashed;
	FileResult.CRC		= bkResult.uCRC32;
	FileResult.MD5.Format(
		L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		bkResult.byMD5[0],  bkResult.byMD5[1],  bkResult.byMD5[2],  bkResult.byMD5[3], 
		bkResult.byMD5[4],  bkResult.byMD5[5],  bkResult.byMD5[6],  bkResult.byMD5[7], 
		bkResult.byMD5[8],  bkResult.byMD5[9],  bkResult.byMD5[10], bkResult.byMD5[11], 
		bkResult.byMD5[12], bkResult.byMD5[13], bkResult.byMD5[14], bkResult.byMD5[15]);

	FileResult.SecLevel  = bkResult.uSecLevel;
	FileResult.Status	 = COther::GetCleanResult( bkResult.uCleanResult );
	FileResult.Type		 = COther::GetScanResult( bkResult.uScanResult );
	FileResult.VirusType = COther::GetVirusType( bkResult.uScanResult );
	FileResult.VirusName = bkResult.szVirusName;
}