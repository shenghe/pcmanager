#include "stdafx.h"
#include "kscbase/kscfilepath.h"
#include "kscbase/kscconv.h"
#include "kscbase/kscsys.h"
#include "kscmaindlg.h"
#include "uihandler_bigfile.h"
#include "bigfilehelper.h"
#include <algorithm>
#include "misc/wildcmp.h"

//////////////////////////////////////////////////////////////////////////

GetFCache_t     g_pfnGetFCache = NULL;
static ULONG g_dwSizeLevel[5] = { 10, 20, 50, 100, 500 };

//////////////////////////////////////////////////////////////////////////
CBigfileFilter::CBigfileFilter()
{
    LoadFilter();
}

CBigfileFilter::~CBigfileFilter()
{

}

CBigfileFilter& CBigfileFilter::Instance()
{
    static CBigfileFilter sington;
    return sington;
}

BOOL CBigfileFilter::LoadFilter()
{
    BOOL bretval = FALSE;
    CStringA  strXml;
    TCHAR szModule[MAX_PATH] = { 0 };
    TiXmlDocument xmlDoc;
    TiXmlElement* pXmlSetting = NULL;
    TiXmlElement* pXmlItem = NULL; 
    m_vsysExt.clear();
    m_vsysFile.clear();
    m_vsysDir.clear();

    WCHAR cSysVol = towlower(bigfilehelper::GetSystemDrive());

    GetModuleFileName(NULL, szModule, MAX_PATH);
    PathRemoveFileSpec(szModule);
    PathAppend(szModule, _T("cfg\\bigfilerule.dat"));
    CDataFileLoader datloader;
    if(!datloader.LoadFile(szModule, strXml))
        goto Exit0;

    xmlDoc.Parse(strXml);

    pXmlSetting = xmlDoc.FirstChildElement("setting");
    if(!pXmlSetting)
        goto Exit0;

    pXmlItem = pXmlSetting->FirstChildElement("filter_ext");
    while (pXmlItem)
    {            
        CString strsrc;  
        CString strtmp;           
        strtmp = KUTF8_To_UTF16(pXmlItem->Attribute("src"));
        _wcslwr_s(strtmp.GetBuffer(), strtmp.GetLength() + 1);
        strsrc += strtmp; 
        m_vsysExt.push_back(strsrc);
        pXmlItem = pXmlItem->NextSiblingElement("filter_ext");     
    }

    pXmlItem = pXmlSetting->FirstChildElement("filter_file");
    while (pXmlItem)
    {
        CString strsrc;  
        CString strtmp;         
        strtmp = KUTF8_To_UTF16(pXmlItem->Attribute("src"));
        _wcslwr_s(strtmp.GetBuffer(), strtmp.GetLength() + 1);
        strsrc += strtmp; 
        m_vsysFile.push_back(strsrc);
        pXmlItem = pXmlItem->NextSiblingElement("filter_file");   
    }

    pXmlItem = pXmlSetting->FirstChildElement("filter_adv");
    while (pXmlItem)
    {
        CString strFilter;  
        CString strTmp;    
        
        strTmp = KUTF8_To_UTF16(pXmlItem->Attribute("src"));

        if (strTmp[0] == _T('\\'))
        {
            // 系统盘过滤
            CString strEnd = strTmp;
            strFilter += cSysVol;
            strFilter += _T(":");
            strFilter += strTmp;
        }
        else
        {
            strFilter = strTmp;
        }
        strFilter.MakeLower();
        m_vsysDir.push_back(strFilter);
        pXmlItem = pXmlItem->NextSiblingElement("filter_adv");   
    }
    bretval = TRUE;

Exit0:     
    return bretval;
}

BOOL CBigfileFilter::IsFileInFilter(const CString& strFilepath)
{
    BOOL bretval = FALSE;
    CString strExtName;
    CString strFileName;
    std::vector<CString>::iterator itext;
    std::vector<CString>::iterator itfile;
    std::vector<CString>::iterator itdir;
    if(strFilepath.IsEmpty())
        goto Exit0;

    strExtName = strFilepath.Mid(strFilepath.ReverseFind(L'.') + 1);
    strFileName = strFilepath.Mid(strFilepath.ReverseFind(L'\\') + 1);

    for(itext = m_vsysExt.begin(); itext != m_vsysExt.end(); ++itext)
    {
        if (0 == strExtName.CompareNoCase(*itext))
        {
            bretval  = TRUE;
            goto Exit0;
        }
                          
    }

    for(itfile =  m_vsysFile.begin(); itfile != m_vsysFile.end(); ++itfile)
    {
        if (0 == strFileName.CompareNoCase(*itfile))
        {
            bretval  = TRUE;
            goto Exit0;
        }
    }

    for(itdir = m_vsysDir.begin(); itdir != m_vsysDir.end(); ++itdir)
    {     
        const wchar_t* szWild = (const wchar_t*)(*itdir);
        const wchar_t* szFilePath =(const wchar_t*)strFilepath;
        int nRetCode = wildcmp(szWild, szFilePath);
        if (nRetCode)
        {
            bretval  = TRUE;
            goto Exit0;
        }
    }
    
Exit0:
    return bretval;

}

//////////////////////////////////////////////////////////////////////////

CVirtualDirManager::CVirtualDirManager()
    : m_ctrlFileList(this)
{
    m_nType         = enumVDT_Dir;
    m_hNotifyWnd    = NULL;
    m_cVolume       = '\0';
    m_qwTotalSize   = 0;
    m_qwTotalCount  = 0;
    m_nEnumState    = enumES_Notstart;
    m_bFilling      = FALSE; 
    m_hFillThread   = NULL;
    m_bStopScanFlag = FALSE;
    m_bStopFillFlag = FALSE;
    m_qwSizeFilter  = (ULONGLONG)g_dwSizeLevel[2] * 1024 * 1024;
    m_piFCache      = NULL;
    m_nDirType      = enumRDT_Other;
    m_bSysVolume    = FALSE;
    m_bEnableSizeFilter = FALSE;
    m_dwFileListLimit = 50;
    m_qwEnumedSize  = 0;
    m_bRemovable    = FALSE;
    m_bSysFilter    = TRUE;
}

CVirtualDirManager::~CVirtualDirManager()
{

}

//////////////////////////////////////////////////////////////////////////

HWND CVirtualDirManager::InitCtrl(HWND hParent)
{
    if ('\0' == m_cVolume)
        m_ctrlFileList.SetSysFilterEnable(FALSE);

    if (NULL == m_ctrlFileList.m_hWnd)
    {
        DWORD dwStyle = WS_CHILD|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SINGLESEL|LVS_OWNERDRAWFIXED;
        m_ctrlFileList.Create(hParent, NULL, NULL, dwStyle);
        DWORD dwExStyle = LVS_EX_FULLROWSELECT;
        dwExStyle |= m_ctrlFileList.GetExtendedListViewStyle();
        m_ctrlFileList.SetExtendedListViewStyle(dwExStyle);
        m_ctrlFileList.InsertColumn(0, L" 文件名", LVCFMT_LEFT, 140); 
        m_ctrlFileList.InsertColumn(1, L" 大小", LVCFMT_LEFT, 60); 
        m_ctrlFileList.InsertColumn(2, L" 文件路径", LVCFMT_LEFT, 300); 
        m_ctrlFileList.InsertColumn(3, L" 操作", LVCFMT_LEFT, 90); 
        m_ctrlFileList.SetNotifyHwnd(m_hNotifyWnd);
        m_ctrlFileList.SetFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));

        if (enumVDT_Dir == m_nType)
        {
            m_ctrlFileList.AddLink(_T("删除到回收站"));
        }
        else
        {
            m_ctrlFileList.AddLink(_T("定位文件"));
        }
        m_ctrlFileList.Init();
    }

    LoadCacheASync();

    return m_ctrlFileList;
}

CBigFileListCtrl& CVirtualDirManager::GetCtrl()
{
    return m_ctrlFileList;
}

void CVirtualDirManager::SetSizeFilter(int idx)
{
    if (idx >= 0 && idx < 5)
    {
        m_qwSizeFilter = (ULONGLONG)g_dwSizeLevel[idx] * 1024 * 1024;
    }
}

int CVirtualDirManager::GetSizeFilter()
{
    int retval = -1;
    int i;
    ULONG dwLevel = (ULONG)(m_qwSizeFilter / (1024 * 1024));

    for (i = 0; i < 5; ++i)
    {
        if (g_dwSizeLevel[i] == dwLevel)
        {
            retval = i;
        }
    }

    return retval;
}

void CVirtualDirManager::SetSizeFilterEnable(BOOL fEnable)
{
    m_bEnableSizeFilter = fEnable;
}

BOOL CVirtualDirManager::GetSizeFilterEnable()
{
    return m_bEnableSizeFilter;
}

void CVirtualDirManager::SetSysFilterEnable(BOOL bEnable)
{
    m_bSysFilter = bEnable;
}

BOOL CVirtualDirManager::GetSysFilterEnable()
{
    return m_bSysFilter;
}

void CVirtualDirManager::SetExtFilter(const CString& strExt)
{
    KAutoLock lock(m_lock);
    m_strExtFilter = strExt;
}

void CVirtualDirManager::GetExtFilter(CString& strExt)
{
    KAutoLock lock(m_lock);
    strExt = m_strExtFilter;
}

void CVirtualDirManager::SetListLimit(DWORD dwLimit)
{
    KAutoLock lock(m_lock);
    m_dwFileListLimit = dwLimit;
}

void CVirtualDirManager::SetDirType(VirtualDirType nType)
{
    m_nType = nType;
}

VirtualDirType CVirtualDirManager::GetDirType()
{
    return m_nType;
}

void CVirtualDirManager::SetRealDirType(RealDirType nType)
{
    m_nDirType = nType;
}

RealDirType CVirtualDirManager::GetRealDirType()
{
    return m_nDirType;
}

void CVirtualDirManager::SetName(const CString& strName)
{
    m_strName = strName; 
}

CString CVirtualDirManager::GetName()
{
    return m_strName;
}

void CVirtualDirManager::SetVolumeChar(CHAR cVol)
{
    m_cVolume = cVol;
    
    UpdateVolumeSizeInfo();
}

void CVirtualDirManager::UpdateVolumeSizeInfo()
{
    BOOL bRetCode;
    CString strDrv;
    DWORD dwSectorsPerCluster, dwBytesPerSector, dwNumberOfFreeClusters, dwTotalNumberOfClusters;

    strDrv.Format(_T("%C:\\"), m_cVolume);
    bRetCode = GetDiskFreeSpace(
        strDrv,
        &dwSectorsPerCluster,
        &dwBytesPerSector,
        &dwNumberOfFreeClusters,
        &dwTotalNumberOfClusters
        );
    ASSERT(bRetCode);

    m_qwVolTotalSize = (ULONGLONG)dwTotalNumberOfClusters * dwBytesPerSector * dwSectorsPerCluster;
    m_qwVolFreeSize = (ULONGLONG)dwNumberOfFreeClusters * dwBytesPerSector * dwSectorsPerCluster;
}

