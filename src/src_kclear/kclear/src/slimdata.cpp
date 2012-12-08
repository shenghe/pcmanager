//////////////////////////////////////////////////////////////////////////
// Creator: Leo Jiang<JiangFengbing@gmail.com>

#include "stdafx.h"
#include "slimdata.h"
#include "kclear/libheader.h"
#include "kscbase/kscconv.h"
#include "kscbase/kscsys.h"
#include "tinyxml/tinyxml.h"
#include "kclearmsg.h"
#include "slimhelper.h"
#include "misc/kregfunction.h"

//////////////////////////////////////////////////////////////////////////

SlimItem::SlimItem()
{
    m_hNotifyWnd    = NULL;
    m_hWorkThread   = NULL;
    m_bEnable       = FALSE;
    m_nMethod       = SLIM_DELETE_FILE;
    m_piCallback    = NULL;
    m_qwSizeOnDisk  = 0;
    m_qwSizeMaySave = 0;
    m_qwSizeSaved   = 0;
    m_bStopScanFlag = FALSE;
    m_bUndoEnable   = FALSE;
}

SlimItem::SlimItem(const SlimItem& item)
{
    m_nId = item.Id();
    m_strName = item.Name();
    m_strDescription = item.Description();
    m_strWarning = item.Warning();
    m_bEnable = item.IsEnable();
    m_nMethod = item.Method();
    m_hNotifyWnd = NULL;
    m_hWorkThread = NULL;
    m_piCallback = item.Callback();
    m_qwSizeOnDisk = item.SizeOnDisk();
    m_qwSizeMaySave = item.SizeMaySave();
    m_qwSizeSaved = item.SizeSaved();
    item.GetPaths(m_itemPaths);
    m_bStopScanFlag = FALSE;
    m_bUndoEnable = UndoEnable();
}

SlimItem::~SlimItem()
{
}

//////////////////////////////////////////////////////////////////////////

int SlimItem::Id() const
{
    return m_nId;
}

void SlimItem::SetId(int nId)
{
    m_nId = nId;
}

CString SlimItem::Name() const
{
    return m_strName;
}

void SlimItem::SetName(const CString& strName)
{
    m_strName = strName;
}

CString SlimItem::Description() const
{
    return m_strDescription;
}

void SlimItem::SetDescription(const CString& strDescription)
{
    m_strDescription = strDescription;
}

CString SlimItem::Warning() const
{
    return m_strWarning;
}

void SlimItem::SetWarning(const CString& strWarning)
{
    m_strWarning = strWarning;
}

