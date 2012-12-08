/********************************************************************
	created:	2010/10/11
	filename: 	ifcache.h
	author:		Jiang Fengbing
	
	purpose:	硬盘文件信息cache接口
*********************************************************************/

#pragma once

//////////////////////////////////////////////////////////////////////////

#include <unknwn.h>

//////////////////////////////////////////////////////////////////////////

typedef enum tagFCacheQueryType {
    enumFQT_Top,    // 按照Top, 带一个参数(ULONG)
    enumFQT_Ext,    // 按照扩展名, 带一个参数(const char*)
    enumFQT_Zone,   // 按照范围, 带两个参数(ULONGLONG*, ULONGLONG*)
    enumFQT_Word,   // 按照关键词, 带一个参数(const wchar_t*)
} FCacheQueryType;

/**
 * @brief Cache查询返回数据的接口
 */
[
    uuid(6a1ba2d1-08a1-4e02-ae74-ee692b478fa4)
]
interface IFCacheQueryback
{
    /**
     * @brief 返回Query查询到的数据
     */
    virtual void OnData(
        FCacheQueryType nQueryType,
        LPCWSTR lpFilePath,
        ULONGLONG qwFileSize
        ) = 0;

    /**
     * @brief 返回QueryTopExt查询到的数据
     */
    virtual void OnExtData(
        LPCWSTR lpExt,
        ULONGLONG qwSize,
        ULONGLONG qwCount
        ) = 0;
};

/**
 * @biref cache接口, 先BeginAdd, 然后AddFile, 最后EndAdd
 */
[
    uuid(45f1e12b-1a5f-402f-8264-3458cb4aafb4)
]
interface IFCache : IUnknown
{
    /**
     * @brief 初始化
     */
    virtual BOOL __cdecl Initialize(
        ) = 0;

    /**
     * @brief 反初始化
     */
    virtual BOOL __cdecl UnInitialize(
        ) = 0;

    /**
     * @brief 开始添加事务
     */
    virtual BOOL __cdecl BeginAdd(
        ) = 0;

    /**
     * @brief 结束添加事务
     */
    virtual BOOL __cdecl EndAdd(
        ) = 0;

    /**
     * @brief 取消添加事务
     */
    virtual BOOL __cdecl CancelAdd(
        ) = 0;

    /**
     * @brief 添加文件信息
     */
    virtual BOOL __cdecl AddFile(
        LPCWSTR lpFilePath,
        ULONGLONG qwFileSize
        ) = 0;

    /**
     * @brief 删除文件信息
     */
    virtual BOOL __cdecl DelFile(
        LPCWSTR lpFilePath
        ) = 0;

    /**
     * @brief 删除某个目录下面的所有文件信息
     */
    virtual BOOL __cdecl DelDir(
        LPCWSTR lpDir
        ) = 0;

    /**
     * @brief 获得总的大小
     */
    virtual BOOL __cdecl GetTotalInfo(
        ULONGLONG& qwTotalSize,
        ULONGLONG& qwTotalCount
        ) = 0;

    /**
     * @brief 按需查询
     */
    virtual BOOL __cdecl Query(
        IFCacheQueryback* piQueryback,
        FCacheQueryType queryType,
        void* pParam1,
        void* pParam2
        ) = 0;

    /**
     * @brief 清空Cache
     */
    virtual BOOL __cdecl Clean(
        ) = 0;

    /**
     * @brief 获得文件的大小
     */
    virtual BOOL __cdecl GetFileSize(
        LPCWSTR lpFilePath,
        ULONGLONG& qwSize
        ) = 0;

    /**
     * @brief 获得目录大小
     */
    virtual BOOL __cdecl GetDirInfo(
        LPCWSTR lpDir,
        ULONGLONG& qwSize,
        ULONGLONG& qwCount
        ) = 0;

    /**
     * @brief 获得扩展名Top榜
     */
    virtual BOOL __cdecl QueryTopExt(
        IFCacheQueryback* piQueryback,
        int nTop
        ) = 0;

    /**
     * @brief 记录扫描是否完整和扫描时间
     */
    virtual BOOL __cdecl SetCacheInfo(
        const SYSTEMTIME& scanTime,
        BOOL bFullCache
        ) = 0;

    /**
     * @brief 获得扫描是否完整和扫描时间
     */
    virtual BOOL __cdecl GetCacheInfo(
        SYSTEMTIME& scanTime,
        BOOL& bFullCache
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 获得Cache访问接口
 * @param riid: in __uuidof(IFCache)
 * @param ppvObject: out 对象实例
 * @param cVol: in 盘符
 */
typedef BOOL (__cdecl *GetFCache_t)(REFIID riid, void** ppvObject, char cVol);

BOOL __cdecl GetFCache(REFIID riid, void** ppvObject, char cVol);

//////////////////////////////////////////////////////////////////////////