CHAR CVirtualDirManager::GetVolumeChar()
{
    return m_cVolume;
}

void CVirtualDirManager::SetSysVolume()
{
    m_bSysVolume = TRUE;
}

BOOL CVirtualDirManager::IsSysVolume()
{
    return m_bSysVolume;
}

void CVirtualDirManager::SetRemovable(BOOL bRemovable)
{
    m_bRemovable = bRemovable;
}

void CVirtualDirManager::GetStatus(CString& strStatus)
{
    KAutoLock lock(m_lock);

    if (enumVDT_Volume == m_nType)
    {
        if (enumES_Notstart == m_nEnumState ||
            enumES_Scanning == m_nEnumState)
        {
            strStatus.Format(
                _T("为您找出占用%C盘%s空间最大的100个文件，并可按类型对文件进行分类管理。"), 
                GetVolumeChar(),
                IsSysVolume() ? _T("(系统盘)") : _T("")
                );
        }
        else
        {
            CString strTotalSize, strFreeSize, strUseSize;
            CString strCacheInfo;
            UpdateVolumeSizeInfo();
            bigfilehelper::GetFileSizeString(m_qwVolTotalSize, strTotalSize);
            bigfilehelper::GetFileSizeString(m_qwVolFreeSize, strFreeSize);
            bigfilehelper::GetFileSizeString(m_qwVolTotalSize - m_qwVolFreeSize, strUseSize);
            GetCacheSummary(strCacheInfo);
            strStatus.Format(
                _T("%C盘%s空间总计：%s(已用%s，剩余%s)  %s"),
                GetVolumeChar(),
                IsSysVolume() ? _T("(系统盘)") : _T(""),
                strTotalSize,
                strUseSize,
                strFreeSize,
                strCacheInfo
                );
        }
    }
    else if (enumVDT_Dir == m_nType)
    {
        CString strNames[] = { _T("桌面"), _T("文档"), _T("下载") };

        if (enumES_Scanning == m_nEnumState)
        {
            strStatus.Format(
                _T("正在扫描%s目录，请稍候..."),
                strNames[(int)m_nDirType]
                );
        }
        else
        {
            if (m_vCacheFiles.size())
            {
                strStatus.Format(
                    //_T("在您的%s目录里存放了如下大文件，您可以选择删除或迁移这些文件，以节省系统盘空间。"),
                    _T("您的 %s 目录存放了如下文件，您可以对不需要的文件进行相应操作，节省硬盘空间。"),
                    strNames[(int)m_nDirType]
                    );
            }
            else
            {
                strStatus.Format(
                    _T("在您的%s目录里没有发现任何大文件，您可以点击“重新扫描”来刷新大文件列表。"),
                    strNames[(int)m_nDirType]
                    );
            }
        }
    }
}

void CVirtualDirManager::GetTopExts(std::vector<BigExtInfo>& vTopExts)
{
    IFCache* piFCache = NULL;
    std::vector<BigExtInfo>::iterator i;
    int nCount = 0;
    KAutoLock lock(m_lock);

    if (!m_qwTotalSize)
        return;

    for (i = m_vTopExts.begin(); i != m_vTopExts.end(); ++i)
    {
        i->nPercent = (UINT)((i->qwTotalSize * 100) / m_qwTotalSize);
        vTopExts.push_back(*i);

        if (++nCount > 10)
            break;
    }

    return;
}

void CVirtualDirManager::GetTop100Files(std::vector<BigFileInfo>& vTopFiles)
{
    KAutoLock lock(m_lock);
    std::vector<BigFileInfo>::iterator i;
    size_t nCount = 0;

    vTopFiles.clear();

    for (i = m_vTopFiles.begin(); i != m_vTopFiles.end(); ++i)
    {
        vTopFiles.push_back(*i);
        if (++nCount > 100)
            break;
    }
}

void CVirtualDirManager::GetTop100Size(ULONGLONG& qwSize)
{
    KAutoLock lock(m_lock);
    std::vector<BigFileInfo>::iterator i;
    size_t nCount = 0;
    qwSize = 0;

    for (i = m_vTopFiles.begin(); i != m_vTopFiles.end(); ++i)
    {
        qwSize += i->qwFileSize;
        if (++nCount > 100)
            break;
    }
}

void CVirtualDirManager::QueryFilesWithFilter(
    std::vector<BigFileInfo>& vFiles, 
    const CString& strExt, 
    ULONG dwSizeLimit
    )
{
    BOOL bIsTop100 = FALSE;
    std::vector<BigFileInfo>::iterator i;
    KAutoLock lock(m_lock);
    size_t nCount = 0;

    vFiles.clear();

    if (strExt.CompareNoCase(_T("top100")) == 0)
        bIsTop100 = TRUE;

    if (bIsTop100)
    {
        for (i = m_vTopFiles.begin(); i != m_vTopFiles.end(); ++i)
        {
            if (!dwSizeLimit ||
                (dwSizeLimit && i->qwFileSize >= dwSizeLimit))
            {
                vFiles.push_back(*i);
                if (++nCount > 100)
                    break;
            }
        }
    }
    else
    {
        if (m_strCacheExt.CompareNoCase(strExt) == 0)
        {
            if (!dwSizeLimit ||
                (dwSizeLimit && i->qwFileSize >= dwSizeLimit))
            {
                vFiles.push_back(*i);
            }
        }
        else
        {

        }
    }
}

void CVirtualDirManager::SetNotify(HWND hWnd)
{
    m_hNotifyWnd = hWnd;
}

IFCache* CVirtualDirManager::GetFCache()
{
    IFCache* retval = NULL;
    BOOL bRetCode;
    KAutoLock lock(m_lock);

    if (m_piFCache)
    {
        retval = m_piFCache;
        goto clean0;
    }

    if (!g_pfnGetFCache)
        goto clean0; 

    bRetCode = g_pfnGetFCache(__uuidof(IFCache), (void**)&retval, m_cVolume);
    if (!bRetCode)
        goto clean0;

    m_piFCache = retval;

clean0:
    return retval;
}

