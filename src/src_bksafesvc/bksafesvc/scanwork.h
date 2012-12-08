#pragma once
#define USE_OPENSSL_H
#include "bkengex.h"
#include "bklogex.h"
#include "backupex.h"
#include "svccallback.h"
#include "scancontext.h"
#include "common/whitelist.h"

class CScanWork :
    public Skylark::IBKEngineCallback,
    public IEnumOperation
{
public:
    CScanWork();
    ~CScanWork();


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
    * @brief    初始化扫描逻辑实例
    */
    HRESULT Initialize( DWORD CliendPid, IScanFileCallBack* pSvcCallback );

    /**
    * @brief    反初始化扫描逻辑实例
    */
    HRESULT Uninitialize();

    /**
    *@brief     开始查杀
    *@param     ScanMod 为扫描类型，（快速、关键位置查杀、全盘、自定义等）
    *@param     CustomPath  附加参数，例如可供自定义扫描存放自定义目录用
    */
    HRESULT Scan(
        /* [in ] */int ScanMod,
        /* [in ] */BOOL bBackGround,
        /* [in ] */const CAtlArray<CString>& CustomParam );

    HRESULT Scan2( 
        int ScanMod, 
        BOOL bBackGround, 
        const CAtlArray<BK_SCAN_ADDITIONAL_PATH>& CustomParam 
        );
    /**
    *@brief     暂停查杀
    */
    HRESULT  Pause(
        );

    /**
    *@brief     暂停查杀
    */
    HRESULT Resume(
        );

    /**
    *@brief     停止查杀
    */
    HRESULT Stop(
        );


    HRESULT QueryScanStep( int ScanMode, CAtlArray<int>& ScanStep );
    /**
    *@brief     查询查杀信息
    *@param     ScanInfo 返回查杀信息
    */
    HRESULT QueryScanInfo(
        /* [out ] */BK_SCAN_INFO& ScanInfo 
        );

    /**
    *@brief		查询清除信息
    *@param     CleanInfo 返回清除信息
    */
    HRESULT QueryCleanInfo(
        /* [out ] */BK_CLEAN_INFO& CleanInfo
        );

    /**
    *@brief		查询文件信息
    *@param     Type     文件类型（未知、病毒）
    *@param     Index    文件索引
    *@param     FileInfo 返回文件信息
    */
    HRESULT QueryFileInfo(
        /* [in  ] */int       Type,
        /* [in  ] */DWORD       Index,
        /* [out ] */BK_FILE_INFO& FileInfo
        );

    /**
    *@brief		清除文件
    *@param     IndexArr	病毒类型文件索引数组
    */
    HRESULT Clean(
        /* [in  ] */const  CAtlArray<DWORD>& fileIndex
        );


    /**
    *@brief		查询上次扫描信息
    *@param		ScanInfo    查杀信息
    */
    HRESULT  QueryLastScanInfo(
        /* [out ] */BK_SCAN_INFO&	ScanInfo
        );

    /**
    *@brief		设置扫描设置
    *@param		
    */
    HRESULT  SetScanSetting(
        /* [in  ] */const BK_SCAN_SETTING&  Setting
        );


    /**
    *@brief		获取扫描选项
    *@param		Setting 扫描选项
    */
    HRESULT  GetScanSetting(
        /* [out ] */BK_SCAN_SETTING&  Setting 
        );
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////


protected:
    ////////////////////////////////////////////////////////////////////////
    /**
    * @brief        扫描文件前调用
    * @param[in]    lpszFilePath    原始文件路径
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */
    virtual HRESULT STDMETHODCALLTYPE BeforeScanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ void*               pvFlag);


    /**
    * @brief        扫描文件后调用
    * @param[in]    lpszFilePath    原始文件路径
    * @param[in]    pResult         当前文件的扫描结果
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */
    virtual HRESULT STDMETHODCALLTYPE AfterScanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag);


    /**
    * @brief        清除文件前调用 (禁用自动清除时,不会调用此回调)
    * @param[in]    lpszFilePath    原始文件路径
    * @param[in]    pResult         当前文件的扫描结果
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */
    virtual HRESULT STDMETHODCALLTYPE BeforeCleanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag);


    /**
    * @brief        清除文件后调用 (禁用自动清除时,不会调用此回调)
    * @param[in]    lpszFilePath    原始文件路径
    * @param[in]    pResult         当前文件的清除结果
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */
    virtual HRESULT STDMETHODCALLTYPE AfterCleanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag);


    /**
    * @brief        扫描过程中返回细节 (如壳信息,压缩包中的病毒等...)
    * @param[in]    lpszVirtualPath 扫描对象的虚拟路径
    * @param[in]    pResult         扫描对象的扫描结果
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */ 
    virtual HRESULT STDMETHODCALLTYPE OnScanDetail(
        /* [in ] */ LPCWSTR             lpszVirtualPath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag);



    virtual HRESULT STDMETHODCALLTYPE OnIdle();


    /**
    * @brief        结束时回调
    * @param[in]    hrFinishCode    扫描返回的错误码
    */ 
    virtual HRESULT STDMETHODCALLTYPE OnFinish(
        HRESULT             hrFinishCode);

    //枚举回调
    virtual HRESULT OperateFile( const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask, void*   pvFlag,  PVOID pParam );
    virtual BOOL    IsExitEnumWork();

    ////////////////////////////////////////////////////////////////////////


