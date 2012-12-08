#include "stdafx.h"
#include "common.h"
#include "cleaninfo.h"



UNTERMINAL_ENTRY CCleanOperation::UnterminalFileTable[] =
{
    {L"\\SystemRoot\\System32",             L"smss.exe"},
    {L"\\SystemRoot\\System32",             L"csrss.exe"},
    {L"\\SystemRoot\\System32",             L"lsass.exe"},
    {L"\\SystemRoot\\System32",             L"rpcss.exe "},
    {L"\\SystemRoot\\System32",             L"wininit.exe"},
    {L"\\SystemRoot\\System32",             L"winlogon.exe"},
    {L"\\SystemRoot\\System32",             L"services.exe"},
    {L"\\SystemRoot\\System32",             L"svchost.exe"},
    {L"\\SystemRoot\\System32",             L"wininit.exe"},
    {L"\\SystemRoot\\",                     L"explorer.exe"},

    {NULL,  NULL}
};

HRESULT CCleanOperation::RecordCleanDataForMemory( const CString& FilePath, DWORD Pid )
{ 
    if ( FALSE == WinMod::CWinPathApi::IsFileExisting(FilePath) )
    {
        return E_FAIL;
    }

    {
        CObjGuard	AutoLock( m_LockForProcess );

        CAtlProcessNamePidMap::CPair*  _pPare = m_ProcessNamePidMap.Lookup( FilePath );
        if ( NULL  == _pPare )
        {
            CAtlPidArray* _pPidArray = new CAtlPidArray;
            _pPidArray->Add( Pid );
            m_ProcessNamePidMap.SetAt( FilePath, _pPidArray );
        }
        else
        {
            _pPare->m_value->Add( Pid );
        }
    }

    return S_OK;
}

HRESULT CCleanOperation::RecordCleanDataForAutorun( const CString& FilePath, PVOID pParam )
{
    if ( NULL == pParam )
    {
        return E_FAIL;
    }

    CObjGuard AutoLock( m_LockForAutorun );

    if ( 0 == FilePath.CompareNoCase(L"IBKAutorunScanner") )
    {
        m_piAutorunScanner = (Skylark::IBKAutorunScanner*)pParam;
        m_piAutorunScanner->AddRef();
    }
    else
    {
        if ( FALSE == WinMod::CWinPathApi::IsFileExisting(FilePath) )
        {
            _ASSERT(0);
            return E_FAIL;
        }
        CAtlModuleNameAutorunItemMap::CPair*  _pPare = m_ModuleNameAutorunItemMap.Lookup( FilePath );
        if ( NULL  == _pPare )
        {
            CAtlAutorunItemArray* _pAutoItemArray = new CAtlAutorunItemArray;
            _pAutoItemArray->Add( (Skylark::IBKAutorunItem*)pParam );
            m_ModuleNameAutorunItemMap.SetAt( FilePath, _pAutoItemArray );
        }
        else
        {
            _pPare->m_value->Add( (Skylark::IBKAutorunItem*)pParam );
        }
        ((Skylark::IBKAutorunItem*)pParam)->AddRef();
    }

    return S_OK;
}




BOOL        CCleanOperation::_IsOperateForClean( const CString& FilePath )
{
    LPCWSTR pFileName = WinMod::CWinPathApi::FindFileName(FilePath);
    if ( NULL == pFileName )
    {
        _ASSERT(0);
        return FALSE;
    }
    int i = 0;
    for (; UnterminalFileTable[i].pFileName; ++i )
    {
        if ( 0 == _wcsicmp(UnterminalFileTable[i].pFileName, pFileName) )
        {
            Skylark::CWinPath UnterminalFilePath = UnterminalFileTable[i].pPath;
            UnterminalFilePath.Append( UnterminalFileTable[i].pFileName );
            UnterminalFilePath.ExpandNormalizedPathName();
            if ( 0 == FilePath.CompareNoCase( UnterminalFilePath.m_strPath) )
            {
                return FALSE;
            }
        }
    }
    //判断结束进程是否，是自己。
    Skylark::CWinPath CurModulePath;
    CurModulePath.GetModuleFileName(NULL);
    CurModulePath.ExpandNormalizedPathName();
    if ( 0 == FilePath.CompareNoCase(CurModulePath.m_strPath) )
    {
        return FALSE;
    }
    
    return TRUE;
}

