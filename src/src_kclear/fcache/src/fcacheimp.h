#pragma once

//////////////////////////////////////////////////////////////////////////

#include "kclear/ifcache.h"
#include "3rdparty/sqlite3/sqlite3.h"

//////////////////////////////////////////////////////////////////////////

class CFCacheImpl : public IFCache
{
public:
    CFCacheImpl(char cVol);
    virtual ~CFCacheImpl();

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);

    /**
     * @brief 初始化
     */
    virtual BOOL __cdecl Initialize(
        );

    /**
     * @brief 反初始化
     */
    virtual BOOL __cdecl UnInitialize(
        );

    /**
     * @brief 开始添加事务
     */
    virtual BOOL __cdecl BeginAdd(
        );

    /**
     * @brief 结束添加事务
     */
    virtual BOOL __cdecl EndAdd(
        );

    /**
     * @brief 取消添加事务
     */
    virtual BOOL __cdecl CancelAdd(
        );

    /**
     * @brief 添加文件信息
     */
    virtual BOOL __cdecl AddFile(
        LPCWSTR lpFilePath,
        ULONGLONG qwFileSize
        );

    /**
     * @brief 删除文件信息
     */
    virtual BOOL __cdecl DelFile(
        LPCWSTR lpFilePath
        );

    /**
     * @brief 删除某个目录下面的所有文件信息
     */
    virtual BOOL __cdecl DelDir(
        LPCWSTR lpDir
        );

    /**
     * @brief 获得总的大小
     */
    virtual BOOL __cdecl GetTotalInfo(
        ULONGLONG& qwTotalSize,
        ULONGLONG& qwTotalCount
        );

    /**
     * @brief 按需查询
     */
    virtual BOOL __cdecl Query(
        IFCacheQueryback* piQueryback,
        FCacheQueryType queryType,
        void* pParam1,
        void* pParam2
        );

    /**
     * @brief 清空Cache
     */
    virtual BOOL __cdecl Clean(
        );

    /**
     * @brief 获得文件的大小
     */
    virtual BOOL __cdecl GetFileSize(
        LPCWSTR lpFilePath,
        ULONGLONG& qwSize
        );

    /**
     * @brief 获得目录大小
     */
    virtual BOOL __cdecl GetDirInfo(
        LPCWSTR lpDir,
        ULONGLONG& qwSize,
        ULONGLONG& qwCount
        );

    /**
     * @brief 获得扩展名Top榜
     */
    virtual BOOL __cdecl QueryTopExt(
        IFCacheQueryback* piQueryback,
        int nTop
        );

    /**
     * @brief 记录扫描是否完整和扫描时间
     */
    virtual BOOL __cdecl SetCacheInfo(
        const SYSTEMTIME& scanTime,
        BOOL bFullCache
        );

    /**
     * @brief 获得扫描是否完整和扫描时间
     */
    virtual BOOL __cdecl GetCacheInfo(
        SYSTEMTIME& scanTime,
        BOOL& bFullCache
        );

protected:
    BOOL PrepareDbFile(char cVol);

private:
    ULONG m_dwRefCount;
    sqlite3* m_pDbConnect;
    CStringA m_strDbPath;
};

//////////////////////////////////////////////////////////////////////////
