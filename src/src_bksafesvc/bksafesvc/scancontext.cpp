#include "stdafx.h"
#include "common.h"
#include "scancontext.h"

BOOL CUnique::IsFilterDir( const CString& DirPath, BOOL& bRecursion )
{
    if ( FALSE == WinMod::CWinPathApi::IsDirectory(DirPath) )
    {
        _ASSERT(0);
        return FALSE;
    }

    return _IsFindDir(DirPath, bRecursion);
}



void CUnique::AddCancelDoubleDir( const CString& ScanDir, BOOL bRecursion )
{
    WinMod::CWinPath DirPath;
    DirPath.m_strPath = ScanDir;
    DirPath.AddBackslash();
  
    if ( FALSE == WinMod::CWinPathApi::IsDirectory(DirPath.m_strPath) )
    {
        _ASSERT(0);
        return;
    }

    BOOL bExit = FALSE;
    POSITION Pos = m_DirPathMap.GetStartPosition();
    if (  NULL == Pos )
    {
        m_DirPathMap.SetAt( DirPath.m_strPath, bRecursion );
        bExit = TRUE;
    }
    else
    {
        //消重判断
        while ( Pos )
        {
            CAtlMap<CString, BOOL>::CPair* pPair = m_DirPathMap.GetNext( Pos );
            if ( -1 != DirPath.m_strPath.Find(pPair->m_key) && pPair->m_value )
            {
                //1 是扫描过的某个目录的子目录，并且扫描过的某个目录是递归扫描。
                //2 是扫描过的某个目录，并且扫描过的目录是递归扫描。
                //不管所要扫描的目录是否递归。它都已经被扫描过来。
                bExit = TRUE;
                break;
            }
        }
    }
    if ( bExit )
    {
        return;
    }

    //消重判断
    if ( bRecursion )
    {
        //1 不是扫描过的目录的子目录，而是已经扫描过目录的父目录，并且当前目录递归扫描（要对消重表进行消重）
        CAtlArray<CString> DelKeyArray;
        POSITION Pos = m_DirPathMap.GetStartPosition();
        while ( Pos )
        {
            CAtlMap<CString, BOOL>::CPair* pPair = m_DirPathMap.GetNext( Pos );
            if ( -1 != pPair->m_key.Find(DirPath.m_strPath) )
            {
                DelKeyArray.Add( pPair->m_key );
            }
        }
        size_t Num = DelKeyArray.GetCount();
        size_t i = 0;
        for(; i<Num; ++i )
        {
            m_DirPathMap.RemoveKey( DelKeyArray[i] );
        }
    }

    //1 并不是扫描过的目录的子目录,而是无关的目录。（添加新项）
    //2 并不是扫描过的目录的子目录，而是以扫描过目录的父目录，并且当前目录递归扫描。（要对消重表进行消重，之后添加新项）
    //3 是扫描过的某个目录的子目录，但扫描过的某个目录，并不是递归扫描。（添加新项）
    //4 有可能DirPath这个目录.已经非递归扫过，这里重新赋下值。（重新赋值）
    m_DirPathMap.SetAt( DirPath.m_strPath, bRecursion );
}



BOOL CUnique::IsFilterFile( const CString& FilePath )
{
    if ( FALSE == WinMod::CWinPathApi::IsFileExisting(FilePath) )
    {
        //文件存在。但判断不出。如pagefile.sys
        return TRUE;
    }
    BOOL bFilter = FALSE;
    CPath DirPath = FilePath;
    DirPath.RemoveFileSpec();
    BOOL bTemp = FALSE;
    if ( _IsFindFile(FilePath) || _IsFindDir(DirPath.m_strPath, bTemp) )
    {
        //是提交过的文件，或文件所在目录为，扫描过的目录。
        bFilter = TRUE;
    }
    return bFilter;
}

void CUnique::AddCancelDoubleFile( const CString& FilePath )
{
    if ( FALSE == WinMod::CWinPathApi::IsFileExisting(FilePath) )
    {
        _ASSERT(0);
        return;
    }
    m_FilePathMap.SetAt( FilePath, 33 );
}