private:

    static unsigned int __stdcall EnumScanFileProc( void * lpParameter )
    {
        _ASSERT( NULL != lpParameter );

        CScanWork * pThis = static_cast<CScanWork*>(lpParameter);
        pThis->_EnumFileWork();

        return 0;
    }

    static unsigned int __stdcall CleanVirusFileProc( void * lpParameter )
    {
        _ASSERT( NULL != lpParameter );

        CScanWork * pThis = static_cast<CScanWork*>(lpParameter);
        pThis->_CleanWork();

        return 0;
    }

    //数据初始设置
    HRESULT     _ResetForScan();
    HRESULT     _ResetForClean();

    //扫描 清除
    HRESULT     _ScanWork();
    HRESULT     _CleanWork();


    //扫描时，文件枚举
    DWORD       _EnumFileWork(); 

    //退出clean线程。
    VOID        _ExitCleanVirusThread();
    //退出枚举
    VOID        _ExitEnumFileThread();
    //停止扫描
    VOID        _StopScan();
    //回复暂停
    HRESULT        _Resume();
    //设置扫描时间
    VOID        _SetScanTime();
    //设置扫描进度
    VOID        _SetScanProgress();

    VOID        _SaveLastScanInfo();

    //回调处理
    BOOL        _IsFindFile( CAtlArray<BK_FILE_RESULT_EX>& FileArray, LPCWSTR pszFilePath );
    VOID        _UpdateScanResult( LPCWSTR  lpszFilePath, const Skylark::BKENG_RESULT* pResult, void* pvFlag );
    VOID        _UpdateScanInfo( LPCWSTR  lpszFilePath, const Skylark::BKENG_RESULT* pResult, void* pvFlag );
    VOID        _UpdateCleanResult( LPCWSTR  lpszFilePath, const Skylark::BKENG_RESULT* pResult, void* pvFlag );
    VOID        _UpdateCleanInfo( LPCWSTR  lpszFilePath, const Skylark::BKENG_RESULT* pResult, void* pvFlag, UINT32 uOldCleanResult = BKENG_CLEAN_RESULT_NO_OP);

    //记录扫描日志
    VOID        _RecordScanLog();

private:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

    ///////
    CObjLock    m_LockData;
    CAtlArray<BK_FILE_RESULT_EX>   m_VirusFileArray;
    CAtlArray<BK_FILE_RESULT_EX>   m_UnknownFileArray;
    BK_CLEAN_INFO				   m_CleanInfo;
    /////// 
    CObjLock                    m_LockOther;
    BOOL				        m_bScanEnableEntry;
    ///////
    //进度
    BOOL                        m_bCommitFinish;
    CScanProgress2              m_ScanProgress;
    //扫描步骤
    CScanStep                   m_ScanStep;
    //扫描时间
    DWORD                       m_StartTimeForScanTime;
    DWORD                       m_BeginPauseTime;
    DWORD                       m_PauseTime;
    //扫描模式
    DWORD                       m_ScanMode;

    //清除索引
    CAtlArray<DWORD>            m_CleanIndexArray;
    //设置
    BK_SCAN_SETTING			    m_Setting;

    //自定义参数
    CAtlArray<CString>          m_CustomParam;
    CAtlArray<BK_SCAN_ADDITIONAL_PATH>  m_CustomParam2;
    BOOL                        m_bScan2;
    //clean 判断
    BOOL                        m_bClean;

    //枚举线程
    HANDLE				        m_hEnumFileThread;
    CEvent				        m_ExitEnumFileEvent;

    //扫描引擎
    CBkScanEngEx                m_ScanEngEx;

    //扫描上下文
    CScanContext                m_ScanContext;

    //clean病毒引擎
    CEngineLiteEx               m_CleanVirusOperation;
    //clean线程
    HANDLE                      m_hCleanThread;
    CEvent                      m_ExitCleanEvent;

    //记录日志接口
    CBkScanLog                  m_ScanLogWriter;

    //隔离病毒接口
    CBackupVirusFile            m_BackupVirusFile;

    //服务回调
    IScanFileCallBack*               m_pSvcCallback;
    CAtlMap<CString, int>       m_whiteMap;
};





class CBkMultiFileScan :
    public Skylark::IBKEngineCallback
{
public:
    CBkMultiFileScan();
    ~CBkMultiFileScan();


    HRESULT  Initialize(
        );


    HRESULT  Uninitialize(
        );

    HRESULT Stop();

    HRESULT  Scan( 
        const CAtlArray<CString>& FilePathArray,
        CAtlArray<BK_FILE_RESULT>& FileResultArray
        );

    HRESULT Scan2(
        const CAtlArray<CString>& FilePathArray,
        CAtlArray<BK_FILE_RESULT>& FileResultArray,
        CAtlArray<size_t>& unKnownIndex
        );

    HRESULT ScanHash( 
        const CAtlArray<CString>& hashArray, 
        CAtlArray<BK_FILE_RESULT>& frArray 
        );

    HRESULT  SetScanSetting( 
        /* [in ] */const BK_SCAN_SETTING& Setting
        );


    HRESULT  GetScanSetting(
        /* [out ] */BK_SCAN_SETTING& Setting 
        );

    ///////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    /**
    * @brief        扫描文件前调用
    * @param[in]    lpszFilePath    原始文件路径
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */
    virtual HRESULT STDMETHODCALLTYPE BeforeScanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ void*               pvFlag);


    /**
    * @brief        扫描文件后调用
    * @param[in]    lpszFilePath    原始文件路径
    * @param[in]    pResult         当前文件的扫描结果
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */
    virtual HRESULT STDMETHODCALLTYPE AfterScanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag);


    /**
    * @brief        清除文件前调用 (禁用自动清除时,不会调用此回调)
    * @param[in]    lpszFilePath    原始文件路径
    * @param[in]    pResult         当前文件的扫描结果
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */
    virtual HRESULT STDMETHODCALLTYPE BeforeCleanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag);


    /**
    * @brief        清除文件后调用 (禁用自动清除时,不会调用此回调)
    * @param[in]    lpszFilePath    原始文件路径
    * @param[in]    pResult         当前文件的清除结果
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */
    virtual HRESULT STDMETHODCALLTYPE AfterCleanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag);


    /**
    * @brief        扫描过程中返回细节 (如壳信息,压缩包中的病毒等...)
    * @param[in]    lpszVirtualPath 扫描对象的虚拟路径
    * @param[in]    pResult         扫描对象的扫描结果
    * @param[in]    pvFlag          提交文件时传入的回调令牌
    * @return       返回E_FAIL或其他错误会导致扫描中断
    */ 
    virtual HRESULT STDMETHODCALLTYPE OnScanDetail(
        /* [in ] */ LPCWSTR             lpszVirtualPath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag);



    virtual HRESULT STDMETHODCALLTYPE OnIdle();


    /**
    * @brief        结束时回调
    * @param[in]    hrFinishCode    扫描返回的错误码
    */ 
    virtual HRESULT STDMETHODCALLTYPE OnFinish(
        HRESULT             hrFinishCode);
    ////////////////////////////////////////////////////////////////////////

