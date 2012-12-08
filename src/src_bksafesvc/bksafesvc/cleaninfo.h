#pragma  once

#include <math.h>
#include <float.h>
#include <algorithm>
#include "skylark2\bkatrun.h"


struct UNTERMINAL_ENTRY
{
    WCHAR * pPath;
    WCHAR * pFileName;
};
class CCleanOperation
{
public:

    static UNTERMINAL_ENTRY UnterminalFileTable[];
public:
    CCleanOperation():m_piAutorunScanner(NULL)
    {
 
    };
    ~CCleanOperation()
    {
        Reset();
    }

    void    Reset()
    {
        _RemoveAllProcessNamePid();
        //autorun
        _RemoveAllModuleAutorunItem();
        if ( NULL != m_piAutorunScanner )
        {
            m_piAutorunScanner->Release();
            m_piAutorunScanner = NULL;
        }
    }
    HRESULT RecordCleanDataForMemory( const CString& FilePath, DWORD Pid );
    HRESULT RecordCleanDataForAutorun( const CString& FilePath, PVOID pParam );
    VOID    OperateMemoryClean( const CString& FilePath );
    VOID    OperateAutoClean( const CString& FilePath );
    
private:
    //内存扫描，进程处理。
    BOOL        _IsOperateForClean( const CString& FilePath );
    VOID        _RemoveAllProcessNamePid();
    BOOL	    _IsProcessMainModule( const CString& StrProcessName, CAtlArray<DWORD>& PidArray );
    VOID        _RemoveProcessNamePid( const CString& StrProcessName );
    //注册表扫描，Autorun处理
    VOID        _RemoveAllModuleAutorunItem();
    BOOL        _IsAutorunModule( const CString& StrModule, CAtlArray<Skylark::IBKAutorunItem*>& AutorunItemArray );
    VOID        _RemoveModuleAutorunItem( const CString& StrModule );


private:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;
    CObjLock    m_LockForProcess;
    CObjLock    m_LockForAutorun;
    //内存主模块表
    typedef CAtlArray<DWORD>					CAtlPidArray;
    typedef CAtlMap<CString, CAtlPidArray*>		CAtlProcessNamePidMap;
    CAtlProcessNamePidMap						m_ProcessNamePidMap;
    //Autorun模块列表
    typedef CAtlArray<Skylark::IBKAutorunItem*>                    CAtlAutorunItemArray;
    typedef CAtlMap<CString, CAtlAutorunItemArray*>                   CAtlModuleNameAutorunItemMap;
    CAtlModuleNameAutorunItemMap                    m_ModuleNameAutorunItemMap;
    Skylark::IBKAutorunScanner*                     m_piAutorunScanner;
};


class CScanStep
{
public:
    CScanStep()
        : m_MemFileNum(0)
        , m_AutoRunFileNum(0)
        , m_CriticalDirFileNum(0)
        , m_AllDiskFileNum(0)
        , m_CustomDirFileNum(0)
        , m_MoveDisFileNum(0)    
    {

    }
    ~CScanStep(){}
public:
    void Reset()
    {
        CObjGuard AutoLock(m_Lock);
        m_MemFileNum        = 0;
        m_AutoRunFileNum        = 0;
        m_CriticalDirFileNum        = 0;
        m_AllDiskFileNum        = 0;
        m_CustomDirFileNum        = 0;
        m_MoveDisFileNum        = 0;
    }
    void IncScanFileNum( int ScanMode, int ScanStep );
    void DecScanFileNum( int ScanMode, int ScanStep );

    HRESULT  QueryScanStep( int ScanMode, CAtlArray<int>& ScanStepArray );

    int GetScanStep( int ScanMode, DWORD ScanFileNum, int SrcScanStep );

    DWORD GetTotalFileNum()
    {
        return (m_MemFileNum + m_AutoRunFileNum + m_CriticalDirFileNum + m_AllDiskFileNum + m_CustomDirFileNum + m_MoveDisFileNum);
    }
private:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;
    CObjLock    m_Lock;

    DWORD m_MemFileNum;
    DWORD m_AutoRunFileNum;
    DWORD m_CriticalDirFileNum;
    DWORD m_AllDiskFileNum;
    DWORD m_CustomDirFileNum;
    DWORD m_MoveDisFileNum;
};

class CScanProgress
{
public:
    CScanProgress()
        :m_LastScanFileNum(0)
        ,m_OnePercentFileNum(0)
        ,m_LastPercent(0)
        ,m_bEntry(FALSE)
    {}
    ~CScanProgress(){};

