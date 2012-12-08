#pragma once
#include "winmod\winfilefinddepthfirst.h"
#include "cleaninfo.h"
#include "avpheurdir.h"

//回调基类
class IEnumOperation
{
public:
    virtual HRESULT OperateFile(
        const CString& FilePath,
        const WIN32_FIND_DATAW* pFindData,
        DWORD dwScanMask,
        void* pvFlag,
        PVOID pParam ) = 0;
    virtual BOOL    IsExitEnumWork() = 0;
};

struct RELATETION_DIR_INFO
{
    BOOL bRecursion;
    int  Step;
};

//消除回调类
class CUnique
{
public: 
    CUnique(){}
    ~CUnique(){}
    void Reset()
    {
        m_FilePathMap.RemoveAll();
        m_DirPathMap.RemoveAll();
    }
public:
    void AddCancelDoubleDir( const CString& ScanDir, BOOL bRecursion );
    BOOL IsFilterDir( const CString& DirPath, BOOL& bRecursion );
    BOOL IsFilterFile( const CString& FilePath );
    void AddCancelDoubleFile( const CString& FilePath );

private:
    BOOL _IsFindFile( const CString& FilePath );

    BOOL _IsFindDir( const CString& DirPath, BOOL& bRecursion );

    CAtlMap<CString, DWORD>       m_FilePathMap;
    CAtlMap<CString, BOOL>        m_DirPathMap;
};


class COptimizeCheckScanPolicy
{
public:
    COptimizeCheckScanPolicy(){};
    ~COptimizeCheckScanPolicy(){};

    BOOL IsNeedOperate( const CString& FilePath, void*   pvFlag,  PVOID pParam );
};



class CScanContext 
{
public:
    CScanContext()
        : m_bAddPathForFile(FALSE)
        , m_bRecordRelationDir(FALSE)
        , m_ScanMode(BkScanModeNull)
    {}
    ~CScanContext(){}

    void Reset()
    {
        m_HeurDir.LoadHeuristDir();

        m_CancelDouble.Reset();
        m_RelationPathMap.RemoveAll();
        m_bAddPathForFile = FALSE;
        m_bRecordRelationDir = FALSE;
        m_CleanOperation.Reset();
        m_ScanMode = BkScanModeNull;
    }

    void    SetCallback( IEnumOperation* pCallback )
    {
        _ASSERT(pCallback);
        m_piOpetFileCallback = pCallback;
    };

    //
    CAtlMap<CString, RELATETION_DIR_INFO>&    GetRelationDir( );
    //
    void    OperateCleanInfo( int Type, const CString& FilePath );

    void    SetScanMode( int ScanMode )
    {
        m_ScanMode = ScanMode;
    }

    int     GetScanMode() { return m_ScanMode; };

    ////////////////////////
    HRESULT OperateFile(  const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask, void* pvFlag, PVOID pParam );
    BOOL    IsExitEnumWork();
    //过滤用
    void    RecordCleanInfo( const CString& FilePath, void* pvFlag, PVOID pParam );
    void    AddRelationDir( const CString& DirPath, int Step );
    BOOL    IsFilterDir( const CString& DirPath, BOOL& bRecursion );
    void    AddCancelDoubleDir( const CString& ScanDir, BOOL bRecursion );
    //////////////////////////



public:
    void    SetAddCancelDoublePathForFile( BOOL bAddPath )
    {
        m_bAddPathForFile = bAddPath;
    }

    void    SetAddRelationDir( BOOL bAddPath )
    {
        m_bRecordRelationDir = bAddPath;
    }

    WinMod::CWinFileFindRecursiveJuctionGuardFilter m_JunctionFilter;

private:
    IEnumOperation*                 m_piOpetFileCallback;
private:
    Skylark::CAvpHeurDir            m_HeurDir;
    CUnique                         m_CancelDouble;
    CAtlMap<CString, RELATETION_DIR_INFO>          m_RelationPathMap;
    BOOL                            m_bAddPathForFile;
    BOOL                            m_bRecordRelationDir;
    //clean 数据
    CCleanOperation                 m_CleanOperation;

    //体检扫描的优化类
    int                             m_ScanMode;
    COptimizeCheckScanPolicy        m_OptimizeForCheckScanPolicy;
};