private:
    VOID _Reset( const CAtlArray<CString>& FilePathArray );

    BK_SCAN_SETTING			    m_Setting;
    CAtlArray<BK_FILE_RESULT>   m_FileResultArray;
    CAtlArray<size_t>           m_unKnownIndex;
    //扫描引擎
    CBkScanEngEx                m_ScanEngEx;
    CEngineLiteEx               m_EngineLiteEx;
    //隔离病毒接口
    CBackupVirusFile            m_BackupVirusFile;
    //记录日志接口
    CBkScanLog                  m_ScanLogWriter;
};


class CBkFileScan 
    : public Skylark::IBKEngineCallback
{
public:
    CBkFileScan()
    {
        m_Setting.Reset();
		m_ubackid = 0;
    }
    ~CBkFileScan(){};


    HRESULT  Initialize( DWORD CliendPid );


    HRESULT  Uninitialize();


    HRESULT  GetScanSetting( BK_SCAN_SETTING& Setting );


    HRESULT  SetScanSetting(const BK_SCAN_SETTING& Setting );


    HRESULT  Scan(
        CString&        FilePath,
        DWORD           dwScanMask,
        BK_FILE_RESULT& ScanResult
        );

	HRESULT  ForceClean(
		CString& FilePath,
		BK_FORCE_CLEAN_DETAIL& detail,
		BK_FILE_RESULT&   ScanResult
		);

public:

    virtual HRESULT STDMETHODCALLTYPE BeforeScanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ void*               pvFlag)
    {
        return S_OK;
    }



    virtual HRESULT STDMETHODCALLTYPE AfterScanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag)
    {
        if ( pResult->bIsVirus 
            && (FALSE == m_Setting.bAutoClean) )
        {
            m_ScanLogWriter.WriteResultAsLog( lpszFilePath, pResult );
            m_ScanLogWriter.SaveToFile();
        }
        return S_OK;
    }



    virtual HRESULT STDMETHODCALLTYPE BeforeCleanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag)
    {
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
                m_BackupVirusFile.BackupFile( lpszFilePath, pResult, m_ubackid, pvFlag );
            }
        }

        return S_OK;
    }



    virtual HRESULT STDMETHODCALLTYPE AfterCleanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag)
    {
        return S_OK;
    }



    virtual HRESULT STDMETHODCALLTYPE OnScanDetail(
        /* [in ] */ LPCWSTR             lpszVirtualPath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag)
    {
        return S_OK;
    }



    virtual HRESULT STDMETHODCALLTYPE OnIdle()
    {
        return S_OK;
    }



    virtual HRESULT STDMETHODCALLTYPE OnFinish(
        HRESULT             hrFinishCode)
    {   
        return S_OK;
    }
    ////////////////////////////////////////////////////////////////////////
private:
    HRESULT _SetBKEngSetting( const BK_SCAN_SETTING& Setting );

	HRESULT ReportFile( 
		CString& FilePath, 
		DWORD  dwTrack,
		UINT64 ubackid
		);

	BOOL TranslateVirusName( LPCWSTR lpVirusName, CString& strTranslateVirusName );
	void TransformResult(const Skylark::BKENG_RESULT& bkResult, BK_FILE_RESULT& FileResult);
	HRESULT GenerateRecycleFilePath( LPCWSTR lpszRecycleFile, CString& strRecycleFilePath );

    CEngineLiteEx               m_EngineLiteEx;
    BK_SCAN_SETTING             m_Setting;

    //隔离病毒接口
    CBackupVirusFile            m_BackupVirusFile;
    //记录日志接口
    CBkScanLog                  m_ScanLogWriter;
	UINT64						m_ubackid;
};

