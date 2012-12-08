#pragma once
#include "scancontext.h"


class CFastScanPolicy 
{
public:
    CFastScanPolicy(){}
    ~CFastScanPolicy(){};
    //快速扫描 扫描内存、启动项、关键目录，关联目录
    HRESULT StartScan( CScanContext* pScanContext );
    HRESULT StartScan( CScanContext* pScanContext, CAtlMap<CString, BOOL>& CustomParamMap );
};


class CIeFixScanPolicy
{
public:
    CIeFixScanPolicy(){};
    ~CIeFixScanPolicy(){};
    HRESULT StartScan( CScanContext* pScanContext );
};

//
class COnlyMemoryScanPolicy
{
public:
    COnlyMemoryScanPolicy(){}
    ~COnlyMemoryScanPolicy(){}
    //体检扫描 扫描内存、启动项
    HRESULT StartScan( CScanContext* pScanContext );
};


//体检扫描
class CCheckScanPolicy
{
public:
    CCheckScanPolicy(){}
    ~CCheckScanPolicy(){}
    //体检扫描 扫描内存、启动项
    HRESULT StartScan( CScanContext* pScanContext );
    HRESULT StartScanForCacheMemory( CScanContext* pScanContext );
    HRESULT StartScanForCacheAutoRun( CScanContext* pScanContext );
};



//自定义扫描
class CCustomScanPolicy
{
public:
    CCustomScanPolicy(){}
    ~CCustomScanPolicy(){};
    //自定义扫描 扫描内存、启动项、关键目录，关联目录，自定义目录
    HRESULT StartScan( CScanContext* pScanContext, CAtlMap<CString, BOOL>& CustomDirMap );

};



//全盘扫描策略
class CFullDiskScanPolicy
{
public:
    CFullDiskScanPolicy(){}
    ~CFullDiskScanPolicy(){};
    //全盘扫描 扫描内存、启动项、关键目录，关联目录，所有盘符
    HRESULT StartScan( CScanContext* pScanContext );

};


class CMoveDiskScanPolicy
{
public:
    CMoveDiskScanPolicy(){}
    ~CMoveDiskScanPolicy(){};
    //扫描内存 扫描启动项 扫描移动磁盘
    HRESULT StartScan( CScanContext* pScanContext );

};



class CRightScanPolicy
{
public:
    CRightScanPolicy(){}
    ~CRightScanPolicy(){};
    //右键扫描 扫描内存和用户指定目录
    HRESULT StartScan( CScanContext* pScanContext, CAtlMap<CString, BOOL>& RightParamMap );

};


class COnlyCustomScanPolicy
{
public:
    HRESULT StartScan( CScanContext* pScanContext, CAtlMap<CString, BOOL>& CustomParamMap );
};