VOID  CCleanOperation::_RemoveAllProcessNamePid()
{
    CObjGuard	AutoLock( m_LockForProcess );

    POSITION Pos = m_ProcessNamePidMap.GetStartPosition();
    if ( NULL == Pos )
    {
        return;
    }
    while ( NULL != Pos )
    {
        CAtlPidArray* _pPidArray = m_ProcessNamePidMap.GetNextValue( Pos );
        delete _pPidArray;
    }

    m_ProcessNamePidMap.RemoveAll();
}


//对进程名和进程ID,map的操作
VOID CCleanOperation::_RemoveProcessNamePid( const CString& StrProcessName )
{
    CObjGuard	AutoLock( m_LockForProcess );

    CAtlProcessNamePidMap::CPair*  _pPare = m_ProcessNamePidMap.Lookup( StrProcessName );
    if ( NULL != _pPare )
    {
        delete _pPare->m_value;
        m_ProcessNamePidMap.RemoveKey( StrProcessName );
    }
}


BOOL	CCleanOperation::_IsProcessMainModule( const CString& StrProcessName, CAtlArray<DWORD>& PidArray )
{
    CObjGuard	AutoLock( m_LockForProcess );
    CAtlProcessNamePidMap::CPair* pPare = m_ProcessNamePidMap.Lookup( StrProcessName );
    if ( NULL == pPare )
    {
        return FALSE;
    }
    if ( pPare->m_value->IsEmpty() )
    {
        return FALSE;
    }

    PidArray.Copy( *(pPare->m_value) );
    return TRUE;
}

VOID  CCleanOperation::OperateMemoryClean( const CString& FilePath )
{
    //这里不能对文件，是否存在进行判断，文件可能以前不存在了。如：被清理了
    if ( _IsOperateForClean(FilePath) )
    {
        CAtlArray<DWORD> PidArray;
        if ( FALSE == _IsProcessMainModule(FilePath, PidArray) )
        {
            return;
        }
        size_t Num = PidArray.GetCount();
        size_t i = 0;
        for (; i<Num; ++i )
        {
            HANDLE hProcess = ::OpenProcess( PROCESS_TERMINATE|SYNCHRONIZE, FALSE, PidArray[i] );
            if ( NULL == hProcess )
            {
                continue;
            }
            ::TerminateProcess( hProcess, 0 );
            ::WaitForSingleObject( hProcess, 500 );
            ::CloseHandle( hProcess );
        }
        _RemoveProcessNamePid( FilePath );
    }
}



VOID  CCleanOperation::OperateAutoClean( const CString& FilePath )
{
    //这里不能对文件，是否存在进行判断，文件可能以前不存在了。如：被清理了

    CAtlArray<Skylark::IBKAutorunItem*> AutorunItemArray;
    if ( FALSE == _IsAutorunModule(FilePath, AutorunItemArray) )
    {
        return;
    }
    size_t Num = AutorunItemArray.GetCount();
    size_t i = 0;
    for (; i<Num; ++i )
    {
        if ( AutorunItemArray[i]->CanBeRepaired() )
        {
            Skylark::BKATRUN_REPAIR_PARAM RepairParam;
            Skylark::BKENG_INIT( &RepairParam );
            RepairParam.pRepairFile = FilePath.GetString();
            AutorunItemArray[i]->Repair( &RepairParam );
            //BKDebugOutput( BKDBG_LVL_DEBUG, L"\n 修复 %s \n 位置 %s \n", RepairParam.pRepairFile, AutorunItemArray[i]->GetInformation() );
        }
    }
    _RemoveModuleAutorunItem( FilePath );
    
}


