#include "stdafx.h"
#include "fcacheimp.h"
#include <shlobj.h>
#include "kscbase/kscconv.h"

//////////////////////////////////////////////////////////////////////////

CFCacheImpl::CFCacheImpl(char cVol)
    : m_dwRefCount(0)
    , m_pDbConnect(NULL)
{
    if (!PrepareDbFile(cVol))
        throw "error";
}

CFCacheImpl::~CFCacheImpl()
{

}

//////////////////////////////////////////////////////////////////////////

BOOL CFCacheImpl::PrepareDbFile(char cVol)
{
    BOOL retval = FALSE;
    CHAR szAppData[MAX_PATH] = { 0 };
    CStringA strCacheName;
    BOOL bRetCode;

    strCacheName.Format("%c.cache", cVol);

    bRetCode = ::SHGetSpecialFolderPathA(NULL, szAppData, CSIDL_COMMON_APPDATA, TRUE);
    if (!bRetCode)
        goto clean0;

    ::PathAppendA(szAppData, "kingsoft");
    if (::GetFileAttributesA(szAppData) == INVALID_FILE_ATTRIBUTES)
    {
        ::CreateDirectoryA(szAppData, NULL);
    }

    ::PathAppendA(szAppData, "kclear");
    if (::GetFileAttributesA(szAppData) == INVALID_FILE_ATTRIBUTES)
    {
        ::CreateDirectoryA(szAppData, NULL);
    }

    ::PathAppendA(szAppData, "fcache");
    if (::GetFileAttributesA(szAppData) == INVALID_FILE_ATTRIBUTES)
    {
        ::CreateDirectoryA(szAppData, NULL);
    }

    ::PathAppendA(szAppData, strCacheName);
    m_strDbPath = szAppData;

    retval = TRUE;

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG) CFCacheImpl::AddRef()
{
    return ++m_dwRefCount;
}

STDMETHODIMP_(ULONG) CFCacheImpl::Release()
{
    ULONG retval = --m_dwRefCount;
    if (!retval)
        delete this;
    return retval;
}

STDMETHODIMP CFCacheImpl::QueryInterface(REFIID riid, void **ppvObject)
{
    HRESULT hr = E_FAIL;

    if (!ppvObject)
    {
        hr = E_INVALIDARG;
        goto clean0;
    }

    if (__uuidof(IFCache) == riid)
    {
        *ppvObject = static_cast<IFCache*>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        goto clean0;
    }
    
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();

    hr = S_OK;

clean0:
    return hr;
}

//////////////////////////////////////////////////////////////////////////

BOOL CFCacheImpl::Initialize()
{
    BOOL retval = FALSE;
    int nRetCode = -1;
    char* szError = NULL;

    nRetCode = sqlite3_open(m_strDbPath, &m_pDbConnect);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_exec(m_pDbConnect,
                           "create table files(path TEXT, ext TEXT, size BIGINT)",
                           NULL,
                           NULL,
                           &szError);

    nRetCode = sqlite3_exec(m_pDbConnect,
                            "create table exts(ext TEXT, size BIGINT, count BIGINT)",
                            NULL,
                            NULL,
                            &szError);

    nRetCode = sqlite3_exec(m_pDbConnect,
                            "create table top100(path TEXT, size BIGINT)",
                            NULL,
                            NULL,
                            &szError);

    nRetCode = sqlite3_exec(m_pDbConnect,
                            "create table info(size BIGINT, count BIGINT)",
                            NULL,
                            NULL,
                            &szError);

    nRetCode = sqlite3_exec(m_pDbConnect,
                            "create table cache(time BIGINT, full INTEGER)",
                            NULL,
                            NULL,
                            &szError);

    retval = TRUE;

clean0:
    return retval;
}

BOOL CFCacheImpl::UnInitialize()
{
    if (m_pDbConnect)
    {
        sqlite3_close(m_pDbConnect);
        m_pDbConnect = NULL;
    }

    return TRUE;
}

