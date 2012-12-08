#pragma once
#include "autorunex.h"
#include "comproxy\comdata_scan_def.h"
#include "common\utility.h"
#include "scancontext.h"


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////








//内存枚举
class CMemoryEnumTask
{
public:
    CMemoryEnumTask(){};
    ~CMemoryEnumTask(){};
    HRESULT EnumScanFile( CScanContext* piCallback );
    HRESULT EnumScanFileForIeFixPolicy( CScanContext* piCallback );
    HRESULT EnumScanFileForCheckPolicy( CScanContext* piCallback );
private:
    HRESULT EnumModuleProcess( CScanContext* piCallback,  DWORD Pid );
};


//启动项枚举
class CAutorunEnumTask 
{
public:
    CAutorunEnumTask()
        :m_ClientPid(0)
    {}
    ~CAutorunEnumTask(){}
    void    SetParam( DWORD ClientPid )
    {
        m_ClientPid = ClientPid;
    }

    HRESULT EnumScanFile( CScanContext* piCallback, DWORD dwEnumParam );
private:
    DWORD                           m_ClientPid;
};


//目录枚举 基类
class CEnumDirTask 
{
public:
    //for dir
    virtual BOOL IsFilterScanDir(  const CString& StrDirPath );

    virtual HRESULT OperateFile( CScanContext* piCallback, const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask, PVOID pParam ) = 0;

    HRESULT EnumScanFileForDir( CScanContext* piCallback, const CString& DirPath, BOOL bRecursion );
    HRESULT EnumFileCurrentDir( CScanContext* piCallback, const CString& DirPath );
    HRESULT EnumFileDepth( CScanContext* piCallback, const CString& DirPath );
    //for dir
};


//关键目录枚举
class CCriticalDirEnumTask :
    public CEnumDirTask
{
public:
    CCriticalDirEnumTask(){}
    ~CCriticalDirEnumTask(){}

    HRESULT EnumScanFile( CScanContext* piCallback );

    virtual BOOL IsFilterScanDir(  const CString& StrDirPath );

    
    
private:
    virtual HRESULT OperateFile( CScanContext* piCallback, const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask,PVOID pParam )
    {
        return piCallback->OperateFile( FilePath, pFindData, dwScanMask, (PVOID)BkScanStepCriticalDir, pParam );
    }
    HRESULT InitCriticalDirInfo();
    CAtlMap<CString, BOOL>      m_CriticalDirMap;
    CAtlList<CString>           m_FilterDirList;
};


//所有盘符 递归或不递归 枚举
class CAllRootDirEnumTask :
    public CEnumDirTask
{
public:
    CAllRootDirEnumTask( BOOL bRecursion ) :m_bRecursion( bRecursion )
    { }
    ~CAllRootDirEnumTask()
    {}
       
    HRESULT EnumScanFile( CScanContext* piCallback );
private:
    virtual HRESULT OperateFile( CScanContext* piCallback, const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask, PVOID pParam );
    BOOL m_bRecursion;
};

//移动测评扫描
class CMoveDisEnumTask :
    public CEnumDirTask
{
public:
    CMoveDisEnumTask(){}
    ~CMoveDisEnumTask(){}
    HRESULT EnumScanFile( CScanContext* piCallback );
private:
    virtual HRESULT OperateFile( CScanContext* piCallback, const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask, PVOID pParam )
    {
        return piCallback->OperateFile( FilePath, pFindData, dwScanMask, (PVOID)BkScanStepMoveDisk, pParam );
    }
    BOOL m_bRecursion;

private:

};


//自定义目录枚举
class CCustomDirEnumTask :
    public CEnumDirTask
{
public:
    CCustomDirEnumTask():m_ScanStep(BkScanStepCustomDir){}
    ~CCustomDirEnumTask(){}

    HRESULT EnumScanFile( CScanContext* piCallback ){return S_OK;}
    
    HRESULT EnumScanFileForCusomDir( CScanContext* piCallback, CAtlMap<CString, BOOL>& DirParam, int ScanStep );
    HRESULT EnumScanFileForRelationDir( CScanContext* piCallback, CAtlMap<CString, RELATETION_DIR_INFO>& RelationDirMap );
private:
    virtual HRESULT OperateFile( CScanContext* piCallback, const CString& FilePath, const WIN32_FIND_DATAW* pFindData, DWORD dwScanMask, PVOID pParam )
    {
        return piCallback->OperateFile( FilePath, pFindData, dwScanMask, (PVOID)(INT_PTR)m_ScanStep, pParam );
    }
    int                  m_ScanStep;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