//注册表扫描，Autorun处理
VOID        CCleanOperation::_RemoveAllModuleAutorunItem()
{
    CObjGuard	AutoLock( m_LockForAutorun );

    POSITION Pos = m_ModuleNameAutorunItemMap.GetStartPosition();
    if ( NULL == Pos )
    {
        return;
    }

    while ( NULL != Pos )
    {
        CAtlAutorunItemArray* _pAutorunItemArray = m_ModuleNameAutorunItemMap.GetNextValue( Pos );
        size_t Num = _pAutorunItemArray->GetCount();
        size_t i = 0;
        for (; i < Num; ++i )
        {
            _pAutorunItemArray->GetAt(i)->Release();
        }
        _pAutorunItemArray->RemoveAll();
        delete _pAutorunItemArray;
    }

    m_ModuleNameAutorunItemMap.RemoveAll();
}

BOOL        CCleanOperation::_IsAutorunModule( const CString& StrModule, CAtlArray<Skylark::IBKAutorunItem*>& AutorunItemArray )
{
    CObjGuard	AutoLock( m_LockForAutorun );

    CAtlModuleNameAutorunItemMap::CPair* pPare = m_ModuleNameAutorunItemMap.Lookup( StrModule );
    if ( NULL == pPare )
    {
        return FALSE;
    }
    if ( pPare->m_value->IsEmpty() )
    {
        return FALSE;
    }

    AutorunItemArray.Copy( *(pPare->m_value) );
    return TRUE;
}

VOID        CCleanOperation::_RemoveModuleAutorunItem( const CString& StrModule )
{
    CObjGuard	AutoLock( m_LockForAutorun );

    CAtlModuleNameAutorunItemMap::CPair* pPare = m_ModuleNameAutorunItemMap.Lookup( StrModule );
    if ( NULL != pPare )
    {
        size_t Num = pPare->m_value->GetCount();
        size_t i = 0;
        for (; i < Num; ++i )
        {
            pPare->m_value->GetAt(i)->Release();
        }
        delete pPare->m_value;
        m_ModuleNameAutorunItemMap.RemoveKey( StrModule );
    }
}


//////////////////////////////////////////////////////////////////////////////

void CScanStep::IncScanFileNum( int ScanMode, int ScanStep )
{
    CObjGuard AutoLock(m_Lock);

    if ( BkScanRelationDirStepMemory == ScanStep 
        || BkScanRelationDirStepAutorun == ScanStep )
    {
        ScanStep = BkScanStepCriticalDir;
    }

    switch (ScanStep)
    {
    case (BkScanStepMemory):
        {
            m_MemFileNum += 1;
        }
        break;

    case (BkScanStepAutorun):
        {
            m_AutoRunFileNum += 1;
        }
        break;

    case (BkScanStepCriticalDir):
        {
            m_CriticalDirFileNum += 1;
        }
        break;

    case (BkScanStepCustomDir):
        {
            m_CustomDirFileNum += 1;
        }
        break;

    case (BkScanStepAllDisk):
        {
            m_AllDiskFileNum += 1;
        }
        break;

    case (BkScanStepMoveDisk):
        {
            m_MoveDisFileNum += 1;
        }
        break;

    default:
        {
            _ASSERT(0);
        }
        break;
    }
}