BOOL CVirtualDirManager::GenerateCacheForTopExts()
{
    BOOL retval = FALSE;
    IFCache* piFCache = NULL;
    BOOL bRetCode;

    piFCache = GetFCache();
    if (!piFCache)
        goto clean0;

    bRetCode = piFCache->Initialize();
    if (!bRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (piFCache)
    {
        piFCache->UnInitialize();
    }

    return retval;
}

BOOL CVirtualDirManager::GenerateCacheForTop100()
{
    BOOL retval = FALSE;
    IFCache* piFCache = NULL;
    BOOL bRetCode;
    ULONG dwTopCount = 100;

    piFCache = GetFCache();
    if (!piFCache)
        goto clean0;

    bRetCode = piFCache->Initialize();
    if (!bRetCode)
        goto clean0;

    bRetCode = piFCache->Query(
        this,
        enumFQT_Top,
        &dwTopCount,
        NULL
        );
    if (!bRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (piFCache)
    {
        piFCache->UnInitialize();
    }

    return retval;
}

BOOL CVirtualDirManager::GetSummaryInfo()
{
    BOOL retval = FALSE;
    IFCache* piFCache = NULL;
    BOOL bRetCode;

    piFCache = GetFCache();
    if (!piFCache)
        goto clean0;

    bRetCode = piFCache->Initialize();
    if (!bRetCode)
        goto clean0;

    bRetCode = piFCache->GetTotalInfo(m_qwTotalSize, m_qwTotalCount);
    if (!bRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (piFCache)
    {
        piFCache->UnInitialize();
    }

    return retval;
}

void CVirtualDirManager::OnData(
    FCacheQueryType nQueryType,
    LPCWSTR lpFilePath,
    ULONGLONG qwFileSize
    )
{
    if (!lpFilePath)
        goto clean0;

    if (enumFQT_Top == nQueryType)
    {
        BigFileInfo fileInfo;
        KAutoLock lock(m_lock);

        if (!m_bEnableSizeFilter)
        {
            fileInfo.strFilePath = lpFilePath;
            fileInfo.qwFileSize = qwFileSize;
            m_vTopFiles.push_back(fileInfo);
        }
        else
        {
            if (qwFileSize >= m_qwSizeFilter)
            {
                fileInfo.strFilePath = lpFilePath;
                fileInfo.qwFileSize = qwFileSize;
                m_vTopFiles.push_back(fileInfo);
            }
        }
    }

    if (enumFQT_Ext == nQueryType)
    {
        BigFileInfo fileInfo;
        KAutoLock lock(m_lock);

        if (!m_bEnableSizeFilter)
        {
            fileInfo.strFilePath = lpFilePath;
            fileInfo.qwFileSize = qwFileSize;
            m_vCacheFiles.push_back(fileInfo);
        }
        else
        {
            if (qwFileSize >= m_qwSizeFilter)
            {
                fileInfo.strFilePath = lpFilePath;
                fileInfo.qwFileSize = qwFileSize;
                m_vCacheFiles.push_back(fileInfo);
            }
        }
    }

clean0:
    return;
}

void CVirtualDirManager::OnExtData(
    LPCWSTR lpExt,
    ULONGLONG qwSize,
    ULONGLONG qwCount
    )
{
    BigExtInfo extInfo;
    KAutoLock lock(m_lock);

    if (!lpExt)
        goto clean0;

    extInfo.strExt = lpExt;
    extInfo.qwTotalSize = qwSize;
    extInfo.qwTotalCount = qwCount;
    extInfo.nPercent = m_qwTotalSize ? int(qwSize * 100 / m_qwTotalSize) : 0;
    m_vTopExts.push_back(extInfo);

clean0:
    return;
}

void CVirtualDirManager::GetCacheList(
    std::vector<BigFileInfo>& vCacheList, 
    size_t nLimit,
    size_t& nMore
    )
{
    KAutoLock lock(m_lock);
    size_t nCopySize;
    std::vector<CString>::iterator i;
    std::vector<BigFileInfo>::iterator j;

    // 清理Cache
    for (i = m_vDeleteCache.begin(); i != m_vDeleteCache.end(); ++i)
    {
        j = std::find(m_vCacheFiles.begin(), m_vCacheFiles.end(), (LPCTSTR)(*i));
        if (j != m_vCacheFiles.end())
            m_vCacheFiles.erase(j);
    }
    m_vDeleteCache.clear();

    if (0 == nLimit)
    {
        nCopySize = m_vCacheFiles.size();
        nMore = 0;
    }
    else
    {
        nCopySize = m_vCacheFiles.size() > nLimit ? nLimit : m_vCacheFiles.size();
        nMore = m_vCacheFiles.size() > nLimit ? m_vCacheFiles.size() - nLimit : 0;
    }
    
    std::sort(m_vCacheFiles.rbegin(), m_vCacheFiles.rend());

    vCacheList.clear();
    vCacheList.resize(nCopySize);
    std::copy(m_vCacheFiles.begin(), m_vCacheFiles.begin() + nCopySize, vCacheList.begin());
}

void CVirtualDirManager::CopyTopListToCacheList()
{
    KAutoLock lock(m_lock);
    size_t nCopySize;

    nCopySize = m_vTopFiles.size();
    m_vCacheFiles.clear();
    if (!m_bEnableSizeFilter)
    {
        m_vCacheFiles.resize(nCopySize);
        std::copy(m_vTopFiles.begin(), m_vTopFiles.end(), m_vCacheFiles.begin());
    }
    else
    {
        for (size_t i = 0; i < m_vTopFiles.size(); ++i)
        {
            if (m_vTopFiles[i].qwFileSize > m_qwSizeFilter)
                m_vCacheFiles.push_back(m_vTopFiles[i]);
        }
    }
    
}

//////////////////////////////////////////////////////////////////////////

// void CVirtualDirManager::Lock()
// {
//     m_lock.Acquire();
// }
// 
// void CVirtualDirManager::UnLock()
// {
//     m_lock.Release();
// }

BOOL CVirtualDirManager::IsFilterFile(const CString& strFile)
{
    BOOL retval = FALSE;
    CString strTemp = strFile;
    CString strSigns[3] = 
    {
        _T("\\$recycle.bin\\"),
        _T("\\recycler\\"),
        _T("\\desktop.ini")
    };
    size_t i;

    strTemp.MakeLower();
    for (i = 0; i < 3; ++i)
    {
        if (strTemp.Find(strSigns[i]) != -1)
        {
            retval = TRUE;
            break;
        }
    }

    return retval;
}

BOOL CVirtualDirManager::OnFindFile(const BigFileInfo& fileInfo)
{
    BOOL retval = FALSE;
    IFCache* piFCache = NULL;
    BOOL bRetCode;

    {
        KAutoLock lock(m_lockStatus);
        m_strCurrentFile = fileInfo.strFilePath;
        m_qwEnumedSize += fileInfo.qwFileSize;
    }

    if (m_bStopScanFlag)
        goto clean0;

    if (IsFilterFile(fileInfo.strFilePath))
    {
        retval = TRUE;
        goto clean0;
    }

    if (enumVDT_Dir == m_nType)
    {
        // 自行保存
        KAutoLock lock(m_lockStatus);
        m_vCacheFiles.push_back(fileInfo);
    }

    if (enumVDT_Volume == m_nType)
    {
        piFCache = GetFCache();
        if (!piFCache)
            goto clean0;

        bRetCode = piFCache->AddFile(fileInfo.strFilePath, fileInfo.qwFileSize);
        //ASSERT(bRetCode); 有部分文件名有特殊符号，插入不了数据库
    }

    retval = TRUE;

clean0:
    return retval;
}

void CVirtualDirManager::BeginEnum()
{
    IFCache* piFCache = NULL;
    BOOL bRetCode;

    {
        KAutoLock lock(m_lock);
        m_qwEnumedSize = 0;
        m_nEnumState = enumES_Scanning;
    }

    if (enumVDT_Dir == m_nType)
    {
        KAutoLock lock(m_lock);
        m_vCacheFiles.clear();
    }
    
    if (enumVDT_Volume == m_nType)
    {
        {
            KAutoLock lock(m_lock);
            m_vTopExts.clear();
            m_vTopFiles.clear();
            m_vCacheFiles.clear();
        }

        piFCache = GetFCache();
        if (!piFCache)
            goto clean0;

        bRetCode = piFCache->Initialize();
        ASSERT(bRetCode);

        bRetCode = piFCache->Clean();
        ASSERT(bRetCode);

        bRetCode = piFCache->BeginAdd();
        ASSERT(bRetCode);
    }

clean0:
    return;
}

void CVirtualDirManager::EndEnum(BOOL bCanceled)
{
    IFCache* piFCache = NULL;
    BOOL bRetCode;
    ULONG dwRetCode;
    std::vector<BigFileInfo> vCacheFiles;
    SYSTEMTIME sysTime;
    ULONGLONG qwTop100Size;

    {
        KAutoLock lock(m_lock);
        m_nEnumState = bCanceled ? enumES_Canceled : enumES_Finished;
    }

    if (enumVDT_Dir == m_nType)
    {
        FillListCtrl();
//         GetCacheList(vCacheFiles, 100, nMore);
//         //std::sort(vCacheFiles.rbegin(), vCacheFiles.rend());
//         CleanBigFileList();
//         size_t i;
//         for (i = 0; i < vCacheFiles.size(); ++i)
//         {
//             AddBigFileToListCtrl((int)i, vCacheFiles[i]);
//         }
    }

    if (enumVDT_Volume == m_nType)
    {
        piFCache = GetFCache();
        if (!piFCache)
            goto clean0;

        bRetCode = piFCache->EndAdd();
        ASSERT(bRetCode);

        // 设置时间记录
        {
            GetLocalTime(&sysTime);
            bRetCode = piFCache->SetCacheInfo(sysTime, !bCanceled);
        }

        // 获得统计信息
        {
            KAutoLock lock(m_lock);
            bRetCode = piFCache->GetTotalInfo(m_qwTotalSize, m_qwTotalCount);
        }

        {
            KAutoLock lock(m_lock);
            bRetCode = piFCache->GetCacheInfo(m_CacheTime, m_bFullCache);
        }

        // 获取Top100文件
        dwRetCode = 100;
        {
            KAutoLock lock(m_lock);
            m_vTopFiles.clear();
        }
        bRetCode = piFCache->Query(this, enumFQT_Top, &dwRetCode, NULL);
        //ASSERT(bRetCode);

        // 获取Top扩展名
        GetTop100Size(qwTop100Size);
        {
            KAutoLock lock(m_lock);
            m_vTopExts.clear();
            BigExtInfo ext;
            ext.strExt = _T("最大的100个文件");
            ext.qwTotalSize = qwTop100Size;
            ext.nPercent = m_qwTotalSize ? int(qwTop100Size * 100 / m_qwTotalSize) : 0;
            m_vTopExts.push_back(ext);
        }
        bRetCode = piFCache->QueryTopExt(this, 10);
        //ASSERT(bRetCode);

        bRetCode = piFCache->UnInitialize();
        ASSERT(bRetCode);

        // 从top表拷贝到cache中
        CopyTopListToCacheList();
//        GetCacheList(vCacheFiles, 100, nMore);
        //std::sort(vCacheFiles.rbegin(), vCacheFiles.rend());
//        CleanBigFileList();
//         size_t i;
//         for (i = 0; i < vCacheFiles.size(); ++i)
//         {
//             AddBigFileToListCtrl((int)i, vCacheFiles[i]);
//         }
        FillListCtrl();
    }

    PostMessage(
        m_hNotifyWnd, 
        WM_BIGFILE_SCAN_END, 
        (WPARAM)(LONG_PTR)(this),
        0
        );

clean0:
    return;
}

EnumState CVirtualDirManager::GetEnumState()
{
    KAutoLock lock(m_lock);
    return m_nEnumState;
}

void CVirtualDirManager::GetScanningFile(CString& strFilePath, int& nPos)
{
    KAutoLock lock(m_lockStatus);
    ULONGLONG qwVolSize;

    strFilePath = m_strCurrentFile;
    qwVolSize = m_qwVolTotalSize - m_qwVolFreeSize;

    if (enumVDT_Dir == m_nType)
    {
        nPos = -1;
    }
    else
    {
        if (m_qwEnumedSize > qwVolSize)
        {
            nPos = 99;
        }
        else
        {
            nPos = qwVolSize ? int(m_qwEnumedSize * 100 / qwVolSize) : 0;
        }
    }
}

BOOL CVirtualDirManager::FillListCtrl()
{
    BOOL retval = FALSE;
    IFCache* piFCache = NULL;
    CStringA strExt;
    BOOL bRetCode;
    BOOL bTop100;
    BOOL bIsVolume = TRUE;
    DWORD dwTop100 = 100;

    {
        KAutoLock lock(m_lock);
        if (enumVDT_Dir == m_nType)
            bIsVolume = FALSE;
    }

    m_ctrlFileList.DeleteAllItems();

    {
        KAutoLock lock(m_lock);
        if (bIsVolume)
        {
            bTop100 = m_strExtFilter.IsEmpty();
            m_vCacheFiles.clear();
        }
    }

    if (bIsVolume)
    {
        if (!bTop100)
        {
            // 按扩展名
            piFCache = GetFCache();
            if (!piFCache)
                goto clean0;

            strExt = KUTF16_To_ANSI(m_strExtFilter);
            bRetCode = piFCache->Initialize();
            ASSERT(bRetCode);

            bRetCode = piFCache->Query(this, enumFQT_Ext, (void*)(LPCSTR)strExt, NULL);
            ASSERT(bRetCode);

            bRetCode = piFCache->UnInitialize();
            ASSERT(bRetCode);
        }
        else
        {
            // Top100
            piFCache = GetFCache();
            if (!piFCache)
                goto clean0;

            {
                KAutoLock lock(m_lock);
                m_vTopFiles.clear();
            }

            strExt = KUTF16_To_ANSI(m_strExtFilter);
            bRetCode = piFCache->Initialize();
            ASSERT(bRetCode);

            bRetCode = piFCache->Query(this, enumFQT_Top, &dwTop100, NULL);
            //ASSERT(bRetCode);

            bRetCode = piFCache->UnInitialize();
            ASSERT(bRetCode);

            CopyTopListToCacheList();
        }
    }

    {
        // 填充
        CString strFilePath, strName, strDir, strFileSize;
        int nItem;
        size_t count = 0;
        size_t i = 0;
        std::vector<BigFileInfo> vfiles;
        size_t nMore;

        GetCacheList(vfiles, (size_t)m_dwFileListLimit, nMore);

        for (i = 0; i < vfiles.size(); ++i)
        {
            strFilePath = vfiles[i].strFilePath;
            //strName = strFilePath.Right(strFilePath.GetLength() - strFilePath.ReverseFind(_T('\\')) - 1);
            //strDir = strFilePath.Left(strFilePath.GetLength());
            //strFileSize.Format(_T("%I64d"), vfiles[i].qwFileSize);

            nItem = m_ctrlFileList.InsertItem((UINT)i, strFilePath);
            m_ctrlFileList.SetItemData(nItem, i);
        }

        m_ctrlFileList.SetMoreItem((DWORD)nMore);
        
        if (nMore != 0)
        {
            nItem = m_ctrlFileList.InsertItem((UINT)i, _T("More"));
            m_ctrlFileList.SetItemData(nItem, -1);
        }
    }

    if (m_ctrlFileList.GetItemCount() == 0)
    {
        m_ctrlFileList.InsertItem(0, _T("None"));
        m_ctrlFileList.SetItemData(0, -2);
    }

clean0:
    return retval;
}

void CVirtualDirManager::CleanBigFileList()
{
    m_ctrlFileList.DeleteAllItems();
}

//////////////////////////////////////////////////////////////////////////

BOOL CVirtualDirManager::LoadCacheASync()
{
    BOOL retval = FALSE;
    HANDLE hThread = NULL;

    hThread = (HANDLE)_beginthreadex(NULL, 0, LoadCacheThread, this, 0, NULL);
    if (!hThread)
        goto clean0;

    retval = TRUE;

clean0:
    if (hThread)
    {
        CloseHandle(hThread);
    }

    return retval;
}

UINT CVirtualDirManager::LoadCacheThread(LPVOID pParam)
{
    CVirtualDirManager* pThis = (CVirtualDirManager*)pParam;

    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);

    if (pThis)
    {
        pThis->LoadCacheSync();
    }

    _endthreadex(0);
    return 0;
}

void CVirtualDirManager::GetCacheSummary(CString& strSummary)
{
    strSummary.Format(
        _T("上次扫描时间：%d月%d日%d点%d分%s"),
        m_CacheTime.wMonth,
        m_CacheTime.wDay,
        m_CacheTime.wHour,
        m_CacheTime.wMinute,
        m_bFullCache ? _T("") : _T("，建议重新扫描")
        );
}

BOOL CVirtualDirManager::LoadCacheSync()
{
    BOOL retval = FALSE;
    BOOL bRetCode;
    IFCache* piFCache = NULL;
    DWORD dwRetCode = 100;
    ULONGLONG qwTop100Size;

    if (enumVDT_Dir == m_nType)
    {
        retval = TRUE;
        goto clean0;
    }

    if (TRUE == m_bRemovable)
    {
        retval = TRUE;
        goto clean0;
    }

    piFCache = GetFCache();
    if (!piFCache)
        goto clean0;

    {
        KAutoLock lock(m_lock);
        m_vCacheFiles.clear();
        m_vTopExts.clear();
    }

    bRetCode = piFCache->Initialize();
    ASSERT(bRetCode);

    bRetCode = piFCache->Query(this, enumFQT_Top, &dwRetCode, NULL);
    if (!bRetCode)
        goto clean0;

    bRetCode = piFCache->GetTotalInfo(m_qwTotalSize, m_qwTotalCount);
    if (!bRetCode)
        goto clean0;

    {
        KAutoLock lock(m_lock);
        bRetCode = piFCache->GetCacheInfo(m_CacheTime, m_bFullCache);
    }

    GetTop100Size(qwTop100Size);
    if (!qwTop100Size)
        goto clean0;

    {
        KAutoLock lock(m_lock);
        BigExtInfo ext;
        ext.strExt = _T("最大的100个文件");
        ext.qwTotalSize = qwTop100Size;
        ext.nPercent = m_qwTotalSize ? int(qwTop100Size * 100 / m_qwTotalSize) : 0;
        m_vTopExts.push_back(ext);
    }
    bRetCode = piFCache->QueryTopExt(this, 10);
    ASSERT(bRetCode);

    bRetCode = piFCache->UnInitialize();
    ASSERT(bRetCode);

    CopyTopListToCacheList();
    FillListCtrl();
    m_nEnumState = enumES_Cache;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CVirtualDirManager::GetBigFileInfo(size_t idx, BigFileInfo& fileInfo)
{
    BOOL retval = FALSE;
    KAutoLock lock(m_lock);

    if (idx >= m_vCacheFiles.size())
        goto clean0;

    fileInfo = m_vCacheFiles[idx];
    retval = TRUE;

clean0:
    return retval;
}

BOOL CVirtualDirManager::DeleteBigFile(const CString& strFilePath)
{
    BOOL retval = FALSE;
    IFCache* piFCache = NULL;
    BOOL bRetCode;

    {
        KAutoLock lock(m_lock);
        if (enumVDT_Dir == m_nType)
        {
            m_vDeleteCache.push_back(strFilePath);
        }
    }

    piFCache = GetFCache();
    if (!piFCache)
        goto clean0;

    bRetCode = piFCache->Initialize();
    if (!bRetCode)
        goto clean0;

    bRetCode = piFCache->DelFile(strFilePath);
    ASSERT(bRetCode);

    bRetCode = piFCache->UnInitialize();
    ASSERT(bRetCode);

    retval = TRUE;

clean0:
    return retval;
}

/*
BOOL CVirtualDirManager::DeleteCacheItem(size_t idx)
{
    BOOL retval = FALSE;
    KAutoLock lock(m_lock);

    if (idx >= m_vCacheFiles.size())
        goto clean0;

    m_vCacheFiles.erase(m_vCacheFiles.begin() + idx);
    m_ctrlFileList.DeleteItem((int)idx);

    retval = TRUE;

clean0:
    return retval;
}

BOOL CVirtualDirManager::RestoreCacheItem(const BigFileInfo& fileInfo)
{
    BOOL retval = FALSE;
    KAutoLock lock(m_lock);

    m_vCacheFiles.push_back(fileInfo);
    std::sort(m_vCacheFiles.rbegin(), m_vCacheFiles.rend());
    m_ctrlFileList.DeleteAllItems();
    // todo:

    retval = TRUE;

clean0:
    return retval;
}
*/
BOOL CVirtualDirManager::StartScan()
{
    BOOL retval = FALSE;
    HANDLE hScanThread = NULL;
    KAutoLock lock(m_lock);

    if (enumES_Scanning == m_nEnumState)
    {
        retval = TRUE;
        goto clean0;
    }

    m_nEnumState = enumES_Scanning;

    m_bStopScanFlag = FALSE;

    hScanThread = (HANDLE)::_beginthreadex(
        NULL, 0, ScanThread, this, 0, NULL);
    if (!hScanThread)
        goto clean0;

    retval = TRUE;

clean0:
    if (hScanThread)
    {
        CloseHandle(hScanThread);
        hScanThread = NULL;
    }
    return retval;
}

BOOL CVirtualDirManager::StopScan()
{
    BOOL retval = FALSE;
    KAutoLock lock(m_lock);

    if (enumES_Scanning == m_nEnumState)
    {
        m_bStopScanFlag = TRUE;
    }

    retval = TRUE;

//clean0:
    return retval;
}

BOOL CVirtualDirManager::StartFillList()
{
    BOOL retval = FALSE;
    KAutoLock lock(m_lock);
    
    if (m_bFilling)
    {
        retval = TRUE;
        goto clean0;
    }

    m_bStopFillFlag = FALSE;

    m_hFillThread = (HANDLE)::_beginthreadex(
        NULL, 0, FillThread, this, 0, NULL);
    if (!m_hFillThread)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CVirtualDirManager::StopFillList()
{
    BOOL retval = FALSE;
    KAutoLock lock(m_lock);

    if (m_bFilling)
    {
        m_bStopFillFlag = TRUE;
    }

    retval = TRUE;

//clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

UINT CVirtualDirManager::ScanThread(LPVOID pParam)
{
    CVirtualDirManager* pThis = (CVirtualDirManager*)pParam;
    std::vector<CString> vEnumPaths;
    size_t i;
    BOOL bRetCode;
    void* pFsRedirect = NULL;

    if (!pThis)
        goto clean0;

    pThis->GetEnumPaths(vEnumPaths);

    if (pThis->IsSysVolume())
    {
        if (IsX64System())
            DisableWow64FsRedirection(&pFsRedirect);
    }

    pThis->BeginEnum();

    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        bRetCode = bigfilehelper::EnumFiles(
            vEnumPaths[i],
            pThis
            );
        if (!bRetCode)
        {
            pThis->EndEnum(TRUE);
            goto clean0;
        }
    }

    pThis->EndEnum(FALSE);

clean0:
    if (pThis->IsSysVolume() && pFsRedirect)
    {
        if (IsX64System())
            RevertWow64FsRedirection(pFsRedirect);
    }

    _endthreadex(0);
    return 0;
}

UINT CVirtualDirManager::FillThread(LPVOID pParam)
{
    CVirtualDirManager* pThis = (CVirtualDirManager*)pParam;

    if (!pThis)
        goto clean0;

clean0:
    _endthreadex(0);
    return 0;
}

//////////////////////////////////////////////////////////////////////////

void CVirtualDirManager::GetEnumPaths(std::vector<CString>& vEnumPaths)
{
    vEnumPaths.clear();

    if (enumVDT_Dir == m_nType)
    {
        if (enumRDT_Desktop == m_nDirType)
        {
            CString strPath;
            bigfilehelper::GetDesktopPath(strPath);
            vEnumPaths.push_back(strPath);
        }
        else if (enumRDT_Document == m_nDirType)
        {
            CString strPath;
            bigfilehelper::GetDocumentPath(strPath);
            vEnumPaths.push_back(strPath);
        }
        else if (enumRDT_Download == m_nDirType)
        {
            bigfilehelper::GetAllDownloadDirs(vEnumPaths);
        }
    }

    if (enumVDT_Volume == m_nType)
    {
        CString strPath;
        strPath.Format(_T("%c:\\"), m_cVolume);
        vEnumPaths.push_back(strPath);
    }
}

//////////////////////////////////////////////////////////////////////////

void CVirtualDirManager::OnFileListSize(CRect rcZone)
{
    if (m_ctrlFileList.m_hWnd)
    {
        m_ctrlFileList.MoveWindow(rcZone);
    }
}

//////////////////////////////////////////////////////////////////////////

BOOL CVirtualDirManager::ShowFileProperties(const CString& strFilePath)
{
    SHELLEXECUTEINFO shellInfo = { 0 };

    shellInfo.cbSize = sizeof(shellInfo);
    shellInfo.lpVerb = _T("properties");
    shellInfo.lpFile = (LPCTSTR)strFilePath;
    shellInfo.fMask = SEE_MASK_INVOKEIDLIST;
    
    return ShellExecuteEx(&shellInfo);
}

//////////////////////////////////////////////////////////////////////////

CUIHandlerBigFile::CUIHandlerBigFile(CKscMainDlg* refDialog)
{
    m_dlg = refDialog;
    m_pCurrentDir = NULL;
    m_hFCacheModule = NULL;
}

CUIHandlerBigFile::~CUIHandlerBigFile()
{

}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerBigFile::Init()
{
    //LoadFCache();           // 加载FCache

    CreateFilterBox();
    CreateFileListCtrl();   // 创建右侧List控件
    CreateDirectoryCtrls(); // 创造左侧Dir控件

	/*
	StartBackgroundScan();

	m_dlg->SetTimer(enumBFTI_Animation, 50);
	m_dlg->SetTimer(enumBFTI_ScanPath, 100);
	*/
}

void CUIHandlerBigFile::InitDelay()
{
	LoadFCache();           // 加载FCache
	StartBackgroundScan();

	m_dlg->SetTimer(enumBFTI_Animation, 50);
	m_dlg->SetTimer(enumBFTI_ScanPath, 100);
}

void CUIHandlerBigFile::UnInit()
{
    // todo: Destory windows
}

void CUIHandlerBigFile::Show()
{
    if (!m_pCurrentDir)
    {
        OnSwitchVDir(1, 0);
    }
}

//////////////////////////////////////////////////////////////////////////

BOOL CUIHandlerBigFile::OnStartScan()
{
    BOOL retval = FALSE;
    CVirtualDirManager* pDir = m_pCurrentDir;

    if (!pDir)
        goto clean0;

    retval = pDir->StartScan();
    ASSERT(retval);

    ShowScanProgress();
    m_dlg->SetTimer(enumBFTI_Animation, 50);
    m_dlg->SetTimer(enumBFTI_ScanPath, 100);

clean0:
    return retval;
}

BOOL CUIHandlerBigFile::OnStopScan()
{
    BOOL retval = FALSE;
    CVirtualDirManager* pDir = m_pCurrentDir;

    if (!pDir)
        goto clean0;

    retval = pDir->StopScan();

clean0:
    return retval;
}

BOOL CUIHandlerBigFile::OnReScan()
{
    BOOL retval = FALSE;
    CVirtualDirManager* pDir = m_pCurrentDir;

    if (!pDir)
        goto clean0;

    pDir->UpdateVolumeSizeInfo();
    pDir->SetListLimit(50);
    retval = pDir->StartScan();
    ASSERT(retval);

    UpdateHeaderTip();
    ShowReScanButton(FALSE);
    ShowFileList(FALSE);
    ShowScanProgress();
    ShowFilterUI(FALSE);

clean0:
    return retval;
}

BOOL CUIHandlerBigFile::LoadFCache()
{
    BOOL retval = FALSE;

    m_hFCacheModule = ::LoadLibrary(_T("fcache.dll"));
    if (!m_hFCacheModule)
        goto clean0;

    g_pfnGetFCache = (GetFCache_t)::GetProcAddress(m_hFCacheModule, "GetFCache");
    if (!g_pfnGetFCache)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerBigFile::OnInitMenuPopup(CMenu menuPopup, UINT nIndex, BOOL bSysMenu)
{
    if (bSysMenu)   // 不处理SysMenu
    {
        SetMsgHandled(FALSE);
        return;
    }

    TCHAR szString[MAX_MENU_ITEM_TEXT_LENGTH];
    BOOL bRet = FALSE;

    for (int i = 0; i < menuPopup.GetMenuItemCount(); i++)
    {
        CMenuItemInfo mii;
        mii.cch = MAX_MENU_ITEM_TEXT_LENGTH;
        mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
        mii.dwTypeData = szString;
        bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
        if (mii.wID >= 200 && mii.wID < 200 + 20)
        {
            // 只关注Filter菜单, 设为自绘
            mii.fType |= MFT_OWNERDRAW;
            mii.dwItemData = (ULONG_PTR)NULL;
            bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
        }
    }
}

void CUIHandlerBigFile::OnMenuSelect(UINT nItemID, UINT nFlags, CMenu menu)
{
    // nothing
}

void CUIHandlerBigFile::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CRect rcItem = lpDrawItemStruct->rcItem;
    CDCHandle dc = lpDrawItemStruct->hDC;
    UINT uId = lpDrawItemStruct->itemID - 200;
    CRect rcExt = rcItem;
    CRect rcProgress = rcItem;
    CRect rcPer = rcItem;
    CRect rcSize = rcItem;
    CPen penBorder, penSelected;
    CBrush brush1, brush2;
    CString strExt;
    CString strSize;
    CString strPer;
    int nPercent;

    std::vector<BigExtInfo> vTopExts;

    if (!m_pCurrentDir)
        goto clean0;

    m_pCurrentDir->GetTopExts(vTopExts);

    strExt = vTopExts[uId].strExt;
    bigfilehelper::GetFileSizeString(vTopExts[uId].qwTotalSize, strSize);
    nPercent = vTopExts[uId].nPercent;
    strPer.Format(_T("%2d%s"), nPercent, _T("%"));
    
    HICON hExtIcon = GetIconFromFileType(strExt);
    HICON hUnknownIcon = KAppRes::Instance().GetIcon("IDI_UNKNOWN_SMALL");
    BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;

    penBorder.CreatePen(PS_SOLID, 1, RGB(0x7d, 0xa2, 0xce));
    brush1.CreateSolidBrush(RGB(0xeb, 0xf4, 0xfd));
    brush2.CreateSolidBrush(RGB(239,246,254));

    dc.SelectPen(penBorder);
    dc.SelectBrush(brush2);
    //dc.FillSolidRect(rcItem, RGB(239,246,254)); // 背景
    dc.FillSolidRect(rcItem, RGB(0xff,0xff,0xff)); // 背景
    dc.SetBkMode(TRANSPARENT);
    if (bSelected)
    {
        CRect rcSelect = rcItem;
        rcSelect.left += 1;
        rcSelect.top += 1;
        rcSelect.right -= 1;
        rcSelect.bottom -= 1;
        dc.RoundRect(rcSelect, CPoint(3, 3));
        rcSelect.left += 2;
        rcSelect.right -= 2;
        rcSelect.top += 2;
        rcSelect.bottom -= 2;
        DrawGradualColorRect(dc, rcSelect, RGB(0xdd, 0xec, 0xfd), RGB(0xc2, 0xdc, 0xfd));
    }

    if (0 == uId)
        rcExt.left = 4;
    else
        rcExt.left = 23;
    rcExt.right = 106;
    dc.DrawText(strExt, strExt.GetLength(), rcExt, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);

    if (uId != 0)
    {
        if (hExtIcon)
        {
            dc.DrawIconEx(rcItem.left + 4, rcItem.top + 3, hExtIcon, 16, 16);
        }
        else
        {
            dc.DrawIconEx(rcItem.left + 4, rcItem.top + 3, hUnknownIcon, 16, 16);
        }
    }

    dc.SelectPen(penBorder);
    dc.SelectBrush(brush1);
    dc.RoundRect(rcItem.left + 108, rcItem.top + 6, rcItem.left + 170, rcItem.bottom - 6, 3, 3);
    dc.FillSolidRect(rcItem.left + 110, rcItem.top + 8, 58, rcItem.Height() - 16, RGB(224,222,223));
    dc.FillSolidRect(rcItem.left + 110, rcItem.top + 8, 58 * nPercent / 100, rcItem.Height() - 16, RGB(148,210,101));

    rcPer.left = 177;
    rcPer.right = 212;
    dc.DrawText(strPer, strPer.GetLength(), rcPer, DT_RIGHT|DT_SINGLELINE|DT_VCENTER);

    rcSize.left = 214;
    rcSize.right = 272;
    dc.DrawText(strSize, strSize.GetLength(), rcSize, DT_RIGHT|DT_SINGLELINE|DT_VCENTER);

    if (hExtIcon)
    {
        DestroyIcon(hExtIcon);
        hExtIcon = NULL;
    }

clean0:
    return;
}

void CUIHandlerBigFile::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    lpMeasureItemStruct->itemHeight = 22;
    lpMeasureItemStruct->itemWidth = 280;
}

//////////////////////////////////////////////////////////////////////////

HICON CUIHandlerBigFile::GetIconFromFileType(LPCTSTR lpExtName)
{
    HICON retval = NULL;
    CString strExtName;
    CString strIconPath;
    TCHAR szBuffer[MAX_PATH] = { 0 };
    DWORD dwLength;
    CRegKey regKey;
    LONG lRetCode;
    CString strRegPath;
    int nFlag;
    int nIconIndex = 0;

    if (!lpExtName)
        goto clean0;

    if (lpExtName[0] == _T('.'))
    {
        strExtName = lpExtName;
    }
    else
    {
        strExtName = _T(".");
        strExtName += lpExtName;
    }

    if (strExtName.CompareNoCase(_T(".exe")) == 0)
        strExtName = _T(".com");

    if (strExtName.CompareNoCase(_T(".sys")) == 0)
        strExtName = _T(".dll");

    lRetCode = regKey.Open(HKEY_CLASSES_ROOT, strExtName, KEY_READ);
    if (lRetCode)
        goto clean0;

    dwLength = MAX_PATH;
    lRetCode = regKey.QueryStringValue(NULL, szBuffer, &dwLength);
    if (lRetCode)
        goto clean0;

    regKey.Close();

    strRegPath = szBuffer;
    strRegPath += _T("\\DefaultIcon");
    lRetCode = regKey.Open(HKEY_CLASSES_ROOT, strRegPath, KEY_READ);
    if (lRetCode)
        goto clean0;

    dwLength = MAX_PATH;
    lRetCode = regKey.QueryStringValue(NULL, szBuffer, &dwLength);
    if (lRetCode)
        goto clean0;

    strIconPath = szBuffer;
    nFlag = strIconPath.ReverseFind(_T(','));
    if (nFlag != -1)
    {
        CString strIndex = strIconPath.Right(strIconPath.GetLength() - nFlag - 1);
        nIconIndex = StrToInt(strIndex);
        strIconPath = strIconPath.Left(nFlag);
    }

    if (strIconPath.Find(_T('\\')) == -1)
    {
        TCHAR szSystem[MAX_PATH] = { 0 };
        GetSystemDirectory(szSystem, MAX_PATH);
        PathAppend(szSystem, strIconPath);
        strIconPath = szSystem;
    }

    if (!ExtractIconExW(strIconPath, nIconIndex, NULL, &retval, 1))
        goto clean0;

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerBigFile::OnFileListSize(CRect rcZone)
{
    m_listWrapper.MoveWindow(rcZone);
}

//////////////////////////////////////////////////////////////////////////

LRESULT CUIHandlerBigFile::OnFileListViewClickUrl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // todo:
    return 0;
}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerBigFile::OnSizeLimitCheck()
{
    BOOL bEnableSizeLimit;
    int idx = -1;

    bEnableSizeLimit = m_dlg->GetItemCheck(ID_CHK_BIGFILE_SIZE_LIMIT);
    if (bEnableSizeLimit)
    {
        m_boxSizeLimit.EnableWindow(TRUE);
        idx = m_boxSizeLimit.GetCurSel();
    }
    else
    {
        m_boxSizeLimit.EnableWindow(FALSE);
    }

    UpdateFileSizeWithSize(idx);
}

void CUIHandlerBigFile::OnSysFilterLnk()
{
    CVirtualDirManager* pDir = m_pCurrentDir;
    if(NULL == pDir)
        return ;

    BOOL bFilter = pDir->GetSysFilterEnable();   
  
    pDir->SetSysFilterEnable(!bFilter);
    pDir->m_ctrlFileList.SetSysFilterEnable(!bFilter);
    pDir->FillListCtrl();

}

void CUIHandlerBigFile::OnComboboxReSize(CRect rcZone)
{
    m_boxSizeLimit.MoveWindow(rcZone);
}

void CUIHandlerBigFile::OnDirectoryCtlReSize(CRect rcZone)
{
    m_ctrlVirtualDirs.MoveWindow(rcZone);
}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerBigFile::OnTimer(UINT_PTR nIDEvent)
{
    if (enumBFTI_Animation == nIDEvent)
    {
        static int nFrame = 0;
        m_dlg->SetItemIntAttribute(ID_IMG_BIGFILE_ENUM_ANIM, "sub", nFrame++);
        if (nFrame > 5)
            nFrame = 0;
    }

    if (enumBFTI_ScanPath == nIDEvent)
    {
        CVirtualDirManager* pDir = m_pCurrentDir;
        int nPos;
        if (pDir)
        {
            CString strScanFile;
            pDir->GetScanningFile(strScanFile, nPos);
            bigfilehelper::CompressedPath(strScanFile);
            m_dlg->SetItemText(ID_TXT_BIGFILE_ENUM_PATH, strScanFile);
            if (nPos != -1)
            {
                m_dlg->SetItemIntAttribute(ID_POS_BIGFILE_ENUM, "value", nPos);
                m_dlg->SetItemVisible(ID_POS_BIGFILE_ENUM, TRUE);
            }
            else
            {
                m_dlg->SetItemVisible(ID_POS_BIGFILE_ENUM, FALSE);
            }
        }
    }
	SetMsgHandled(FALSE);
}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerBigFile::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    BOOL bRetCode;

    if (wndCtl.m_hWnd == m_ctrlVirtualDirs.m_hWnd)
    {
        size_t nSelect = (size_t)uNotifyCode >> 8;
        size_t nSubSelect = (size_t)uNotifyCode & 0xFF;
        OnSwitchVDir(nSelect, nSubSelect);
    }

    if (CBN_SELCHANGE == uNotifyCode)
    {
        if (ID_CTL_BIGFILE_SIZE_COMBOBOX == nID)
        {
            OnSizeLimitCheck();
            SetMsgHandled(TRUE);
        }
    }

    if (nID >= 100 && nID < 106)
    {
        // 查看属性
        CVirtualDirManager* pDir = m_pCurrentDir;
        if (!pDir)
            return;

        BigFileInfo fileInfo;
        int nItem = pDir->GetCtrl().GetSelectedIndex();
        size_t idx = (size_t)pDir->GetCtrl().GetItemData(nItem);
        if (idx == size_t(-1) || idx == size_t(-2))
            return;

        pDir->GetBigFileInfo(idx, fileInfo);

        if (100 == nID)
        {
            // 定位文件
            bigfilehelper::LocateFile(fileInfo.strFilePath);
        }
        else if (101 == nID)
        {
            // 
        }
        else if (102 == nID)
        {
            // 删除到回收站
            pDir->GetCtrl().EnableWindow(FALSE);
            bRetCode = DelBigFile(fileInfo.strFilePath);
            pDir->GetCtrl().EnableWindow(TRUE);
            if (bRetCode)
            {
                pDir->GetCtrl().DeleteItem(nItem);
            }
        }
        else if (103 == nID)
        {
            // 查看属性
            bigfilehelper::ShowFileProperties(fileInfo.strFilePath);
        }
        else if(104 == nID || 105 == nID)
        {
            bigfilehelper::BaiduFileinfo(fileInfo.strFilePath);
        }
       
    }

    if (nID >= 200 && nID < 211)
    {
        UpdateFileListWithExt(nID - 200);
    }
    SetMsgHandled(FALSE);
}

void CUIHandlerBigFile::UpdateFileListWithExt(int idx)
{
    std::vector<BigExtInfo> exts;
    CVirtualDirManager* pDir = m_pCurrentDir;

    if (idx < 0)
        goto clean0;

    if (!pDir)
        goto clean0;

    pDir->SetListLimit(50);
    
    if (0 == idx)
    {
        m_dlg->SetItemText(ID_TXT_BIGFILE_FILTER, _T("最大的100个文件"));
        pDir->SetExtFilter(_T(""));
    }
    else
    {
        pDir->GetTopExts(exts);
        if ((size_t)idx < exts.size())
        {
            CString strTitle;
            CString strExt = exts[idx].strExt;
            bigfilehelper::GetShortExtString(strExt);
            strTitle.Format(_T("按%s类型过滤"), strExt);
            m_dlg->SetItemText(ID_TXT_BIGFILE_FILTER, strTitle);
            pDir->SetExtFilter(exts[idx].strExt);
        }
    }

    pDir->FillListCtrl();

clean0:
    return;
}

void CUIHandlerBigFile::UpdateFileSizeWithSize(int idx)
{
    CVirtualDirManager* pDir = m_pCurrentDir;

    if (!pDir)
        goto clean0;

    if (-1 == idx)
    {
        pDir->SetSizeFilterEnable(FALSE); //不过滤
    }
    else
    {
        pDir->SetSizeFilterEnable(TRUE);
        pDir->SetSizeFilter(m_boxSizeLimit.GetCurSel());
    }

    pDir->FillListCtrl();

clean0:
    return;
}

LRESULT CUIHandlerBigFile::OnScanEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CVirtualDirManager* pDir = (CVirtualDirManager*)wParam;
    VirtualDirType nType;

    if (pDir)
    {
        if (pDir == m_pCurrentDir)
        {
            UpdateHeaderTip();
            HideScanUI();
            ShowFileList(TRUE);
            ShowReScanButton(TRUE);

            nType = pDir->GetDirType();
            if (nType != enumVDT_Dir)
                ShowFilterUI(TRUE);
        }
    }

    return 0L;
}

LRESULT CUIHandlerBigFile::OnListMore(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CVirtualDirManager* pDir = m_pCurrentDir;

    if (pDir)
    {
        pDir->SetListLimit(0);
        pDir->FillListCtrl();
    }

    return 0L;
}

LRESULT CUIHandlerBigFile::OnListLink(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CVirtualDirManager* pDir = m_pCurrentDir;
    int nItem;
    int nLink;
    size_t idx;
    CString strFilePath;
    BigFileInfo fileInfo;
    VirtualDirType nType;
    BOOL bRetCode;

    if (pDir)
    {
        nItem = (int)wParam;
        nLink = (int)lParam;

        idx = pDir->GetCtrl().GetItemData(nItem);
        if (idx == size_t(-1) || idx == size_t(-2))
            return 0L;

        pDir->GetBigFileInfo(idx, fileInfo);
        nType = pDir->GetDirType();
        strFilePath = fileInfo.strFilePath;

        if (enumVDT_Dir == nType)
        {
            if (0 == nLink)
            {
                // 删除到回收站
                pDir->GetCtrl().EnableWindow(FALSE);
                bRetCode = DelBigFile(strFilePath);
                pDir->GetCtrl().EnableWindow(TRUE);
                if (bRetCode)
                {
                    pDir->GetCtrl().DeleteItem(nItem);
                }
            }
        }
        else if (enumVDT_Volume == nType)
        {
            if (0 == nLink)
            {
                // 定位
                bigfilehelper::LocateFile(strFilePath);
            }
        }


    }

    return 0L;
}
/*
static UINT WINAPI MessageChainThread(LPVOID pParam)
{
    MSG* pMsg = (MSG*)pParam;

    if (!pMsg)
        goto clean0;

    ::SendMessage(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);

clean0:
    if (pMsg)
    {
        delete pMsg;
        pMsg = NULL;
    }
    _endthreadex(0);
    return 0;
}

LRESULT CUIHandlerBigFile::OnListLink(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HANDLE hThread = NULL;
    MSG* pMsg = new MSG;

    if (!pMsg)
        goto clean0;

    pMsg->hwnd = m_dlg->m_hWnd;
    pMsg->message = WM_BIGFILE_LIST_LINK2;
    pMsg->wParam = wParam;
    pMsg->lParam = lParam;

    hThread = (HANDLE)_beginthreadex(NULL, 0, MessageChainThread, pMsg, 0, NULL);

clean0:
    if (hThread)
    {
        CloseHandle(hThread);
        hThread = NULL;
    }

    return 0L;
}
*/
LRESULT CUIHandlerBigFile::OnListDBClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int nItem = (int)wParam;
    CVirtualDirManager* pDir = m_pCurrentDir;
    BigFileInfo fileInfo;
    size_t idx;

    if (pDir)
    {
        idx = pDir->GetCtrl().GetItemData(nItem);
        if (idx == size_t(-1) || idx == size_t(-2))
            return 0L;

        // 定位文件
        pDir->GetBigFileInfo(idx, fileInfo);
        if (fileInfo.strFilePath.GetLength())
            bigfilehelper::LocateFile(fileInfo.strFilePath);
    }

    return 0L;
}