BOOL CFCacheImpl::BeginAdd()
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;

    if (!m_pDbConnect)
        goto clean0;

    nRetCode = sqlite3_exec(m_pDbConnect, 
                            "begin transaction", 
                            NULL, 
                            NULL, 
                            &szError);
    if (nRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CFCacheImpl::EndAdd()
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;

    if (!m_pDbConnect)
        goto clean0;

    nRetCode = sqlite3_exec(m_pDbConnect, 
                            "commit transaction", 
                            NULL, 
                            NULL, 
                            &szError);
    if (nRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CFCacheImpl::CancelAdd()
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;

    if (!m_pDbConnect)
        goto clean0;

    nRetCode = sqlite3_exec(m_pDbConnect, 
                            "rollback transaction", 
                            NULL, 
                            NULL, 
                            &szError);
    if (nRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CFCacheImpl::AddFile(
    LPCWSTR lpFilePath,
    ULONGLONG qwFileSize
    )
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    CStringW strFilePath;
    CStringW strExt;
    int nExt;

    if (!m_pDbConnect)
        goto clean0;

    if (!lpFilePath)
        goto clean0;

    strFilePath = lpFilePath;
    strFilePath.MakeLower();

    nExt = strFilePath.ReverseFind(_T('.'));
    if (nExt != -1 && nExt > strFilePath.ReverseFind(_T('\\')))
    {
        strExt = strFilePath.Right(strFilePath.GetLength() - nExt);
    }

    if (strExt.IsEmpty())
    {
        strExt = _T(".n/a");
    }

    strSql.Format("insert into files values('%s', '%s', %I64d)",
                  KUTF16_To_UTF8(strFilePath),
                  KUTF16_To_UTF8(strExt),
                  qwFileSize);
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);
    if (nRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CFCacheImpl::DelFile(LPCWSTR lpFilePath)
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    CStringW strFilePath;
    int nExt;
    CStringW strExt;
    sqlite3_stmt* pStmt = NULL;
    ULONGLONG qwSize;

    if (!m_pDbConnect)
        goto clean0;

    if (!lpFilePath)
        goto clean0;

    strFilePath = lpFilePath;
    strFilePath.MakeLower();

    nExt = strFilePath.ReverseFind(_T('.'));
    if (nExt != -1 && nExt > strFilePath.ReverseFind(_T('\\')))
    {
        strExt = strFilePath.Right(strFilePath.GetLength() - nExt);
    }

    if (strExt.IsEmpty())
    {
        strExt = _T(".n/a");
    }

    // 获得大小
    strSql.Format("select size from files where path = '%s'", KUTF16_To_UTF8(strFilePath));
    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (SQLITE_ROW != nRetCode)
        goto clean0;

    qwSize = sqlite3_column_int64(pStmt, 0);

    // 删除文件
    strSql.Format("delete from files where path = '%s'", KUTF16_To_UTF8(strFilePath));
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);
    if (nRetCode)
        goto clean0;

    // 从Top100中删除
    strSql.Format("delete from top100 where path = '%s'", KUTF16_To_UTF8(strFilePath));
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);

    // 从Exts中去除大小
    strSql.Format("insert into exts values('%s', -%I64d, -1)", KUTF16_To_UTF8(strExt), qwSize);
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);

    // 从总大小中去除大小
    strSql.Format("insert into info values(-%I64d, -1)", qwSize);
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);

    retval = TRUE;

clean0:
    if (pStmt)
    {
        sqlite3_finalize(pStmt);
        pStmt = NULL;
    }

    return retval;
}

BOOL CFCacheImpl::DelDir(LPCWSTR lpDir)
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    CStringW strDir;

    if (!m_pDbConnect)
        goto clean0;

    if (!lpDir)
        goto clean0;

    strDir = lpDir;
    strDir.MakeLower();

    strSql.Format("delete from files where path like '%s%s'", 
                  KUTF16_To_UTF8(strDir),
                  "%");
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);
    if (nRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CFCacheImpl::GetTotalInfo(ULONGLONG& qwTotalSize, ULONGLONG& qwTotalCount)
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    sqlite3_stmt* pStmt = NULL;

    if (!m_pDbConnect)
        goto clean0;

    strSql = "select sum(size), sum(count) from info";
    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (SQLITE_ROW == nRetCode)
    {
        qwTotalSize = sqlite3_column_int64(pStmt, 0);
        qwTotalCount = sqlite3_column_int64(pStmt, 1);

        if (qwTotalCount && qwTotalSize)
        {
            retval = TRUE;
            goto clean0;
        }
    }

    sqlite3_finalize(pStmt);
    pStmt = NULL;

    strSql = "insert into info(size, count) select sum(size), count(size) from files";
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);
    if (nRetCode)
        goto clean0;

    strSql = "select size, count from info";
    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (SQLITE_ROW != nRetCode)
        goto clean0;

    qwTotalSize = sqlite3_column_int64(pStmt, 0);
    qwTotalCount = sqlite3_column_int64(pStmt, 1);

    retval = TRUE;

clean0:
    if (pStmt)
    {
        sqlite3_finalize(pStmt);
        pStmt = NULL;
    }

    return retval;
}