void CScanStep::DecScanFileNum( int ScanMode, int ScanStep )
{
    CObjGuard AutoLock(m_Lock);

    if ( BkScanRelationDirStepMemory == ScanStep 
        || BkScanRelationDirStepAutorun == ScanStep )
    {
        ScanStep = BkScanStepCriticalDir;
    }

    switch (ScanStep)
    {
    case (BkScanStepMemory):
        {
            m_MemFileNum -= 1;
        }
        break;

    case (BkScanStepAutorun):
        {
            m_AutoRunFileNum -= 1;
        }
        break;

    case (BkScanStepCriticalDir):
        {
            m_CriticalDirFileNum -= 1;
        }
        break;

    case (BkScanStepCustomDir):
        {
            m_CustomDirFileNum -= 1;
        }
        break;

    case (BkScanStepAllDisk):
        {
            m_AllDiskFileNum -= 1;
        }
        break;

    case (BkScanStepMoveDisk):
        {
            m_MoveDisFileNum -= 1;
        }
        break;

    default:
        {
            _ASSERT(0);
        }
        break;
    }
}


HRESULT  CScanStep::QueryScanStep( int ScanMode, CAtlArray<int>& ScanStepArray )
{
    if ( ScanMode <= BkScanModeNull
        || BkScanModeNum <= ScanMode )
    {
        return E_FAIL;
    }
    
    CObjGuard AutoLock(m_Lock);

    switch (ScanMode)
    {
    case (BkScanModeFastScan):
        {
            ScanStepArray.Add( BkScanStepMemory );
            ScanStepArray.Add( BkScanStepAutorun );
            ScanStepArray.Add( BkScanStepCriticalDir  );
        }
        break;

    case (BkScanModeIeFix):
    case (BkScanModeOnlyMemoryScan):
        {
            ScanStepArray.Add( BkScanStepMemory );
        }
        break;

    case (BkScanModeBootSystemScan):
    case (BkScanModeCheckScan):
        {
            ScanStepArray.Add( BkScanStepMemory );
            ScanStepArray.Add( BkScanStepAutorun );
        }
        break;

    case (BkScanModeCustomScan):
        {
            ScanStepArray.Add( BkScanStepMemory );
            ScanStepArray.Add( BkScanStepAutorun );
            ScanStepArray.Add( BkScanStepCriticalDir );
            ScanStepArray.Add( BkScanStepCustomDir );
        }
        break;

    case (BKScanModeMoveDiskScan):
        {
            ScanStepArray.Add( BkScanStepMemory );
            ScanStepArray.Add( BkScanStepAutorun );
            ScanStepArray.Add( BkScanStepMoveDisk  );
        }
        break;

    case (BkScanModeRightScan):
        {
            ScanStepArray.Add( BkScanStepMemory );
            ScanStepArray.Add( BkScanStepCustomDir );
        }
        break;

    case (BkScanModeFullScan):
        {
            ScanStepArray.Add( BkScanStepMemory );
            ScanStepArray.Add( BkScanStepAutorun );
            ScanStepArray.Add( BkScanStepCriticalDir );
            ScanStepArray.Add( BkScanStepAllDisk );
        }
        break;

    case (BkScanModeOnlyCustomScan):
        {
            ScanStepArray.Add( BkScanStepCustomDir );
        }
        break;

    default:
        {
            _ASSERT(0);
            return E_FAIL;
        }
        break;
    }
    return S_OK;
}