HMENU CUIHandlerBigFile::InitFileContextMenu()
{
    HMENU retval = NULL;
    MENUITEMINFO menuItemInfo = { 0 };
    CString strItem;
    BigFileInfo fileInfo;
    int nIndex = 0;
    BOOL bRetCode;

    CVirtualDirManager* pDir = m_pCurrentDir;

    int nItem = pDir->GetCtrl().GetSelectedIndex();
    size_t idx = (size_t)pDir->GetCtrl().GetItemData(nItem);
    if (idx == size_t(-1) || idx == size_t(-2))
        goto clean0;

    pDir->GetBigFileInfo(idx, fileInfo);
    BOOL bFilter = CBigfileFilter::Instance().IsFileInFilter(fileInfo.strFilePath);
    BOOL bFilterEnable = pDir->GetSysFilterEnable();

    retval = ::CreatePopupMenu();
    if (!retval)
        goto clean0;


    strItem = _T("定位文件");
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.fState = MFS_ENABLED;
    menuItemInfo.wID = 100;
    menuItemInfo.dwTypeData = (LPTSTR)(LPCTSTR)strItem;  
    menuItemInfo.cch = strItem.GetLength(); 
    bRetCode = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
    if (!bRetCode)
        goto clean0;

    // 分隔符
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = MIIM_ID | MIIM_STRING;
    menuItemInfo.fType = MFT_SEPARATOR;
    menuItemInfo.wID = 0;
    menuItemInfo.dwTypeData = NULL;  
    menuItemInfo.cch = 0; 
    bRetCode = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
    if (!bRetCode)
        goto clean0;

    if(!(bFilter && bFilterEnable))
    {
        strItem = _T("删除到回收站");
        menuItemInfo.cbSize = sizeof(menuItemInfo);
        menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
        menuItemInfo.fType = MFT_STRING;
        menuItemInfo.fState = MFS_ENABLED;
        menuItemInfo.wID = 102;
        menuItemInfo.dwTypeData = (LPTSTR)(LPCTSTR)strItem;  
        menuItemInfo.cch = strItem.GetLength(); 
        bRetCode = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
        if (!bRetCode)
            goto clean0;

        // 分隔符
        menuItemInfo.cbSize = sizeof(menuItemInfo);
        menuItemInfo.fMask = MIIM_ID | MIIM_STRING;
        menuItemInfo.fType = MFT_SEPARATOR;
        menuItemInfo.wID = 0;
        menuItemInfo.dwTypeData = NULL;  
        menuItemInfo.cch = 0; 
        bRetCode = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
        if (!bRetCode)
            goto clean0;
    }
    

    strItem = _T("查看属性");
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.fState = MFS_ENABLED;
    menuItemInfo.wID = 103;
    menuItemInfo.dwTypeData = (LPTSTR)(LPCTSTR)strItem;  
    menuItemInfo.cch = strItem.GetLength(); 
    bRetCode = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
    if (!bRetCode)
        goto clean0;

    // 分隔符
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = MIIM_ID | MIIM_STRING;
    menuItemInfo.fType = MFT_SEPARATOR;
    menuItemInfo.wID = 0;
    menuItemInfo.dwTypeData = NULL;  
    menuItemInfo.cch = 0; 
    bRetCode = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
    if (!bRetCode)
        goto clean0;

    strItem = _T("百度一下");
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
    menuItemInfo.fType = MFT_STRING;
    menuItemInfo.fState = MFS_ENABLED;
    menuItemInfo.wID = 104;
    menuItemInfo.dwTypeData = (LPTSTR)(LPCTSTR)strItem;  
    menuItemInfo.cch = strItem.GetLength(); 
    bRetCode = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
    if (!bRetCode)
        goto clean0;

clean0:
    return retval;
}
/*
HMENU CUIHandlerBigFile::InitExtFilterMenu()
{
    HMENU retval = NULL;
    MENUITEMINFO menuItemInfo = { 0 };
    CString strItem;
    int nIndex = 0;
    BOOL bRetCode;
    CVirtualDirManager* pDir = m_pCurrentDir;
    std::vector<BigExtInfo> vTopExts;
    size_t i;

    if (!pDir)
        goto clean0;

    retval = ::CreatePopupMenu();
    if (!retval)
        goto clean0;

    pDir->GetTopExts(vTopExts);
    for (i = 0; i < vTopExts.size(); ++i)
    {
        strItem = vTopExts[i].strExt;
        menuItemInfo.cbSize = sizeof(menuItemInfo);
        menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
        menuItemInfo.fType = MFT_STRING | MFT_OWNERDRAW;
        menuItemInfo.fState = MFS_ENABLED;
        menuItemInfo.wID = 100;
        menuItemInfo.dwTypeData = (LPTSTR)(LPCTSTR)strItem;  
        menuItemInfo.cch = strItem.GetLength(); 
        menuItemInfo.dwItemData = i;
        bRetCode = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
        if (!bRetCode)
            goto clean0;
    }

clean0:
    return retval;
}

void CUIHandlerBigFile::OnExtFilterContext()
{

}
*/
LRESULT CUIHandlerBigFile::OnListContext(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CVirtualDirManager* pDir = m_pCurrentDir;
    CPoint pos;
    HMENU hMenu = NULL;

    if (!pDir)
        goto clean0;

    hMenu = InitFileContextMenu();
    if (!hMenu)
        goto clean0;

    GetCursorPos(&pos);
    ::TrackPopupMenu(
        hMenu,
        TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
        pos.x, 
        pos.y,
        0,
        m_dlg->m_hWnd,
        NULL
        );

clean0:
    return 0;
}