    void Reset()
    {
        m_LastScanFileNum = 0;
        m_OnePercentFileNum = 0;
        m_LastPercent = 0;
        m_bEntry = 0;
    }
    //DWORD GetScanProcess( DWORD CleanFileNum, DWORD TotalFileNum )
    //{
    //     return (CleanFileNum * 100) / TotalFileNum;
    //}
    DWORD GetScanProgress( int ScanMode, DWORD ScanFileNum, DWORD TotalFileNum, DWORD LastProgress, BOOL bEnumFinished)
    {
        DWORD Progress = 0;
        if ( bEnumFinished )
        {
            if ( !m_bEntry )
            {   
                DWORD NoScanFileNum = TotalFileNum - ScanFileNum;
                DWORD NoScanPercent = 100 - LastProgress;
                m_OnePercentFileNum = NoScanFileNum / NoScanPercent;
                m_LastScanFileNum = ScanFileNum;
                m_LastPercent = LastProgress;
                m_bEntry = TRUE;
            }
            if ( (0 <= m_OnePercentFileNum && m_OnePercentFileNum <= 5)
                || (LastProgress > 95) )
            {
                //m_OnePercentFileNum 太小了，m_LastPercent + (ScanFileNum-m_LastScanFileNum)/m_OnePercentFileNum; 可能会超出 100。如：m_OnePercentFileNum 为 1。
                //m_OnePercentFileNum 为 0，肯定也不行。
                //LastProgress 大于 95 时，要用真实数据。因为，最后有可能Progress已经100了，但还有文件没有扫玩，这个是ScanFileNum-m_LastScanFileNum)/m_OnePercentFileNum的余数原因造成的。
                Progress =  (ScanFileNum * 100) / TotalFileNum;
            }
            else
            {
                Progress = m_LastPercent + (ScanFileNum-m_LastScanFileNum)/m_OnePercentFileNum;
            }
        }
        else
        {
            if ( BkScanModeFullScan == ScanMode )
            {
                Progress = (DWORD)(ScanFileNum / 2000);
                if ( Progress > 50 )
                {
                    Progress = 50;
                }
            }
            else if ( BkScanModeCheckScan == ScanMode )
            {
                //体检扫描进度
                //体检扫描大体文件个数个机器差别不太大，这里默认1000.
                //模拟最高百分比 90%
                Progress =  (DWORD)(ScanFileNum / 10);
                if ( Progress > 90 )
                {
                    Progress = 90;
                }
            }
            else
            {
                Progress = (DWORD)(ScanFileNum / 150);
                if ( Progress > 80 )
                {
                    Progress = 80;
                }
            }

        }

        return Progress;
    }
private:
    DWORD                       m_LastScanFileNum;
    DWORD                       m_OnePercentFileNum;
    DWORD                       m_LastPercent;
    BOOL                        m_bEntry;
};



class CScanProgress2Hint
{
public:
    CScanProgress2Hint()
    {

    }

    ~CScanProgress2Hint()
    {

    }

    void Reset()
    {
        m_bAlreadyRead  = FALSE;
        m_bAlreadyWrite = FALSE;
        m_dwHintCount   = 0;
    }

    DWORD GetHintCount(LPCWSTR lpszScanMode, DWORD dwMin, DWORD dwMax);
    void  SetHintCount(LPCWSTR lpszScanMode, DWORD dwCount, DWORD dwMin, DWORD dwMax);

    LPCWSTR GetScanIniFile() {return L"ksafesv.scan.ini";}
    LPCWSTR GetScanAppName() {return L"EnumHint";}

    BOOLEAN m_bAlreadyRead;
    BOOLEAN m_bAlreadyWrite;
    DWORD   m_dwHintCount;
};


class CScanProgress2
{
public:
    CScanProgress2();
    ~CScanProgress2();

    void Reset();



    // 线性插值
    // xSample 和 ySample 都必须是升序
    double LinearProgress(
        double xVal,
        double xSamples[],
        double ySamples[],
        size_t nSampleCount,
        double yLowerBound = 0,
        double yUpperBound = DBL_MAX);



    DWORD GetScanProgress( int ScanMode, DWORD ScanFileNum, DWORD TotalFileNum, DWORD LastProgress, BOOL bEnumFinished);


    DWORD GetHintCountForFastScan();
    void  SetHintCountForFastScan(DWORD dwCount);



private:

    BOOL    m_bEnumFinished;                            ///< 是否已经完成枚举
    double  m_dVirtualProgress;                         ///< 当前虚拟扫描进度

    double  m_dLastVirtualProgress;                     ///< 最后一次虚拟扫描进度
    DWORD   m_dwScanFileCountForLastVirtualProgress;    ///< 最后一次虚拟扫描进度的对应文件个数

    CScanProgress2Hint  m_hintFastScan;         ///< 快速扫描提示
    CScanProgress2Hint  m_hintFullScan;         ///< 完全扫描提
};