int CScanStep::GetScanStep( int ScanMode, DWORD ScanFileNum, int SrcScanStep )
{
    if ( ScanMode <= BkScanModeNull
        || BkScanModeNum <= ScanMode )
    {
        return SrcScanStep;
    }

    CObjGuard AutoLock(m_Lock);

    int RetScanStep = SrcScanStep;
    if ( BkScanRelationDirStepMemory == RetScanStep 
        || BkScanRelationDirStepAutorun == RetScanStep )
    {
        RetScanStep = BkScanStepCriticalDir;
    }

    switch (ScanMode)
    {
    case (BkScanModeFastScan):
        {
            if ( ScanFileNum <= m_MemFileNum )
            {
                RetScanStep = BkScanStepMemory;
            }
            else if (  ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum) )
            {
                RetScanStep = BkScanStepAutorun;
            }
            else if ( ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum+m_CriticalDirFileNum) )
            {
                RetScanStep = BkScanStepCriticalDir;
            }
            else
            {
                _ASSERT(0);
            }
        }
        break;

    case (BkScanModeIeFix):
    case (BkScanModeOnlyMemoryScan):
        {
            if ( ScanFileNum <= m_MemFileNum )
            {
                RetScanStep = BkScanStepMemory;
            }
            else
            {
                _ASSERT(0);
            }
        }
        break;

    case (BkScanModeBootSystemScan):
    case (BkScanModeCheckScan):
        {
            if ( ScanFileNum <= m_MemFileNum )
            {
                RetScanStep = BkScanStepMemory;
            }
            else if (  ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum) )
            {
                RetScanStep = BkScanStepAutorun;
            }
            else
            {
                _ASSERT(0);
            }
        }
        break;

    case (BkScanModeCustomScan):
        {
            if ( ScanFileNum <= m_MemFileNum )
            {
                RetScanStep = BkScanStepMemory;
            }
            else if (  ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum) )
            {
                RetScanStep = BkScanStepAutorun;
            }
            else if ( ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum+m_CriticalDirFileNum) )
            {
                RetScanStep = BkScanStepCriticalDir;
            }
            else if ( ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum+m_CriticalDirFileNum+m_CustomDirFileNum) )
            {
                RetScanStep = BkScanStepCustomDir;
            }
            else
            {
                _ASSERT(0);
            }
        }
        break;

    case (BKScanModeMoveDiskScan):
        {
            if ( ScanFileNum <= m_MemFileNum )
            {
                RetScanStep = BkScanStepMemory;
            }
            else if (  ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum) )
            {
                RetScanStep = BkScanStepAutorun;
            }
            else if ( ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum+m_MoveDisFileNum) )
            {
                RetScanStep = BkScanStepMoveDisk;
            }
            else
            {
                _ASSERT(0);
            }
        }
        break;

    case (BkScanModeRightScan):
        {
            if ( ScanFileNum <= m_MemFileNum )
            {
                RetScanStep = BkScanStepMemory;
            }
            else if (  ScanFileNum <= (m_MemFileNum+m_CustomDirFileNum) )
            {
                RetScanStep = BkScanStepCustomDir;
            }
            else
            {
                _ASSERT(0);
            }

        }
        break;

    case (BkScanModeFullScan):
        {
            if ( ScanFileNum <= m_MemFileNum )
            {
                RetScanStep = BkScanStepMemory;
            }
            else if (  ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum) )
            {
                RetScanStep = BkScanStepAutorun;
            }
            else if ( ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum+m_CriticalDirFileNum) )
            {
                RetScanStep = BkScanStepCriticalDir;
            }
            else if ( ScanFileNum <= (m_MemFileNum+m_AutoRunFileNum+m_CriticalDirFileNum+m_AllDiskFileNum) )
            {
                RetScanStep = BkScanStepAllDisk;
            }
            else
            {
                _ASSERT(0);
            }

        }
        break;

    case (BkScanModeOnlyCustomScan):
        {
            RetScanStep = BkScanStepCustomDir;
        }
        break;

    default:
        {
            _ASSERT(0);
        }
        break;
    }

    return RetScanStep;
}

















DWORD CScanProgress2Hint::GetHintCount(LPCWSTR lpszScanMode, DWORD dwMin, DWORD dwMax)
{
    assert(lpszScanMode);
    assert(dwMin <= dwMax);
    if (!m_bAlreadyRead)
    {
        m_bAlreadyRead = TRUE;  // 一次扫描仅读一次配置

        WinMod::CWinPath pathIni;
        HRESULT hr = CAppPath::Instance().GetLeidianLogPath(pathIni.m_strPath);
        if (FAILED(hr))
            return 0;

        pathIni.Append(GetScanIniFile());

        m_dwHintCount = ::GetPrivateProfileInt(GetScanAppName(), L"FastScan", 0, pathIni.m_strPath);
    }  

    if (m_dwHintCount < dwMin || m_dwHintCount > dwMax)
    {   // 提示的数目超出范围
        return 0;
    }

    return m_dwHintCount;
}