void CUIHandlerBigFile::OnSwitchVDir(size_t nItem, size_t nSubItem)
{
    EnumState nState;
    PVOID pData = NULL;

    pData = m_ctrlVirtualDirs.GetItemData(nItem, nSubItem);
    if (!pData)
        goto clean0;

    if (m_pCurrentDir != (CVirtualDirManager*)pData)
    {
        ShowFileList(FALSE);
        m_pCurrentDir = (CVirtualDirManager*)pData;
    }

    nState = m_pCurrentDir->GetEnumState();
    if (enumES_Notstart == nState)
    {
        ShowFileList(FALSE);
        ShowFilterUI(FALSE);
        ShowReScanButton(FALSE);
        ShowScanButton();
    }
    else if (enumES_Scanning == nState)
    {
        ShowFileList(FALSE);
        ShowFilterUI(FALSE);
        ShowReScanButton(FALSE);
        ShowScanProgress();
    }
    else if (enumES_Canceled == nState)
    {
        ShowFilterUI(nItem != 0 ? TRUE : FALSE);
        ShowReScanButton(TRUE);
        HideScanUI();
        ShowFileList(TRUE);
    }
    else if (enumES_Finished == nState)
    {
        ShowFilterUI(nItem != 0 ? TRUE : FALSE);
        ShowReScanButton(TRUE);
        HideScanUI();
        ShowFileList(TRUE);
    }
    else if (enumES_Cache == nState)
    {
        ShowFilterUI(nItem != 0 ? TRUE : FALSE);
        ShowReScanButton(TRUE);
        HideScanUI();
        ShowFileList(TRUE);
    }

    UpdateHeaderTip();

clean0:
    return;
}