void SlimItem::InsertPath(const CString& strPath, const CStringA& strOsFlag)
{
    CString strRealPath;
    TCHAR* temp = new TCHAR[MAX_PATH * 2];
    WinVersion sysVersion = KGetWinVersion();

    if (!strOsFlag.CompareNoCase("xp") && sysVersion >= WINVERSION_VISTA)
        return;

    if (!strOsFlag.CompareNoCase("vista") && sysVersion != WINVERSION_VISTA)
        return;

    if (!strOsFlag.CompareNoCase("win7") && sysVersion <= WINVERSION_VISTA)
        return;

    if (!ExpandEnvironmentStrings(strPath, temp, MAX_PATH * 2))
        return;

    strRealPath = temp;

    if (strRealPath.Find(L"*") != -1)
    {
        _FindPath(strRealPath);
    } 
    else if (::GetFileAttributes(strRealPath) != INVALID_FILE_ATTRIBUTES)
    {    
        m_itemPaths.Add(strRealPath);
    }
}
void SlimItem::_FindPath(const CString& strPath)
{
    TCHAR szScanPath[MAX_PATH] = {0};
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA findData;

    wcscpy(szScanPath, strPath);

    PathRemoveFileSpec(szScanPath);

    hFind = ::FindFirstFile(strPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
        goto Clear0;

    do 
    {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  
        {
            CString strFileName = findData.cFileName;
            if (strFileName.GetLength() >= 2 &&
                strFileName[0] == L'$' && 
                strFileName[strFileName.GetLength() - 1] == L'$')
            {
                CString strRealPath;
                strRealPath += szScanPath;
                strRealPath += L"\\";
                strRealPath += findData.cFileName;
                m_itemPaths.Add(strRealPath);
            }
        }
    } while (::FindNextFile(hFind, &findData));

Clear0:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
    return;
}


void SlimItem::GetPaths(CAtlArray<CString>& itemPaths) const
{
    itemPaths.RemoveAll();
    for (size_t i = 0; i < m_itemPaths.GetCount(); i++)
    {
        itemPaths.Add(m_itemPaths[i]);
    }
}

BOOL SlimItem::IsEnable() const
{
    return m_bEnable;
}

void SlimItem::CheckEnable()
{
    m_bEnable = FALSE;

    if (SLIM_DELETE_FILE == m_nMethod)
    {
        // 如果是目录，判断目录里面是否有标记文件，如果不是则评测文件是否存在
        if (m_qwSizeOnDisk > 0 && m_itemPaths.GetCount() > 0)
            m_bEnable = TRUE;
//         BOOL bExist = FALSE;
//         size_t nCount = m_itemPaths.GetCount();
//         for (size_t i = 0; i < nCount; i++)
//         {
//             DWORD dwAttributes = ::GetFileAttributes(m_itemPaths[i]);
// 
//             if (INVALID_FILE_ATTRIBUTES == dwAttributes)
//             {
//                 continue;
//             }
//             if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
//             {
//                 CString strFlagFile = m_itemPaths[i];
//                 if (strFlagFile[strFlagFile.GetLength() - 1] != _T('\\'))
//                     strFlagFile += _T("\\");
// 
//                 strFlagFile += slimhelper::g_kSlimFlag;
//                 if (::GetFileAttributes(strFlagFile) == INVALID_FILE_ATTRIBUTES)
//                 {
//                     if (m_qwSizeOnDisk > 1 * 1024 * 1024 || (m_nId == 11 || m_nId == 10 && m_qwSizeOnDisk > 0) )
//                         m_bEnable = TRUE;
// 
//                     goto clean0;
//                 }
//                 else if ((m_nId == 11 || 
//                           m_nId == 5 || 
//                           m_nId == 6 || 
//                           m_nId == 7 || 
//                           m_nId == 8 ||
//                           m_nId == 0 ||
//                           m_nId == 9
//                           ) && m_qwSizeOnDisk > 0)  // 软件安装补丁
//                 {
//                     m_bEnable = TRUE;
//                     goto clean0;
//                 }
//             }
//             else
//             {
//                 if (m_qwSizeOnDisk > 0)
//                     m_bEnable = TRUE;
//                 goto clean0;
//             }
//        }

    }
    else if (SLIM_COMPRESS_FILE == m_nMethod)
    {
        // 判断最上层的目录是否被压缩
        for (size_t i = 0; i < m_itemPaths.GetCount(); i++)
        {
            BOOL bCompressed = FALSE;
            if (slimhelper::IsCompressed(m_itemPaths[i], bCompressed))
            {
                if (!bCompressed)
                {
                    m_bEnable = TRUE;
                    goto clean0;
                }
            }
        }
    }

clean0:
    return;
}

BOOL SlimItem::UndoEnable() const
{
    return m_bUndoEnable;
}

void SlimItem::SetUndoEnable(BOOL bEnable)
{
    m_bUndoEnable = bEnable;
}

SlimMethod SlimItem::Method() const
{
    return m_nMethod;
}

void SlimItem::SetMethod(SlimMethod nMethod)
{
    m_nMethod = nMethod;
}

BOOL SlimItem::Valid() const
{
    if (m_itemPaths.GetCount())
        return TRUE;
    else
        return FALSE;
}

ULONGLONG SlimItem::SizeOnDisk() const
{
    return m_qwSizeOnDisk;
}

ULONGLONG SlimItem::SizeMaySave() const
{
    return m_qwSizeMaySave;
}

ULONGLONG SlimItem::SizeSaved() const
{
    return m_qwSizeSaved;
}

ISystemSlimCallBack* SlimItem::Callback() const
{
    return m_piCallback;
}

void SlimItem::SetCallback(ISystemSlimCallBack* piCallback)
{
    m_piCallback = piCallback;
}

BOOL SlimItem::BeginProcess(HWND hNotifyWnd)
{
    BOOL retval = FALSE;

    if (!hNotifyWnd)
        goto clean0;

    m_hNotifyWnd = hNotifyWnd;

    m_bStopScanFlag = FALSE;

    m_hWorkThread = (HANDLE)_beginthreadex(NULL, 0, ProcessThreadProc, this, 0, NULL);
    if (!m_hWorkThread)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

void SlimItem::CancelProcess()
{
    m_bStopScanFlag = TRUE;
}

UINT SlimItem::ProcessThreadProc(void* pParam)
{
    SlimItem* pThis = (SlimItem*)pParam;
    SlimMethod nMethod;
    BOOL bRetCode = TRUE;
    
    if (!pThis)
        goto clean0;

    nMethod = pThis->Method();
    switch (nMethod)
    {
    case SLIM_DELETE_FILE:
        bRetCode = pThis->DeletePaths();
        break;

    case SLIM_COMPRESS_FILE:
        pThis->CompressPaths();
        break;
    }

clean0:
    if (pThis)
    {
        pThis->CheckEnable();
        ::PostMessage(pThis->m_hNotifyWnd, SLIM_WM_ITEM_TREATED, bRetCode, 0);
    }

    _endthreadex(0);
    return 0;
}

BOOL SlimItem::DeletePaths()
{
    BOOL retval = FALSE;
    DWORD dwAttributes;
    CAtlArray<CString> itemPaths;
    BOOL bRetCode;

    GetPaths(itemPaths);

    for (size_t i = 0; i < itemPaths.GetCount(); i++)
    {

        if (m_bStopScanFlag)
            goto clean0;

        dwAttributes = ::GetFileAttributes(itemPaths[i]);
        if (INVALID_FILE_ATTRIBUTES == dwAttributes)
            continue;

        if (m_bUndoEnable)
        {
            if (!this->OnBeginProcessItem(itemPaths[i]))
                continue;

            BOOL bKeepRootDir = TRUE;
            if (m_nId == 10) // 系统升级补丁备份文件
            {
                bKeepRootDir = FALSE;
            }

            bRetCode = slimhelper::RecyclePath(itemPaths[i], bKeepRootDir);
            this->OnEndProcessItem(itemPaths[i], 0);
            if (!bRetCode)
                goto clean0;
        }
        else
        {
            if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                BOOL bKeepRootDir = TRUE;
                if (m_nId == 10) // 系统升级补丁备份文件
                {
                    bKeepRootDir = FALSE;
                }
                slimhelper::DeleteDirectory(itemPaths[i], this, bKeepRootDir);
            }
            else
            {
                slimhelper::DeleteFile(itemPaths[i], 0, INVALID_FILE_ATTRIBUTES, this);
            }
        }
    }

    if (m_bUndoEnable)
    {
        this->OnEndProcessItem(_T(""), m_qwSizeMaySave);
    }

    if (!m_bStopScanFlag)
    {
        m_qwSizeOnDisk = 0;
        m_qwSizeMaySave = 0;
    }

    retval = TRUE;

clean0:
    return retval;
}

BOOL SlimItem::CompressPaths()
{
    DWORD dwAttributes;
    CAtlArray<CString> itemPaths;

    GetPaths(itemPaths);

    for (size_t i = 0; i < itemPaths.GetCount(); i++)
    {
        dwAttributes = ::GetFileAttributes(itemPaths[i]);
        if (INVALID_FILE_ATTRIBUTES == dwAttributes)
            continue;

        if (dwAttributes & FILE_ATTRIBUTE_COMPRESSED
            || dwAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
        {
            continue;
        }

        if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            slimhelper::CompressDirectory(itemPaths[i], this);
        }
        else
        {
            slimhelper::CompressFile(itemPaths[i], 0, dwAttributes, this);
        }
    }

    return TRUE;
}

void SlimItem::BeginScan(HWND hNotifyWnd)
{
    HANDLE hScanThread = NULL;

    m_hNotifyWnd = hNotifyWnd;
    m_qwSizeOnDisk = 0;
    m_qwSizeMaySave = 0;
    m_qwSizeSaved = 0;
    m_bStopScanFlag = FALSE;
    hScanThread = (HANDLE)_beginthreadex(NULL, 0, ScanThreadProc, this, 0, NULL);
    if (hScanThread)
    {
        ::CloseHandle(hScanThread);
        hScanThread = NULL;
    }
}

void SlimItem::StopScan()
{
    m_bStopScanFlag = TRUE;
}

void SlimItem::GetCurrentFile(CString& strFilePath)
{
    m_lock.Acquire();
    strFilePath = m_strCurrentProcess;
    m_lock.Release();
}

KLockHandle SlimItem::Lock() const
{
    return m_lock;
}

void SlimItem::SetLock(KLockHandle lock)
{
    m_lock = lock;
}

UINT SlimItem::ScanThreadProc(void* pParam)
{
    SlimItem* pThis = (SlimItem*)pParam;

    if (!pThis)
        goto clean0;

    pThis->ScanPathSync();

clean0:
    _endthreadex(0);
    return 0;
}

void SlimItem::ScanPathSync()
{
    DWORD dwAttributes;
    CAtlArray<CString> itemPaths;

    GetPaths(itemPaths);

    for (size_t i = 0; i < itemPaths.GetCount(); i++)
    {
        dwAttributes = ::GetFileAttributes(itemPaths[i]);
        if (INVALID_FILE_ATTRIBUTES == dwAttributes)
            continue;

        if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (!slimhelper::ScanDirectory(itemPaths[i], this))
                break;
        }
        else
        {
            slimhelper::ScanFile(itemPaths[i], 0, INVALID_FILE_ATTRIBUTES, this);
        }
    }

    CheckEnable();
    ::PostMessage(m_hNotifyWnd, SLIM_WM_ITEM_SCAN_END, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
// 扫描某个具体的文件
BOOL SlimItem::OnScanItem(
    const wchar_t* szFilePath,
    ULONGLONG qwFileSizeOnDisk,
    BOOL bCompressed
    )
{
    BOOL bRet = FALSE;
    static CString g_FilterPath = L"";


    if (g_FilterPath.IsEmpty())
    {
        _GetWallpaperFilterPath(g_FilterPath);
    }
    if (_wcsicmp(g_FilterPath, szFilePath) == 0)
        goto clean0;

    m_qwSizeOnDisk += qwFileSizeOnDisk;

    if (SLIM_COMPRESS_FILE == m_nMethod)
    {
        if (!bCompressed)
            m_qwSizeMaySave += (ULONGLONG)(qwFileSizeOnDisk * 0.5); // 预计压缩可以节约一半空间
    }
    else
    {
        m_qwSizeMaySave += qwFileSizeOnDisk;
    }
    
    bRet = TRUE;

clean0: 
    return bRet;
}

// 处理某个具体的文件
BOOL SlimItem::OnBeginProcessItem(
    const wchar_t* szFilePath
    )
{
    BOOL bRet = FALSE;
    static CString g_FilterPath = L"";

    if (g_FilterPath.IsEmpty())
    {
        _GetWallpaperFilterPath(g_FilterPath);
    }
    if (_wcsicmp(g_FilterPath, szFilePath) == 0)
        goto clean0;

    m_lock.Acquire();
    m_strCurrentProcess = szFilePath;
    m_lock.Release();

    bRet = TRUE;
clean0:
    return bRet;
}

void SlimItem::_GetWallpaperFilterPath(CString& strPath)
{
    TCHAR temp[MAX_PATH * 2] = {0};
    TCHAR szLongPathBuffer[MAX_PATH * 2] = { 0 };
    DWORD len = sizeof(szLongPathBuffer);
    CString strValueName;

    if (KGetWinVersion() == WINVERSION_WIN7)
    {
        strValueName = L"WallpaperSource";
    }
    else
    {
        strValueName = L"Wallpaper";
    }

    // 过滤 当前壁纸
    GetRegistryValue(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Internet Explorer\\Desktop\\General",
        strValueName,
        NULL,
        (LPBYTE)szLongPathBuffer,
        &len
        );

    if (ExpandEnvironmentStrings(szLongPathBuffer, temp, MAX_PATH * 2))
    {
        strPath = temp;
    }
    else
    {
        strPath = szLongPathBuffer;
    }

    if (strPath.IsEmpty())
    {
        strPath = L"NULL";
    }
}

//////////////////////////////////////////////////////////////////////////
BOOL SlimItem::OnEndProcessItem(
    const wchar_t* szFilePath,
    ULONGLONG qwSavedSize
    )
{
    m_lock.Acquire();
    m_qwSizeSaved += qwSavedSize;
    m_lock.Release();

    return TRUE;
}

// 空闲和控制退出
BOOL SlimItem::OnIdle()
{
    if (m_bStopScanFlag)
        return FALSE;
    else
        return TRUE;
}

//////////////////////////////////////////////////////////////////////////

SlimData::SlimData() : m_hNotifyWnd(NULL)
{

}

SlimData::~SlimData()
{

}

//////////////////////////////////////////////////////////////////////////

BOOL SlimData::LoadData(HWND hNotifyWnd)
{
    BOOL retval = FALSE;
    HANDLE hThread = NULL;

    {
        KAutoLock lock(m_lock);
        m_hNotifyWnd = hNotifyWnd;
    }

    hThread = (HANDLE)_beginthreadex(NULL, 0, LoadDataThreadProc, this, 0, NULL);
    if (!hThread)
        goto clean0;

    retval = TRUE;

clean0:
    if (hThread)
    {
        ::CloseHandle(hThread);
    }

    return retval;
}

void SlimData::ClearData()
{
    KAutoLock lock(m_lock);
    m_slimItems.RemoveAll();
}

BOOL SlimData::IsEmpty()
{
    KAutoLock lock(m_lock);
    return m_slimItems.IsEmpty() ? TRUE : FALSE;
}

//////////////////////////////////////////////////////////////////////////

UINT SlimData::LoadDataThreadProc(void* pParam)
{
    SlimData* pThis = (SlimData*)pParam;

    if (pThis)
    {
        pThis->LoadDataSync();
    }

    _endthreadex(0);
    return 0;
}

BOOL SlimData::LoadDataSync()
{
    BOOL retval = FALSE;
    KAutoLock lock(m_lock);
    CStringA  strXml;
    TCHAR szModule[MAX_PATH] = { 0 };
    TiXmlDocument xmlDoc;
    TiXmlElement* pXmlSlims = NULL;
    TiXmlElement* pXmlItem = NULL;
    CDataFileLoader datloader;
    
    GetModuleFileName(NULL, szModule, MAX_PATH);
    PathRemoveFileSpec(szModule);
    PathAppend(szModule, _T("data\\slimdata.dat"));

    if (!datloader.LoadFile(szModule, strXml))
        goto clean0;

    xmlDoc.Parse(strXml);

    pXmlSlims = xmlDoc.FirstChildElement("slims");
    if (!pXmlSlims)
        goto clean0;

    pXmlItem = pXmlSlims->FirstChildElement("slim");
    while (pXmlItem)
    {
        TiXmlElement* pXmlId = pXmlItem->FirstChildElement("id");
        TiXmlElement* pXmlName = pXmlItem->FirstChildElement("name");
        TiXmlElement* pXmlDescription = pXmlItem->FirstChildElement("description");
        TiXmlElement* pXmlWarning = pXmlItem->FirstChildElement("warning");
        TiXmlElement* pXmlMethod = pXmlItem->FirstChildElement("method");

        if (pXmlId && pXmlName && pXmlDescription && pXmlWarning && pXmlMethod)
        {
            SlimItem item;
            TiXmlElement* pXmlPath = NULL;

            item.SetLock(m_sharedLock);
            item.SetId(::StrToIntA(pXmlId->GetText()));
            item.SetName(KUTF8_To_UTF16(pXmlName->GetText()));
            item.SetDescription(KUTF8_To_UTF16(pXmlDescription->GetText()));
            item.SetWarning(KUTF8_To_UTF16(pXmlWarning->GetText()));
            if (!stricmp(pXmlMethod->GetText(), "delete"))
            {
                item.SetMethod(SLIM_DELETE_FILE);
            }
            else if (!stricmp(pXmlMethod->GetText(), "compress"))
            {
                item.SetMethod(SLIM_COMPRESS_FILE);
            }

            pXmlPath = pXmlItem->FirstChildElement("path");
            while (pXmlPath)
            {
                CStringA strOsFlag = pXmlPath->Attribute("os");
                item.InsertPath(KUTF8_To_UTF16(pXmlPath->GetText()), strOsFlag);
                pXmlPath = pXmlPath->NextSiblingElement("path");
            }

            if (item.Valid() || (KGetWinVersion() < WINVERSION_VISTA && item.Id() == 10))
            {
                m_slimItems.Add(item);
            }
        }

        pXmlItem = pXmlItem->NextSiblingElement("slim");
    }

    retval = TRUE;

clean0:
    if (m_hNotifyWnd)
    {
        ::PostMessage(m_hNotifyWnd, SLIM_WM_LOADCOMPLETE, 0, 0);
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

CString SlimData::GetName(int nIndex)
{
    return m_slimItems[nIndex].Name();
}

int SlimData::GetCount()
{
    KAutoLock lock(m_lock);
    return (int)m_slimItems.GetCount();
}

SlimItem& SlimData::GetItem(int nIndex)
{
    KAutoLock lock(m_lock);
    return m_slimItems[nIndex];
}

int SlimData::GetValidCount()
{
    int retval = 0;
    KAutoLock lock(m_lock);

    for (size_t i = 0; i < m_slimItems.GetCount(); i++)
    {
        if (m_slimItems[i].Valid())
            retval += 1;
    }

    return retval;
}

int SlimData::GetEnalbeCount()
{
    int retval = 0;
    KAutoLock lock(m_lock);

    for (size_t i = 0; i < m_slimItems.GetCount(); i++)
    {
        if (m_slimItems[i].IsEnable())
            retval += 1;
    }

    return retval;
}

void SlimData::Scan(int nIndex, HWND hNotifyWnd)
{
    if (nIndex >= 0 && nIndex < (int)m_slimItems.GetCount())
    {
        m_slimItems[nIndex].BeginScan(hNotifyWnd);
    }
}

ULONGLONG SlimData::GetSizeMaySaved()
{
    ULONGLONG retval = 0;

    for (size_t i = 0; i < m_slimItems.GetCount(); i++)
    {
        retval += m_slimItems[i].SizeMaySave();
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////