void CScanProgress2Hint::SetHintCount(LPCWSTR lpszScanMode, DWORD dwCount, DWORD dwMin, DWORD dwMax)
{
    assert(lpszScanMode);
    assert(dwMin <= dwMax);
    if (dwCount < dwMin || dwCount > dwMax)
    {   // 提示的数目超出范围
        return;
    }

    if (dwCount == m_dwHintCount)
    {   // 值未发生变化
        return;
    }

    if (!m_bAlreadyWrite)
    {
        m_bAlreadyWrite = TRUE;  // 一次扫描仅写一次配置

        WinMod::CWinPath pathIni;
        HRESULT hr = CAppPath::Instance().GetLeidianLogPath(pathIni.m_strPath);
        if (FAILED(hr))
            return;

        pathIni.Append(GetScanIniFile());

        CString strConfig;
        strConfig.Format(L"%lu", dwCount);

        ::WritePrivateProfileString(GetScanAppName(), L"FastScan", strConfig, pathIni.m_strPath);
    }  
}







CScanProgress2::CScanProgress2()
{
    Reset();
}

CScanProgress2::~CScanProgress2() 
{

}

void CScanProgress2::Reset()
{
    m_bEnumFinished                         = FALSE; 
    m_dVirtualProgress                      = 0;
    m_dLastVirtualProgress                  = 0;

    m_dwScanFileCountForLastVirtualProgress = 0;
    
    m_hintFastScan.Reset();
    m_hintFullScan.Reset();
}




// 线性插值
// xSample 和 ySample 都必须是升序
double CScanProgress2::LinearProgress(
    double xVal,
    double xSamples[],
    double ySamples[],
    size_t nSampleCount,
    double yLowerBound,
    double yUpperBound)
{
    assert(xVal >= 0);
    assert(nSampleCount >= 0);

    double* pFind = std::lower_bound(xSamples, xSamples + nSampleCount, xVal);
    size_t nOffset = pFind - xSamples;

    // 检查是否越界
    if (nOffset <= 0)
        return yLowerBound;

    if (nOffset >= nSampleCount)
        return yUpperBound;


    double xDelta = xSamples[nOffset] - xSamples[nOffset - 1];
    double yDelta = ySamples[nOffset] - ySamples[nOffset - 1];


    assert(xDelta > DBL_EPSILON);
    if (xDelta <= DBL_EPSILON)
        return ySamples[nOffset - 1];

    assert(yDelta > DBL_EPSILON);
    if (yDelta <= DBL_EPSILON)
        return ySamples[nOffset - 1];


    // 计算斜率
    double dSlope = yDelta / xDelta;

    // 线性插值
    double yVal   = ySamples[nOffset - 1] + (xVal - xSamples[nOffset - 1]) * dSlope;

    return yVal;
}