BOOL CFCacheImpl::Query(
    IFCacheQueryback* piQueryback,
    FCacheQueryType queryType,
    void* pParam1,
    void* pParam2
    )
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    sqlite3_stmt* pStmt = NULL;

    if (!m_pDbConnect)
        goto clean0;

    if (!piQueryback)
        goto clean0;

    if (enumFQT_Top == queryType)
    {
        if (!pParam1)
            goto clean0;

        strSql.Format("select path, size from top100 order by size desc limit %d", *(ULONG*)pParam1);
        nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
        if (nRetCode)
            goto clean0;

        nRetCode = sqlite3_step(pStmt);
        if (SQLITE_ROW == nRetCode)
            goto get_data;

        sqlite3_finalize(pStmt);
        pStmt = NULL;

        strSql = "delete from top100";
        nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);

        strSql = "insert into top100(path, size) select path, size from files order by size desc limit 500";
        nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);
        if (nRetCode)
            goto clean0;

        strSql.Format("select path, size from top100 order by size desc limit %d", *(ULONG*)pParam1);
    }
    else if (enumFQT_Ext == queryType)
    {
        CStringA strExt;

        if (!pParam1)
            goto clean0;

        if (*(char*)pParam1 == '.')
        {
            strExt = (char*)pParam1;
        }
        else
        {
            strExt = ".";
            strExt += (char*)pParam1;
        }

        strSql.Format("select path, size from files where ext = '%s' order by size desc", strExt);
    }
    else if (enumFQT_Zone == queryType)
    {
        ULONGLONG qwFrom, qwTo;

        if (!pParam1 || !pParam2)
            goto clean0;

        qwFrom = *(ULONGLONG*)pParam1;
        qwTo = *(ULONGLONG*)pParam2;

        strSql.Format("select path, size from files where size >= %I64d and size <= %I64d order by size desc",
                      qwFrom,
                      qwTo);
    }
    else if (enumFQT_Word == queryType)
    {
        CStringW strWord;

        if (!pParam1)
            goto clean0;

        strWord = (const wchar_t*)pParam1;

        strSql.Format("select path, size from files where path like '%s%s%s' order by size desc",
                      "%",
                      KUTF16_To_UTF8(strWord),
                      "%");
    }

    strSql.MakeLower();
    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (nRetCode != SQLITE_ROW)
        goto clean0;

get_data:
    while (SQLITE_ROW == nRetCode)
    {
        CStringA strFilePath = (char*)sqlite3_column_text(pStmt, 0);
        ULONGLONG qwFileSize = sqlite3_column_int64(pStmt, 1);

        piQueryback->OnData(queryType, KUTF8_To_UTF16(strFilePath), qwFileSize);

        nRetCode = sqlite3_step(pStmt);
    }

    retval = TRUE;

clean0:
    if (pStmt)
    {
        sqlite3_finalize(pStmt);
        pStmt = NULL;
    }

    return retval;
}

BOOL CFCacheImpl::Clean()
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;

    if (!m_pDbConnect)
        goto clean0;

    nRetCode = sqlite3_exec(m_pDbConnect, 
                            "delete from files", 
                            NULL, 
                            NULL, 
                            &szError);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_exec(m_pDbConnect, 
                            "delete from exts", 
                            NULL, 
                            NULL, 
                            &szError);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_exec(m_pDbConnect,
                            "delete from top100",
                            NULL,
                            NULL,
                            &szError);

    nRetCode = sqlite3_exec(m_pDbConnect,
                            "delete from info",
                            NULL,
                            NULL,
                            &szError);

    nRetCode = sqlite3_exec(m_pDbConnect,
                            "delete from cache",
                            NULL,
                            NULL,
                            &szError);

    retval = TRUE;

clean0:
    return retval;
}

BOOL CFCacheImpl::GetFileSize(
    LPCWSTR lpFilePath,
    ULONGLONG& qwSize
    )
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    CStringW strFilePath;
    sqlite3_stmt* pStmt = NULL;

    if (!m_pDbConnect)
        goto clean0;

    if (!lpFilePath)
        goto clean0;

    strFilePath = lpFilePath;
    strFilePath.MakeLower();

    strSql.Format("select size from files where path = '%s'",
                   KUTF16_To_UTF8(strFilePath));
    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (SQLITE_ROW != nRetCode)
        goto clean0;

    qwSize = sqlite3_column_int64(pStmt, 0);

    retval = TRUE;

clean0:
    if (pStmt)
    {
        sqlite3_finalize(pStmt);
        pStmt = NULL;
    }

    return retval;
}   