void CUIHandlerBigFile::ShowFileList(BOOL bShow)
{
    CVirtualDirManager* pDir = m_pCurrentDir;

    m_dlg->SetItemVisible(ID_CTL_BIGFILE_LISTRIGHT, bShow);
    if (bShow && pDir)
    {
        m_listWrapper.ShowChild(pDir->GetCtrl());
    }
}

BOOL CUIHandlerBigFile::ShowTopExtMenu()
{
    BOOL retval = FALSE;
    HMENU hMenu = NULL;
    MENUITEMINFO menuItemInfo = { 0 };
    CVirtualDirManager* pDir = m_pCurrentDir;
    std::vector<BigExtInfo> exts;
    size_t i;
    BOOL bRetCode;
    CString strItem;
    CRect rc;
    CPoint pt;

    if (!pDir)
        goto clean0;

    pDir->GetTopExts(exts);
    if (exts.empty())
        goto clean0;

    hMenu = ::CreatePopupMenu();
    if (!hMenu)
        goto clean0;

    m_dlg->GetWindowRect(&rc);
    pt.x = rc.right - 6;
    pt.y = rc.top + 62; 

    for (i = 0; i < exts.size(); ++i)
    {
        strItem = exts[i].strExt;
        menuItemInfo.cbSize = sizeof(menuItemInfo);
        menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
        menuItemInfo.fType = MFT_STRING;
        menuItemInfo.fState = MFS_ENABLED;
        menuItemInfo.wID = 200 + (UINT)i;
        menuItemInfo.dwTypeData = (LPTSTR)(LPCTSTR)strItem;  
        menuItemInfo.cch = strItem.GetLength(); 
        menuItemInfo.dwItemData = (ULONG_PTR)i;
        bRetCode = ::InsertMenuItem(hMenu, (UINT)i, TRUE, &menuItemInfo);
        if (!bRetCode)
            goto clean0;
    }

    bRetCode = ::TrackPopupMenu(hMenu, TPM_RIGHTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON,
        pt.x, pt.y, 0, m_dlg->m_hWnd, NULL);
    if (!bRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (!retval)
    {
        if (hMenu)
        {
            ::DestroyMenu(hMenu);
            hMenu = NULL;
        }
    }
    return retval;
}

void CUIHandlerBigFile::UpdateHeaderTip()
{
    CVirtualDirManager* pDir = m_pCurrentDir;
    CString strTip;

    if (!pDir)
    {
        m_dlg->SetItemText(ID_TXT_BIGFILE_STATE, _T("正在加载..."));
        goto clean0;
    }

    pDir->GetStatus(strTip);
    m_dlg->SetItemText(ID_TXT_BIGFILE_STATE, strTip);

clean0:
    return;
}

void CUIHandlerBigFile::ShowFilterUI(BOOL bShow)
{
    CVirtualDirManager* pDir = m_pCurrentDir;

    m_dlg->SetItemVisible(ID_BTN_BIGFILE_FILTER_MENU1, bShow);
    m_dlg->SetItemVisible(ID_BTN_BIGFILE_FILTER_MENU2, bShow);
    m_dlg->SetItemVisible(ID_TXT_BIGFILE_FILTER, bShow);
    m_dlg->SetItemVisible(ID_CHK_BIGFILE_SIZE_LIMIT, bShow);
    m_dlg->SetItemVisible(ID_CTL_BIGFILE_SIZE_COMBOBOX, bShow);
    m_dlg->SetItemVisible(ID_TXT_BIGFILE_SIZE_LIMIT, bShow);
    m_dlg->SetItemVisible(ID_CHK_SYSFILE_DISPLAY, bShow);

    if (!pDir)
        return;

    if(pDir->GetSysFilterEnable())
    {
        m_dlg->SetItemCheck(ID_CHK_SYSFILE_DISPLAY, TRUE);      
    }
    else
    {
        m_dlg->SetItemCheck(ID_CHK_SYSFILE_DISPLAY, FALSE);     
    }

    int nFilterIdx = pDir->GetSizeFilter();
    if (pDir->GetSizeFilterEnable())
    {
        m_dlg->SetItemCheck(ID_CHK_BIGFILE_SIZE_LIMIT, TRUE);
        m_boxSizeLimit.EnableWindow(TRUE);
    }
    else
    {
        m_dlg->SetItemCheck(ID_CHK_BIGFILE_SIZE_LIMIT, FALSE);
        m_boxSizeLimit.EnableWindow(FALSE);
    }

    m_boxSizeLimit.SetCurSel(nFilterIdx);

    CString strExt;
    pDir->GetExtFilter(strExt);
    if (strExt.IsEmpty())
    {
        m_dlg->SetItemText(ID_TXT_BIGFILE_FILTER, _T("最大的100个文件"));
    }
    else
    {
        CString strTitle;
        bigfilehelper::GetShortExtString(strExt);
        strTitle.Format(_T("按%s类型过滤"), strExt);
        m_dlg->SetItemText(ID_TXT_BIGFILE_FILTER, strTitle);
    }
}

void CUIHandlerBigFile::ShowReScanButton(BOOL bShow)
{
    m_dlg->SetItemVisible(ID_BTN_BIGFILE_RESCAN, bShow);
}

void CUIHandlerBigFile::ShowScanButton()
{
    m_dlg->SetItemVisible(ID_DIV_BIGFILE_ENUM_START, TRUE);
    m_dlg->SetItemVisible(ID_DIV_BIGFILE_ENUM_STOP, FALSE);
    m_dlg->SetItemVisible(ID_DIV_BIGFILE_ENUM_DLG, TRUE);
}

void CUIHandlerBigFile::ShowScanProgress()
{
    m_dlg->SetItemVisible(ID_DIV_BIGFILE_ENUM_START, FALSE);
    m_dlg->SetItemVisible(ID_DIV_BIGFILE_ENUM_STOP, TRUE);
    m_dlg->SetItemVisible(ID_DIV_BIGFILE_ENUM_DLG, TRUE);
}

void CUIHandlerBigFile::HideScanUI()
{
    m_dlg->SetItemVisible(ID_DIV_BIGFILE_ENUM_DLG, FALSE);
}

//////////////////////////////////////////////////////////////////////////

BOOL CUIHandlerBigFile::CreateDirectoryCtrls()
{
    std::vector<VolInfo> vVols;
    HICON hFolder = KAppRes::Instance().GetIcon("IDI_FOLDER_ICON");
    HICON hDisk = KAppRes::Instance().GetIcon("IDI_DISK_ICON");
    WCHAR cSysVol = bigfilehelper::GetSystemDrive();

    m_ctrlVirtualDirs.AddItem(hFolder, _T("系统盘数据"));
    {
        CVirtualDirManager* pDir = new CVirtualDirManager();
        pDir->SetName(_T("我的桌面"));
        pDir->SetDirType(enumVDT_Dir);
        pDir->SetRealDirType(enumRDT_Desktop);
        pDir->SetNotify(m_dlg->m_hWnd);
        HWND hList = pDir->InitCtrl(m_listWrapper);
        m_listWrapper.AddChild(hList);

        m_ctrlVirtualDirs.AddSubItem(0, _T("我的桌面"), _T(""));
        m_ctrlVirtualDirs.SetItemData(0, 0, pDir);
    }
    {
        CVirtualDirManager* pDir = new CVirtualDirManager();
        pDir->SetName(_T("我的文档"));
        pDir->SetDirType(enumVDT_Dir);
        pDir->SetRealDirType(enumRDT_Document);
        pDir->SetNotify(m_dlg->m_hWnd);
        HWND hList = pDir->InitCtrl(m_listWrapper);
        m_listWrapper.AddChild(hList);

        m_ctrlVirtualDirs.AddSubItem(0, _T("我的文档"), _T(""));
        m_ctrlVirtualDirs.SetItemData(0, 1, pDir);
    }
    {
        CVirtualDirManager* pDir = new CVirtualDirManager();
        pDir->SetName(_T("我的下载"));
        pDir->SetDirType(enumVDT_Dir);
        pDir->SetRealDirType(enumRDT_Download);
        pDir->SetNotify(m_dlg->m_hWnd);
        HWND hList = pDir->InitCtrl(m_listWrapper);
        m_listWrapper.AddChild(hList);

        m_ctrlVirtualDirs.AddSubItem(0, _T("我的下载"), _T(""));
        m_ctrlVirtualDirs.SetItemData(0, 2, pDir);
    }
    
    m_ctrlVirtualDirs.AddItem(hDisk, _T("我的硬盘"));
    bigfilehelper::GetAllVols(vVols);
    for (size_t i = 0; i < vVols.size(); ++i)
    {
        CString strName, strComment;
        int nPercent;
        CVirtualDirManager* pDir = new CVirtualDirManager();
    
        strName.Format(_T("(%c:)"), vVols[i].cVol);
        strComment = vVols[i].strVolName;
        if (vVols[i].qwTotalSize)
            nPercent = 100 - UINT(vVols[i].qwFreeSize * 100 / vVols[i].qwTotalSize);
        else
            nPercent = 100;

        pDir->SetName(strName);
        pDir->SetDirType(enumVDT_Volume);
        pDir->SetVolumeChar(vVols[i].cVol);
        pDir->SetRemovable(vVols[i].bRemovable);
        if ((WCHAR)vVols[i].cVol == cSysVol)
        {
            pDir->SetSysVolume();
        }
        pDir->SetNotify(m_dlg->m_hWnd);
        HWND hList = pDir->InitCtrl(m_listWrapper);
        m_listWrapper.AddChild(hList);

        m_ctrlVirtualDirs.AddSubItem(1, strName, strComment, nPercent, TRUE);
        m_ctrlVirtualDirs.SetItemData(1, i, pDir);
    }

    m_ctrlVirtualDirs.Select(1, 0);

    m_ctrlVirtualDirs.Create(m_dlg->GetViewHWND(), NULL, NULL, WS_CHILD, 0, ID_CTL_BIGFILE_LISTLEFT);
    m_ctrlVirtualDirs.ShowWindow(SW_SHOW);

    ShowScanButton();

    return TRUE;
}

BOOL CUIHandlerBigFile::CreateFileListCtrl()
{
    m_listWrapper.Create(m_dlg->GetViewHWND(), NULL, NULL, WS_CHILD|WS_CLIPCHILDREN, 0, ID_CTL_BIGFILE_LISTRIGHT);
    return TRUE;
}

BOOL CUIHandlerBigFile::CreateFilterBox()
{
    // 创建下面的大小过滤
    m_boxSizeLimit.Create(
        m_dlg->GetRichWnd(), 
        NULL, 
        NULL, 
        WS_CHILD|WS_VISIBLE|WS_DISABLED|CBS_DROPDOWNLIST|CBS_HASSTRINGS, 
        0, 
        ID_CTL_BIGFILE_SIZE_COMBOBOX
        );
    m_boxSizeLimit.SetFont(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
    m_boxSizeLimit.InsertString(0, _T("10"));
    m_boxSizeLimit.SetItemData(0, 10);
    m_boxSizeLimit.InsertString(1, _T("20"));
    m_boxSizeLimit.SetItemData(1, 20);
    m_boxSizeLimit.InsertString(2, _T("50"));
    m_boxSizeLimit.SetItemData(2, 50);
    m_boxSizeLimit.InsertString(3, _T("100"));
    m_boxSizeLimit.SetItemData(3, 100);
    m_boxSizeLimit.InsertString(4, _T("500"));
    m_boxSizeLimit.SetItemData(4, 500);
    m_boxSizeLimit.SetCurSel(2);

    return TRUE;
}

BOOL CUIHandlerBigFile::StartBackgroundScan()
{
    BOOL retval = FALSE;
    BOOL fRetCode;
    CVirtualDirManager* pDir = NULL;
    
    pDir = (CVirtualDirManager*)m_ctrlVirtualDirs.GetItemData(0, 0);
    if (!pDir)
        goto clean0;

    fRetCode = pDir->StartScan();
    if (!fRetCode)
        goto clean0;

    pDir = (CVirtualDirManager*)m_ctrlVirtualDirs.GetItemData(0, 1);
    if (!pDir)
        goto clean0;

    fRetCode = pDir->StartScan();
    if (!fRetCode)
        goto clean0;

    pDir = (CVirtualDirManager*)m_ctrlVirtualDirs.GetItemData(0, 2);
    if (!pDir)
        goto clean0;

    fRetCode = pDir->StartScan();
    if (!fRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL CUIHandlerBigFile::DelBigFile(const CString& strFilePath)
{
    BOOL retval = FALSE;
    CString strMsg;
    CString strTitle;
    UINT_PTR uRetCode;
    CVirtualDirManager* pDir = m_pCurrentDir;
    CKuiMsgBox msg1, msg2;
    CString strShowPath = strFilePath;

    bigfilehelper::CompressedPath(strShowPath);
    strMsg.Format(_T("您确认要删除“%s”到回收站吗？"), strShowPath);

   // strTitle = _T("金山卫士");
    uRetCode = msg1.ShowMsg(strMsg, NULL, MB_OKCANCEL|MB_ICONQUESTION);
    if (uRetCode != IDOK)
        goto clean0;

    retval = bigfilehelper::Recycle(strFilePath);
    if (retval)
    {
        if (pDir)
        {
            pDir->DeleteBigFile(strFilePath);
            UpdateHeaderTip();
        }
    }
    else
    {
        strMsg.Format(_T("删除“%s”到回收站失败！您可以定位到该文件再尝试手工删除。"), strShowPath);
        uRetCode = msg2.ShowMsg(strMsg, NULL, MB_OK|MB_ICONWARNING);
    }

clean0:
    return retval;
}

BOOL CUIHandlerBigFile::DoDelJobs(const CString& strFilePath, BOOL bType)
{
    return TRUE;
}

BOOL CUIHandlerBigFile::SaveAutoDestDir(CString& strDir)
{
    BOOL retval = FALSE;

    TiXmlDocument xmlDoc;  
    TiXmlDeclaration *pXmlDecl = new TiXmlDeclaration("1.0", "utf-8", "yes"); 
    TiXmlElement *pXmlSetting = new TiXmlElement("setting");
    TiXmlElement *pXmlDestDir = new TiXmlElement("destdir");
    TiXmlElement *pXmlChild = NULL;
    KFilePath savePath = KFilePath::GetFilePath(_Module.m_hInst);

    savePath.RemoveFileSpec();
    savePath.Append(L"cfg\\bigfilemgr.xml");
    xmlDoc.LinkEndChild(pXmlDecl);

    pXmlDestDir->SetAttribute("path", KUTF16_To_UTF8(strDir));
    pXmlSetting->LinkEndChild(pXmlDestDir);
    xmlDoc.LinkEndChild(pXmlSetting);

    retval = xmlDoc.SaveFile(UnicodeToAnsi(savePath.value()).c_str());

    //clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////