DWORD CScanProgress2::GetScanProgress( int ScanMode, DWORD ScanFileNum, DWORD TotalFileNum, DWORD LastProgress, BOOL bEnumFinished)
{
    double dNewProgress = m_dVirtualProgress;

    if ( bEnumFinished )
    {
        if ( BkScanModeFastScan == ScanMode )
        {
            SetHintCountForFastScan(TotalFileNum);
        }
        

        // 没有文件被枚举到
        if (0 == TotalFileNum)
        {
            m_dVirtualProgress = 100.0f;
            return 100;
        }

        // 扫描结果和枚举数相同
        if (ScanFileNum == TotalFileNum)
        {
            m_dVirtualProgress = 100.0f;
            return 100;
        }


        if (ScanFileNum > m_dwScanFileCountForLastVirtualProgress &&
            ScanFileNum < TotalFileNum)
        {
            // 根据最后一次获取到的虚拟进度及相应的文件数,进行线性追赶
            double xSamples[] = {m_dwScanFileCountForLastVirtualProgress,   TotalFileNum};
            double ySamples[] = {m_dLastVirtualProgress,                    99};
            assert(_countof(xSamples) == _countof(ySamples));

            // 线性插值
            dNewProgress = LinearProgress(
                double(ScanFileNum), 
                xSamples, ySamples, 
                _countof(xSamples),
                0, 98);
        }
    }
    else
    {
        if ( BkScanModeFullScan == ScanMode )
        {   // 完全扫描,通常会超过50000个文件
            double xSamples[] = {100, 1000, 10000, 50000, 100000, 200000};
            double ySamples[] = {1,   5,    25,    50,    75,     95};
            assert(_countof(xSamples) == _countof(ySamples));

            // 线性插值
            dNewProgress = LinearProgress(
                double(ScanFileNum), 
                xSamples, ySamples, 
                _countof(xSamples),
                0, 95);
        }
        else if ( BkScanModeCheckScan == ScanMode )
        {   // 体检扫描,通常会超过800个文件
            double xSamples[] = {10, 50, 200, 800};
            double ySamples[] = {10, 20, 30,  90};
            assert(_countof(xSamples) == _countof(ySamples));

            // 线性插值
            dNewProgress = LinearProgress(
                double(ScanFileNum), 
                xSamples, ySamples, 
                _countof(xSamples),
                0, 95);
        }
        else if ( BkScanModeFastScan == ScanMode )
        {   // 快速扫描,通常会是 10000 到 40000 个文件
            DWORD dwHintCountFastScan = GetHintCountForFastScan();
            if (dwHintCountFastScan)
            {   // 使用提示的数目
                double xSamples[] = {0,   dwHintCountFastScan / 2, dwHintCountFastScan, dwHintCountFastScan * 2};
                double ySamples[] = {0,   45,                      90,                  95};
                assert(_countof(xSamples) == _countof(ySamples));

                // 线性插值
                dNewProgress = LinearProgress(
                    double(ScanFileNum), 
                    xSamples, ySamples, 
                    _countof(xSamples),
                    0, 95);
            }
            else
            {   // 没有提示的数目,使用猜测的数目
                double xSamples[] = {100, 1000, 5000, 10000, 30000, 100000};
                double ySamples[] = {1,   5,    25,   50,    75,    95};
                // 斜率倒数          100, 225,  200,  200,   800,   3500
                assert(_countof(xSamples) == _countof(ySamples));

                // 线性插值
                dNewProgress = LinearProgress(
                    double(ScanFileNum), 
                    xSamples, ySamples, 
                    _countof(xSamples),
                    0, 95);
            }
        }
        else
        {   // 其他扫描,按照完全扫描进行
            double xSamples[] = {100, 1000, 10000, 50000, 100000, 200000};
            double ySamples[] = {1,   5,    25,    50,    75,     95};
            assert(_countof(xSamples) == _countof(ySamples));

            // 线性插值
            dNewProgress = LinearProgress(
                double(ScanFileNum), 
                xSamples, ySamples, 
                _countof(xSamples),
                0, 95);
        }


        // 记录最新的文件个数和虚拟扫描进度
        m_dwScanFileCountForLastVirtualProgress = ScanFileNum;
        m_dLastVirtualProgress                  = dNewProgress;
    }



    // 进度不能后退,只能前进
    m_dVirtualProgress = max(dNewProgress, m_dVirtualProgress);


    // 对外返回的进度总是加1
    return min(DWORD(m_dVirtualProgress + 1), 100);
}




DWORD CScanProgress2::GetHintCountForFastScan()
{
    return m_hintFastScan.GetHintCount(L"FastScan", 5000, 50000);
}

void CScanProgress2::SetHintCountForFastScan(DWORD dwCount)
{
    m_hintFastScan.SetHintCount(L"FastScan", dwCount, 5000, 50000);
}