BOOL CUnique::_IsFindFile( const CString& FilePath )
{
    if ( NULL != m_FilePathMap.Lookup(FilePath) )
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CUnique::_IsFindDir( const CString& DirPath, BOOL& bRecursion )
{
    BOOL bRet = FALSE;
    //匹配子目录查找,用于判断是否是，递归查找的子目录。
    POSITION Pos = m_DirPathMap.GetStartPosition();
    if ( NULL == Pos )
    {
        return FALSE;
    }
    while( Pos )
    {
        CAtlMap<CString, BOOL>::CPair* pPair = m_DirPathMap.GetNext( Pos );

        if ( -1 != DirPath.Find(pPair->m_key) && pPair->m_value )
        {
            //1 是扫描过的某个目录的子目录，并且扫描过的某个目录是递归扫描。
            //2 是扫描过的某个目录，并且扫描过的目录是递归扫描。
            bRecursion = TRUE;
            bRet = TRUE;
            //记录结果，并返回
            break;
        }
        if ( 0 == DirPath.CompareNoCase(pPair->m_key) && (FALSE == pPair->m_value) )
        {
            //1 是消除目录表中某相同目录，并且没递归扫描过。
            bRecursion = FALSE;
            bRet = TRUE;
            //记录结果，并返回
            break;
        }
    }
    return bRet;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
CAtlMap<CString, RELATETION_DIR_INFO>&    CScanContext::GetRelationDir( )
{
    return m_RelationPathMap;
}

void    CScanContext::RecordCleanInfo( const CString& FilePath, void* pvFlag, PVOID pParam )
{
    DWORD ScanStep = (DWORD)(DWORD_PTR)pvFlag;
    if ( (BkScanStepMemory == ScanStep)
        && (NULL != pParam) )
    {
        m_CleanOperation.RecordCleanDataForMemory( FilePath, (DWORD)(DWORD_PTR)pParam );
    }
    else if ( BkScanStepAutorun == ScanStep )
    {
        //autorun 扫描添加文件
        m_CleanOperation.RecordCleanDataForAutorun( FilePath, pParam );
    }
}

void    CScanContext::OperateCleanInfo( int Type, const CString& FilePath )
{
    if ( BkScanStepMemory == Type )
    {
        m_CleanOperation.OperateMemoryClean( FilePath );
    }
    else if ( BkScanStepAutorun == Type )
    {
        m_CleanOperation.OperateAutoClean( FilePath );
    }
    else
    {
        _ASSERT(0);
    }
}
 
HRESULT CScanContext::OperateFile(  const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask, void* pvFlag,  PVOID pParam )
{
    //////
    if ( (BkScanModeCheckScan == m_ScanMode) 
        && (BkScanStepAutorun == (INT)(INT_PTR)pvFlag) )
    {
        if ( FALSE == m_OptimizeForCheckScanPolicy.IsNeedOperate( FilePath, pvFlag, pParam ) )
        {
            return S_OK;
        }
    }
    //////

    if ( m_CancelDouble.IsFilterFile(FilePath) )
    {
        return S_OK;
    }

    if (m_HeurDir.IsFileUnderHeuristDir(FilePath))
    {   // 如果在系统目录,则允许使用云端高启发
        dwScanMask |= BKENG_SCAN_MASK_USE_CLOUD_HEURIST_HIGH_RISK;
    }

    m_piOpetFileCallback->OperateFile( FilePath, pFindData, dwScanMask, pvFlag, pParam );
    if ( m_bAddPathForFile )
    {
        m_CancelDouble.AddCancelDoubleFile( FilePath );
    }
    return S_OK;
}
BOOL    CScanContext::IsExitEnumWork()
{
    return m_piOpetFileCallback->IsExitEnumWork();
}

//过滤用
void    CScanContext::AddRelationDir( const CString& DirPath, int Step )
{
    if ( FALSE == WinMod::CWinPathApi::IsDirectory(DirPath) )
    {
        _ASSERT(0);
        return;
    }
    if ( m_bRecordRelationDir )
    {
        if ( NULL == m_RelationPathMap.Lookup( DirPath ) )
        {
            RELATETION_DIR_INFO RelationDirInfo;
            RelationDirInfo.bRecursion = FALSE;
            RelationDirInfo.Step       = Step;
            m_RelationPathMap.SetAt( DirPath, RelationDirInfo );
        }
    }
}
BOOL    CScanContext::IsFilterDir( const CString& DirPath, BOOL& bRecursion )
{
    return m_CancelDouble.IsFilterDir( DirPath, bRecursion );
}
void    CScanContext::AddCancelDoubleDir( const CString& ScanDir, BOOL bRecursion )
{
    m_CancelDouble.AddCancelDoubleDir( ScanDir, bRecursion );
}



/////////////////////////////////////////////////////////////////////////////////////
BOOL COptimizeCheckScanPolicy::IsNeedOperate( const CString& FilePath, void*   pvFlag,  PVOID pParam )
{
    BOOL bRet = FALSE;

    const static WCHAR* ppDirName[] =
    {
        //
        L"\\winrar\\",
        L"\\realplayer\\",
        L"\\office11\\",
        L"\\outlook express\\",
        L"\\internet explorer\\",
        L"\\tencent\\",
        L"\\kwmusic\\",
        L"\\stormii\\",
        L"\\thunder\\",
        L"\\sogouinput\\",
        L"\\ttplayer\\",
        L"\\aliwangwang\\",
        //
        NULL
    };

    CString FindData = L"\\program files\\";
    FindData.MakeLower();
    if ( -1 == FilePath.Find(FindData) )
    {
        //没有找到，说明不是第三方软件，扫描文件。
        bRet = TRUE;
    }
    else
    {
        //第三方软件
        for ( int i=0; ppDirName[i]; ++i )
        {
            FindData = ppDirName[i];
            FindData.MakeLower();
            if ( -1 != FilePath.Find(FindData) )
            {
                //比较重要的第三方软件目录，扫描文件。
                bRet = TRUE;
            }
        }
    }

    return bRet;
}