BOOL CFCacheImpl::GetDirInfo(
    LPCWSTR lpDir,
    ULONGLONG& qwSize,
    ULONGLONG& qwCount
    )
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    CStringW strDir;
    sqlite3_stmt* pStmt = NULL;

    if (!m_pDbConnect)
        goto clean0;

    if (!lpDir)
        goto clean0;

    strDir = lpDir;
    strDir.MakeLower();

    strSql.Format("select sum(size), count(size) from files where path like '%s\\%s'",
                  KUTF16_To_UTF8(strDir),
                  "%");
    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (SQLITE_ROW != nRetCode)
        goto clean0;

    qwSize = sqlite3_column_int64(pStmt, 0);
    qwCount = sqlite3_column_int64(pStmt, 1);

    retval = TRUE;

clean0:
    if (pStmt)
    {
        sqlite3_finalize(pStmt);
        pStmt = NULL;
    }

    return retval;
}

BOOL CFCacheImpl::QueryTopExt(
    IFCacheQueryback* piQueryback,
    int nTop
    )
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    sqlite3_stmt* pStmt = NULL;

    if (!m_pDbConnect)
        goto clean0;

    if (!piQueryback)
        goto clean0;

    strSql.Format("select ext, sum(size), sum(count) from exts group by ext order by sum(size) desc");
    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (nRetCode == SQLITE_ROW)
        goto get_data;

    sqlite3_finalize(pStmt);
    pStmt = NULL;

    strSql.Format("insert into exts(ext, size, count) select ext, sum(size), count(size) from files group by ext order by sum(size) desc limit %d",
                  nTop);

    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);
    if (nRetCode)
        goto clean0;

    strSql.Format("select ext, size, count from exts");

    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (nRetCode != SQLITE_ROW)
        goto clean0;

get_data:
    while (SQLITE_ROW == nRetCode)
    {
        CStringA strExt = (char*)sqlite3_column_text(pStmt, 0);
        ULONGLONG qwSize = sqlite3_column_int64(pStmt, 1);
        ULONGLONG qwCount = sqlite3_column_int64(pStmt, 2);

        if (strExt.CompareNoCase(".n/a") == 0)
        {
            strExt = "N/A";
        }

        piQueryback->OnExtData(KUTF8_To_UTF16(strExt), qwSize, qwCount);

        nRetCode = sqlite3_step(pStmt);
    }

    retval = TRUE;

clean0:
    if (pStmt)
    {
        sqlite3_finalize(pStmt);
        pStmt = NULL;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

BOOL CFCacheImpl::SetCacheInfo(
    const SYSTEMTIME& scanTime,
    BOOL bFullCache
    )
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    FILETIME fileTime;
    LARGE_INTEGER llTime;

    if (!m_pDbConnect)
        goto clean0;

    SystemTimeToFileTime(&scanTime, &fileTime);
    llTime.LowPart = fileTime.dwLowDateTime;
    llTime.HighPart = fileTime.dwHighDateTime;

    strSql = "delete from cache";
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);

    strSql.Format("insert into cache values(%I64d, %d)", 
                  llTime.QuadPart,
                  bFullCache ? 1 : 0);
    nRetCode = sqlite3_exec(m_pDbConnect, strSql, NULL, NULL, &szError);
    if (nRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CFCacheImpl::GetCacheInfo(
    SYSTEMTIME& scanTime,
    BOOL& bFullCache
    )
{
    BOOL retval = FALSE;
    int nRetCode;
    char* szError = NULL;
    CStringA strSql;
    FILETIME fileTime;
    LARGE_INTEGER llTime;
    sqlite3_stmt* pStmt = NULL;

    if (!m_pDbConnect)
        goto clean0;

    strSql = "select time, full from cache";
    nRetCode = sqlite3_prepare(m_pDbConnect, strSql, -1, &pStmt, 0);
    if (nRetCode)
        goto clean0;

    nRetCode = sqlite3_step(pStmt);
    if (SQLITE_ROW != nRetCode)
        goto clean0;

    llTime.QuadPart = sqlite3_column_int64(pStmt, 0);
    bFullCache = sqlite3_column_int(pStmt, 1) ? TRUE : FALSE;

    fileTime.dwLowDateTime = llTime.LowPart;
    fileTime.dwHighDateTime = llTime.HighPart;
    FileTimeToSystemTime(&fileTime, &scanTime);

    retval = TRUE;

clean0:
    if (pStmt)
    {
        sqlite3_finalize(pStmt);
        pStmt = NULL;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////
