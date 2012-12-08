#include "stdafx.h"
#include "bigfilemgr.h"
#include "tinyxml.h"
#include "misc/kregfunction.h"
#include "misc/IniEditor.h"

//////////////////////////////////////////////////////////////////////////

CBigFileMgr::CBigFileMgr()
    : m_fNeedReboot(FALSE)
{

}

CBigFileMgr::~CBigFileMgr()
{

}

BOOL CBigFileMgr::Init()
{
    return TRUE;
}

void CBigFileMgr::UnInit()
{

}

BOOL CBigFileMgr::NeedReboot()
{
    return m_fNeedReboot;
}

//////////////////////////////////////////////////////////////////////////

BOOL _CreateDirectory(const CString& strDir)
{
    BOOL retval = FALSE;
    CString strTemp = strDir;
    CString strBase;
    int nFind;

    if (GetFileAttributes(strDir) == FILE_ATTRIBUTE_DIRECTORY)
    {
        retval = TRUE;
        goto clean0;
    }

    if (strTemp[strTemp.GetLength() - 1] != _T('\\'))
        strTemp += _T("\\");

    nFind = strTemp.Find(_T("\\"));
    while (nFind != -1)
    {
        strBase += strTemp.Left(nFind + 1);
        strTemp.Delete(0, nFind + 1);

        if (GetFileAttributes(strBase) == INVALID_FILE_ATTRIBUTES)
        {
            if (!CreateDirectory(strBase, NULL))
                goto clean0;
        }

        nFind = strTemp.Find(_T("\\"));
    }

    retval = TRUE;

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////\

typedef enum _System_Version
{
    enumSystem_Unkonw,
    enumSystem_Win_95,
    enumSystem_Win_98,
    enumSystem_Win_me,
    enumSystem_Win_nt,
    enumSystem_Win_2000,
    enumSystem_Win_xp,
    enumSystem_Win_Ser_2003,
    enumSystem_Win_Vista,
    enumSystem_Win_7
} SYSTEM_VERSION;
int KAEGetSystemVersion(SYSTEM_VERSION *pnSystemVersion);
BOOL FindFileInDirectory(LPCTSTR pszFullPath,CString &filePathName);

WCHAR GetSystemDrive()
{
    
    WCHAR szPath[MAX_PATH] = { 0 };
    ::GetSystemDirectory(szPath,MAX_PATH);
    CString strsysDriver(szPath);
    return strsysDriver.GetAt(0);
}

BOOL _GetDrive(CAtlList<CString>& dirs)  //获取卷标名称
{
    int nPos = 0;
    CString strDrive = "?:";
    DWORD dwDriveList = ::GetLogicalDrives();
    BOOL  bRet = FALSE;

    dirs.RemoveAll();

    while (dwDriveList)
    {
        if (dwDriveList & 1)
        {	
            strDrive.SetAt (0, 'A' + nPos);
            {
                int nDiskType = GetDriveType(strDrive);   
                switch(nDiskType)
                {   
                case  DRIVE_NO_ROOT_DIR:
                    break;
                case  DRIVE_REMOVABLE: 
                    break;
                case  DRIVE_FIXED: 
                    {
                        if(strDrive.GetAt(0)==GetSystemDrive())
                            dirs.AddTail(strDrive);
                    }
                    break;
                case   DRIVE_REMOTE:
                    break;   
                case   DRIVE_CDROM:
                    break;
                }
            }
        }
        dwDriveList >>= 1;
        nPos++;
    }
    bRet = TRUE;
    return bRet;
}

BOOL _GetMyDocumentDir(CAtlList<CString>& dirs)
{
    BOOL bRet = TRUE;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    dirs.RemoveAll();
    bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
    if (bRet)
        if (GetSystemDrive() == szAppDataPath[0]||szAppDataPath[0]==GetSystemDrive()-32||szAppDataPath[0]==GetSystemDrive()+32)
            dirs.AddTail(szAppDataPath);

    return bRet;
}

BOOL _GetMyDeskTopDir(CAtlList<CString>& dirs)
{
    BOOL bRet = TRUE;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    dirs.RemoveAll();
    bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_DESKTOP, FALSE);
    if(bRet)
        if (GetSystemDrive() == szAppDataPath[0]||szAppDataPath[0]==GetSystemDrive()-32||szAppDataPath[0]==GetSystemDrive()+32)
            dirs.AddTail(szAppDataPath);

    return bRet;
}

BOOL _GetMyDocument(CString& dirs)
{
    BOOL bRet = TRUE;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
   
    bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
    if (bRet)
        if (GetSystemDrive() == szAppDataPath[0]||szAppDataPath[0]==GetSystemDrive()-32||szAppDataPath[0]==GetSystemDrive()+32)
            dirs = szAppDataPath;

    return bRet;
}

BOOL _GetMyDeskTop(CString& dirs)
{
    BOOL bRet = TRUE;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };

    bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_DESKTOP, FALSE);
    if(bRet)
        if (GetSystemDrive() == szAppDataPath[0]||szAppDataPath[0]==GetSystemDrive()-32||szAppDataPath[0]==GetSystemDrive()+32)
            dirs = szAppDataPath;

    return bRet;
}

BOOL _GetDownLoadsDir(CString& dirs)
{
    BOOL bRet = FALSE;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    
    SYSTEM_VERSION     m_eSysVer;
    KAEGetSystemVersion(&m_eSysVer);

    if(m_eSysVer == enumSystem_Win_7)
    {
        bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
        if(bRet)
        {
            CString tmpdir(szAppDataPath);
            tmpdir = tmpdir.Left(tmpdir.ReverseFind(L'\\'));
            tmpdir += L"\\downloads";
            if(tmpdir.GetAt(0)==GetSystemDrive())
                dirs = tmpdir;
        }

    }
    else
    {
        bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
        if(bRet)
        {
            wcscat(szAppDataPath,L"\\下载");
            if(szAppDataPath[0]==GetSystemDrive())
                dirs = szAppDataPath;
        }
    }

    return bRet;
}

BOOL _GetFlashGetBigDirs(CAtlList<CString>& dirs)
{

    SYSTEM_VERSION     m_eSysVer;
    KAEGetSystemVersion(&m_eSysVer);
    IniEditor IniEdit;
    BOOL bRet = TRUE;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };

    dirs.RemoveAll();

    WCHAR szAppPath[MAX_PATH] = {0};
    szAppPath[0] = GetSystemDrive();
    wcscat(szAppPath,L":\\Downloads");
    dirs.AddTail(szAppPath);

    if(m_eSysVer == enumSystem_Win_7)
    {
        CString tmpStrPath = L"";
        bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_APPDATA, FALSE);
        if (!bRet)
            goto clean0;
        tmpStrPath += szAppDataPath;
        tmpStrPath += L"\\FlashGet\\v3\\dat\\Appsetting.cfg";
        IniEdit.SetFile(tmpStrPath.GetBuffer());
        std::wstring UserPath = IniEdit.ReadString(L"FlashGet3 Setting Value",L"UserPath");
        if(UserPath.length()==0)
            bRet = FALSE;
        else 
        {
            
            
            if(UserPath[0]==GetSystemDrive()||UserPath[0]==GetSystemDrive()-32||UserPath[0]==GetSystemDrive()+32)
                dirs.AddTail(UserPath.c_str());
            else bRet = FALSE;
        }
    }
    else
    {
        TCHAR szLongPathBuffer[MAX_PATH] = { 0 };
        DWORD len = sizeof(szLongPathBuffer);
        GetRegistryValue(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\FlashGet Network\\FlashGet 3",
            L"Path",
            NULL,
            (LPBYTE)szLongPathBuffer,
            &len
            );
        if(wcslen(szLongPathBuffer) == 0)
            bRet = FALSE;
        else 
        {
            ::PathAppend(szLongPathBuffer,L"dat\\Appsetting.cfg");
            IniEdit.SetFile(szLongPathBuffer);
            std::wstring UserPath = IniEdit.ReadString(L"FlashGet3 Setting Value",L"UserPath");
            if(UserPath.length()==0)
                bRet = FALSE;
            else 
            {
                if(UserPath.at(0)==GetSystemDrive()||UserPath[0]==GetSystemDrive()-32||UserPath[0]==GetSystemDrive()+32)
                    dirs.AddTail(UserPath.c_str());
                else bRet = FALSE;
            }

        }
    }
clean0:
    return bRet;
}

BOOL _SetFlashGetBigDirs(const CString &strDir)
{

    SYSTEM_VERSION     m_eSysVer;
    KAEGetSystemVersion(&m_eSysVer);
    IniEditor IniEdit;
    BOOL bRet = TRUE;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };


    if(m_eSysVer == enumSystem_Win_7)
    {
        CString tmpStrPath = L"";
        bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_APPDATA, FALSE);
        if (!bRet)
            goto clean0;
        tmpStrPath += szAppDataPath;
        tmpStrPath += L"\\FlashGet\\v3\\dat\\Appsetting.cfg";
        IniEdit.SetFile(tmpStrPath.GetBuffer());
        CString strWTmp = strDir;
        IniEdit.WriteString(L"FlashGet3 Setting Value",L"UserPath",strWTmp.GetBuffer());
       

    }
    else
    {
        TCHAR szLongPathBuffer[MAX_PATH] = { 0 };
        DWORD len = sizeof(szLongPathBuffer);
        GetRegistryValue(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\FlashGet Network\\FlashGet 3",
            L"Path",
            NULL,
            (LPBYTE)szLongPathBuffer,
            &len
            );
        if(wcslen(szLongPathBuffer) == 0)
            bRet = FALSE;
        else 
        {
            ::PathAppend(szLongPathBuffer,L"dat\\Appsetting.cfg");
            IniEdit.SetFile(szLongPathBuffer);
            CString strWTmp = strDir;
            IniEdit.WriteString(L"FlashGet3 Setting Value",L"UserPath",strWTmp.GetBuffer());

        }
    }
clean0:
    return bRet;
}

BOOL _GetXunLeiBigDirs(CAtlList<CString>& dirs)
{
    BOOL bRet = TRUE;
    TCHAR szLongPathBuffer[MAX_PATH] = { 0 };
    DWORD len = sizeof(szLongPathBuffer);

    dirs.RemoveAll();

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\TDDownload");
    dirs.AddTail(szAppDataPath);

    GetRegistryValue(HKEY_CURRENT_USER,
        L"Software\\Thunder Network\\Thunder",
        L"DefaultPath",
        NULL,
        (LPBYTE)szLongPathBuffer,
        &len
        );
    if(wcslen(szLongPathBuffer) == 0)
        bRet = FALSE;
    else 
    {
        if(szLongPathBuffer[0]==GetSystemDrive()||szLongPathBuffer[0]==GetSystemDrive()-32||szLongPathBuffer[0]==GetSystemDrive()+32)
            dirs.AddTail(szLongPathBuffer);
        else bRet = FALSE;
          
    }
    return bRet;
}

BOOL _SetXunLeiBigDirs(const CString strDir)
{
    BOOL bRet = TRUE;
  
    DWORD len = strDir.GetLength();

   
    CString strTmp = strDir;
    SetRegistryValue(HKEY_CURRENT_USER,
        L"Software\\Thunder Network\\Thunder",
        L"DefaultPath",
        REG_SZ,
        (LPBYTE)strTmp.GetBuffer(),
        len
        );
    
    return bRet;
}

BOOL _GetYouKuBigDirs(CAtlList<CString>& dirs)
{
    BOOL retval = FALSE;
    TCHAR szLongPathBuffer[MAX_PATH] = { 0 };
    DWORD len = sizeof(szLongPathBuffer);

    dirs.RemoveAll();

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\Youku\\download");
    dirs.AddTail(szAppDataPath);

    GetRegistryValue(HKEY_CURRENT_USER,
        L"SOFTWARE\\youku\\iKu",
        L"ChangeDataDirectoryTo",
        NULL,
        (LPBYTE)szLongPathBuffer,
        &len
        );
    if(wcslen(szLongPathBuffer) == 0)
        goto clean0;
    else
    {
        if(szLongPathBuffer[0]==GetSystemDrive()||szLongPathBuffer[0]==GetSystemDrive()-32||szLongPathBuffer[0]==GetSystemDrive()+32)
            dirs.AddTail(szLongPathBuffer);
        else goto clean0;
    }

    retval = TRUE;

clean0:
    return retval;
}


BOOL _SetYouKuBigDirs(CString strDir)
{
    BOOL retval = FALSE;

    DWORD len = strDir.GetLength();


    SetRegistryValue(HKEY_CURRENT_USER,
        L"SOFTWARE\\youku\\iKu",
        L"ChangeDataDirectoryTo",
        REG_SZ,
        (LPBYTE)strDir.GetBuffer(),
        len
        );
    retval = TRUE;

    return retval;
}

BOOL _GetBitCometBigDirs(CAtlList<CString>& dirs)
{
    BOOL retval = FALSE;
    TiXmlDocument xmlDoc;
    const TiXmlElement *pBT = NULL;
    const TiXmlElement *pSet = NULL;
    const TiXmlElement *pDownLoad = NULL;
    const char* szDefaultDownPath = NULL;
    CString strDirs;
    int pos = -1;
    TCHAR szAppPath[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;
    CRegKey regKey;
    LONG lRetCode;

    dirs.RemoveAll();

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\downloads");
    dirs.AddTail(szAppDataPath);

    lRetCode = regKey.Open(HKEY_CURRENT_USER, _T("Software\\BitComet"), KEY_READ);
    if (lRetCode)
        goto clean0;

    lRetCode = regKey.QueryStringValue(NULL, szAppPath, &dwLength);
    if (lRetCode)
        goto clean0;

    PathRemoveFileSpec(szAppPath);
    PathAppend(szAppPath, _T("BitComet.xml"));

    if (GetFileAttributes(szAppPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    if (!xmlDoc.LoadFile(UnicodeToAnsi(szAppPath).c_str(), TIXML_ENCODING_UTF8))
        goto clean0;

    pBT = xmlDoc.FirstChildElement("BitComet");
    if (!pBT)
        goto clean0;

    pSet = xmlDoc.FirstChildElement("Settings");
    if (!pSet)
        goto clean0;

    pDownLoad = xmlDoc.FirstChildElement("DefaultDownloadPath");
    if (!pDownLoad)
        goto clean0;


    szDefaultDownPath = pDownLoad->GetText();
    if (!szDefaultDownPath)
        goto clean0;

    strDirs = Utf8ToUnicode(szDefaultDownPath).c_str();
    if (GetFileAttributes(strDirs) != INVALID_FILE_ATTRIBUTES)
    { 
        if(strDirs.GetAt(0)==GetSystemDrive()||strDirs.GetAt(0)==GetSystemDrive()-32||strDirs.GetAt(0)==GetSystemDrive()+32)
            dirs.AddTail(strDirs);
        else goto clean0;
    }
    retval = TRUE;

clean0:
    return retval;
}

BOOL _SetBitCometBigDirs(const CString& strDir)
{
    BOOL retval = FALSE;
    TiXmlDocument xmlDoc;
    const TiXmlElement *pBT = NULL;
    const TiXmlElement *pSet = NULL;
    const TiXmlElement *pDownLoad = NULL;
    const char* szDefaultDownPath = NULL;
    CString strDirs;
    int pos = -1;
    TCHAR szAppPath[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;
    CRegKey regKey;
    LONG lRetCode;


    lRetCode = regKey.Open(HKEY_CURRENT_USER, _T("Software\\BitComet"), KEY_READ);
    if (lRetCode)
        goto clean0;

    lRetCode = regKey.QueryStringValue(NULL, szAppPath, &dwLength);
    if (lRetCode)
        goto clean0;

    PathRemoveFileSpec(szAppPath);
    PathAppend(szAppPath, _T("BitComet.xml"));

    if (GetFileAttributes(szAppPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    if (!xmlDoc.LoadFile(UnicodeToAnsi(szAppPath).c_str(), TIXML_ENCODING_UTF8))
        goto clean0;

    pBT = xmlDoc.FirstChildElement("BitComet");
    if (!pBT)
        goto clean0;

    pSet = xmlDoc.FirstChildElement("Settings");
    if (!pSet)
        goto clean0;
   
    retval = TRUE;

clean0:
    return retval;
}


BOOL _GetKu6BigDirs(CAtlList<CString>& dirs)
{
    BOOL retval = FALSE;
    CRegKey regKey;
    LONG lRetCode;
    TCHAR szAppIniPath[MAX_PATH] = { 0 };
    TCHAR szDownloadPath[MAX_PATH] = { 0 };
    TCHAR szAppDataPath[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;
    DWORD dwRetCode;

    dirs.RemoveAll();

    retval = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
    if (retval)
        if (GetSystemDrive() == szAppDataPath[0])
        {
            CString strTmpPathDown(szAppDataPath);
            CString strTmpPathTrans(szAppDataPath);
            strTmpPathDown += L"\\酷6视频\\我下载的视频";
            strTmpPathTrans += L"\\酷6视频\\我转换的视频";
            dirs.AddTail(strTmpPathDown);
            dirs.AddTail(strTmpPathTrans);
        }


    lRetCode = regKey.Open(HKEY_LOCAL_MACHINE, 
        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ku6SpeedUpper"), KEY_READ);
    if (lRetCode)
        goto clean0;

    lRetCode = regKey.QueryStringValue(_T("UninstallString"), szAppIniPath, &dwLength);
    if (lRetCode)
        goto clean0;

    if (GetFileAttributes(szAppIniPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    PathRemoveFileSpec(szAppIniPath);
    PathAppend(szAppIniPath, _T("Ku6SpeedUpper.ini"));

    dwRetCode = GetPrivateProfileString(
        _T("Settings"), 
        _T("DownPath"), 
        _T(""), 
        szDownloadPath, 
        MAX_PATH,
        szAppIniPath
        );
    if (!dwRetCode)
        goto clean0;

    if (GetFileAttributes(szDownloadPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    if(szDownloadPath[0]==GetSystemDrive()||szDownloadPath[0]==GetSystemDrive()-32||szDownloadPath[0]==GetSystemDrive()+32)
        dirs.AddTail(szDownloadPath);
    else goto clean0;

    retval = TRUE;

clean0:
    return retval;
}


BOOL _SetKu6BigDirs(const CString& strDir)
{
    BOOL retval = FALSE;
    CRegKey regKey;
    LONG lRetCode;
    TCHAR szAppIniPath[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;
    DWORD dwRetCode;

    _CreateDirectory(strDir);

    lRetCode = regKey.Open(HKEY_LOCAL_MACHINE, 
        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ku6SpeedUpper"), KEY_READ);
    if (lRetCode)
        goto clean0;

    lRetCode = regKey.QueryStringValue(_T("UninstallString"), szAppIniPath, &dwLength);
    if (lRetCode)
        goto clean0;

    if (GetFileAttributes(szAppIniPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    PathRemoveFileSpec(szAppIniPath);
    PathAppend(szAppIniPath, _T("Ku6SpeedUpper.ini"));

    dwRetCode = WritePrivateProfileString(
        _T("Settings"), 
        _T("DownPath"), 
        strDir, 
        szAppIniPath
        );
    if (!dwRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL _GetXunleiKankanBigDirs(CAtlList<CString>& dirs)
{
    BOOL retval = FALSE;
    CRegKey regKey;
   
    TCHAR szKankanCacheDir[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;

    dirs.RemoveAll();

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\kankan");
    dirs.AddTail(szAppDataPath);

    GetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Thunder network\\Xmp",
        L"theaterstorepath",
        NULL,
        (LPBYTE)szKankanCacheDir,
        &dwLength
        );
    
    if (GetFileAttributes(szKankanCacheDir) == INVALID_FILE_ATTRIBUTES)
        goto clean0;


    if(szKankanCacheDir[0]==GetSystemDrive()||szKankanCacheDir[0]==GetSystemDrive()-32||szKankanCacheDir[0]==GetSystemDrive()+32)

        dirs.AddTail(szKankanCacheDir);
    else goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL _SetXunleiKankanBigDirs(const CString& strDir)
{
    BOOL retval = FALSE;
    CRegKey regKey;
    LONG lRetCode;

    _CreateDirectory(strDir);

    lRetCode = regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Thunder network\\Xmp"), KEY_READ);
    if (lRetCode)
        goto clean0;

    lRetCode = regKey.SetStringValue(_T("theaterstorepath"), strDir, REG_SZ);
    if (lRetCode)
        goto clean0;



    retval = TRUE;

clean0:
    return retval;
}

BOOL _GetQQDownLoadBigDirs(CAtlList<CString>& dirs)
{
    BOOL retval = FALSE;
    BOOL fRetCode;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    WCHAR szCachePath[MAX_PATH] = { 0 };
    CString szFilePath = L"";
    DWORD dwRetCode;

    dirs.RemoveAll();

    WCHAR szAppPath[MAX_PATH] = {0};
    szAppPath[0] = GetSystemDrive();
    wcscat(szAppPath,L":\\qqdownload");
    dirs.AddTail(szAppPath);

    fRetCode = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_APPDATA, FALSE);
    if (!fRetCode)
        goto clean0;

    PathAppend(szAppDataPath, _T("QQDownload\\"));
    if (GetFileAttributes(szAppDataPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    fRetCode = FindFileInDirectory(szAppDataPath,szFilePath);
    if(!fRetCode || szFilePath.GetLength() == 0)
    {
        goto clean0;
    }

    dwRetCode = GetPrivateProfileString(
        _T("TaskInfo"),
        _T("DefaultSavePath"),
        _T(""),
        szCachePath, 
        MAX_PATH,
        szFilePath.GetBuffer()
        );
    if (!dwRetCode)
        goto clean0;

    if (GetFileAttributes(szCachePath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    if(szCachePath[0]==GetSystemDrive()||szCachePath[0]==GetSystemDrive()-32||szCachePath[0]==GetSystemDrive()+32)
        dirs.AddTail(szCachePath);
    else goto clean0;

    retval = TRUE;

clean0:
    return retval;
}


BOOL _GetQQMusicBigDirs(CAtlList<CString>& dirs)
{
    BOOL retval = FALSE;
    BOOL fRetCode;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    WCHAR szCachePath[MAX_PATH] = { 0 };
    DWORD dwRetCode;

    dirs.RemoveAll();

    fRetCode = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_APPDATA, FALSE);
    if (!fRetCode)
        goto clean0;

    PathAppend(szAppDataPath, _T("Tencent\\QQMusic\\CachePath.ini"));
    if (GetFileAttributes(szAppDataPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    dwRetCode = GetPrivateProfileString(
        _T("Cache"),
        _T("path"),
        _T(""),
        szCachePath, 
        MAX_PATH,
        szAppDataPath
        );
    if (!dwRetCode)
        goto clean0;

    if (GetFileAttributes(szCachePath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    if(szCachePath[0]==GetSystemDrive()||szCachePath[0]==GetSystemDrive()-32||szCachePath[0]==GetSystemDrive()+32)
        dirs.AddTail(szCachePath);
    else goto clean0;


    retval = TRUE;

clean0:
    return retval;
}

BOOL _SetQQMusicBigDirs(const CString& strDir)
{
    BOOL retval = FALSE;
    BOOL fRetCode;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    DWORD dwRetCode;

    _CreateDirectory(strDir);

    fRetCode = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_APPDATA, FALSE);
    if (!fRetCode)
        goto clean0;

    PathAppend(szAppDataPath, _T("Tencent\\QQMusic\\CachePath.ini"));
    if (GetFileAttributes(szAppDataPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    dwRetCode = WritePrivateProfileString(
        _T("Cache"),
        _T("path"),
        strDir, 
        szAppDataPath
        );
    if (!dwRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL _GetKuwoBigDirs(CAtlList<CString>& dirs)
{
    BOOL retval = FALSE;
    TCHAR szAppPath[MAX_PATH] = { 0 };
    TCHAR szDownloadPath[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;
    DWORD dwRetCode;
    CRegKey regKey;
    

    dirs.RemoveAll();

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\kwdownload\\song");
    dirs.AddTail(szAppDataPath);

   

    GetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\KWMUSIC",
        L"KWMUSIC_HOME",
        NULL,
        (LPBYTE)szAppPath,
        &dwLength
        );
    if(wcslen(szAppPath) == 0)
        goto clean0;
   // PathRemoveFileSpec(szAppPath);
    PathAppend(szAppPath, _T("config.ini"));
    if (GetFileAttributes(szAppPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    dwRetCode = GetPrivateProfileString(
        _T("Setting"), 
        _T("songpath"), 
        _T("C:\\KwDownload\\song"), 
        szDownloadPath, 
        MAX_PATH,
        szAppPath
        );
    if (!dwRetCode)
        goto clean0;

    if (GetFileAttributes(szDownloadPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    if(szDownloadPath[0]==GetSystemDrive()||szDownloadPath[0]==GetSystemDrive()-32||szDownloadPath[0]==GetSystemDrive()+32)
        dirs.AddTail(szDownloadPath);
    else goto clean0;


    retval = TRUE;

clean0:
    return retval;
}

BOOL _SetKuwoBigDirs(const CString& strDir)
{
    BOOL retval = FALSE;
    TCHAR szAppPath[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;
    DWORD dwRetCode;
    CRegKey regKey;
    LONG lRetCode;

    _CreateDirectory(strDir);

    lRetCode = regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\KWMUSIC"), KEY_READ);
    if (lRetCode)
        goto clean0;

    lRetCode = regKey.QueryStringValue(_T("KWMUSIC_HOME"), szAppPath, &dwLength);
    if (lRetCode)
        goto clean0;

    PathRemoveFileSpec(szAppPath);
    PathAppend(szAppPath, _T("config.ini"));
    if (GetFileAttributes(szAppPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    dwRetCode = WritePrivateProfileString(
        _T("Setting"), 
        _T("songpath"), 
        strDir,
        szAppPath
        );
    if (!dwRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL _GetKugouBigDirs(CAtlList<CString>& dirs)
{
    BOOL retval = FALSE;
    TiXmlDocument xmlDoc;
    const TiXmlElement *pKugoo = NULL;
    const TiXmlElement *pMisc = NULL;
    const char* szDefaultDownPath = NULL;
    CString strDirs;
    int pos = -1;
    TCHAR szAppPath[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;
    CRegKey regKey;
    LONG lRetCode;

    dirs.RemoveAll();

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\kugou");
    dirs.AddTail(szAppDataPath);

    lRetCode = regKey.Open(HKEY_CURRENT_USER, _T("Software\\KuGoo6"), KEY_READ);
    if (lRetCode)
        goto clean0;

    lRetCode = regKey.QueryStringValue(_T("AppFileName"), szAppPath, &dwLength);
    if (lRetCode)
        goto clean0;

    PathRemoveFileSpec(szAppPath);
    PathAppend(szAppPath, _T("KuGoo.xml"));

    if (GetFileAttributes(szAppPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    if (!xmlDoc.LoadFile(UnicodeToAnsi(szAppPath).c_str(), TIXML_ENCODING_UTF8))
        goto clean0;

    pKugoo = xmlDoc.FirstChildElement("KuGoo");
    if (!pKugoo)
        goto clean0;

    pMisc = xmlDoc.FirstChildElement("Misc");
    if (!pMisc)
        goto clean0;

    szDefaultDownPath = pMisc->Attribute("DefaultDownPath");
    if (!szDefaultDownPath)
        goto clean0;

    strDirs = Utf8ToUnicode(szDefaultDownPath).c_str();
    pos = strDirs.Find(_T(";"));
    while (pos != -1)
    {
        CString strDir = strDirs.Left(pos);
        if (strDir.GetLength())
        {
            if (GetFileAttributes(strDir) != INVALID_FILE_ATTRIBUTES)
                if (strDir.GetAt(0)==GetSystemDrive()||strDir[0]==GetSystemDrive()-32||strDir[0]==GetSystemDrive()+32)
                  dirs.AddTail(strDir);
        }

        strDirs.Delete(0, pos);
        pos = strDirs.Find(_T(";"));
    }

    retval = TRUE;

clean0:
    return retval;
}

BOOL _SetKugouBigDirs(const CString& strDir)
{
    BOOL retval = FALSE;
    TiXmlDocument xmlDoc;
    TiXmlElement *pKugoo = NULL;
    TiXmlElement *pMisc = NULL;
    int pos = -1;
    TCHAR szAppPath[MAX_PATH] = { 0 };
    ULONG dwLength = MAX_PATH;
    CRegKey regKey;
    LONG lRetCode;
    CStringA strNewDir = KUTF16_To_UTF8(strDir + _T(";"));

    lRetCode = regKey.Open(HKEY_CURRENT_USER, _T("Software\\KuGoo6"), KEY_READ);
    if (lRetCode)
        goto clean0;

    lRetCode = regKey.QueryStringValue(_T("AppFileName"), szAppPath, &dwLength);
    if (lRetCode)
        goto clean0;

    PathRemoveFileSpec(szAppPath);
    PathAppend(szAppPath, _T("KuGoo.xml"));

    if (GetFileAttributes(szAppPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    if (!xmlDoc.LoadFile(UnicodeToAnsi((LPCTSTR)szAppPath).c_str(), TIXML_ENCODING_UTF8))
        goto clean0;

    pKugoo = xmlDoc.FirstChildElement("KuGoo");
    if (!pKugoo)
        goto clean0;

    pMisc = xmlDoc.FirstChildElement("Misc");
    if (!pMisc)
        goto clean0;

    pMisc->SetAttribute(
        "DefaultDownPath", 
        (LPCSTR)strNewDir
        );
    if (!xmlDoc.SaveFile(UnicodeToAnsi((LPCTSTR)szAppPath).c_str()))
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

int KAEGetSystemVersion(SYSTEM_VERSION *pnSystemVersion)
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	*pnSystemVersion = enumSystem_Unkonw;

	memset(&osvi, 0, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return FALSE;
	}

	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:

		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
			*pnSystemVersion = enumSystem_Win_Vista;

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
			*pnSystemVersion = enumSystem_Win_Ser_2003;

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			*pnSystemVersion = enumSystem_Win_xp;

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			*pnSystemVersion = enumSystem_Win_2000;

		if ( osvi.dwMajorVersion <= 4 )
			*pnSystemVersion = enumSystem_Win_nt;
        if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 )
            *pnSystemVersion = enumSystem_Win_7;

	case VER_PLATFORM_WIN32_WINDOWS:

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			*pnSystemVersion = enumSystem_Win_95;
		} 

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			*pnSystemVersion = enumSystem_Win_98;
		} 

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			*pnSystemVersion = enumSystem_Win_me;
		} 
		break;

	default:
		*pnSystemVersion = enumSystem_Unkonw;
		break;
	}
	return TRUE; 
}

BOOL FindFileInDirectory(LPCTSTR pszFullPath,CString &filePathName)
{
    BOOL bResult = TRUE;
    WIN32_FIND_DATA ff = { 0 }; 


    // 递归搜索子目录
    TCHAR szFindName[MAX_PATH] = {0};
    _tcsncpy_s( szFindName, MAX_PATH, pszFullPath,  MAX_PATH - 1 );
    _tcsncat_s( szFindName, MAX_PATH, TEXT("*.*") , MAX_PATH - _tcslen(pszFullPath) - 1 );
    HANDLE findhandle = ::FindFirstFile( szFindName, &ff ); 

    if( findhandle == INVALID_HANDLE_VALUE )
    {
        bResult = FALSE;
        goto Exit0;
    }

    BOOL res = TRUE;

    while(res)
    {
        if( ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  
        {
            if (( _tcscmp( ff.cFileName, _T( "."  ) ) != 0) 
                && ( _tcscmp( ff.cFileName, _T( ".." ) ) != 0 ))
            {
                memset(szFindName, 0, sizeof(szFindName));
                _tcsncpy_s( szFindName, MAX_PATH, pszFullPath,
                    MAX_PATH - 1 );
                _tcsncat_s( szFindName, MAX_PATH, ff.cFileName, 
                    MAX_PATH - _tcslen(szFindName) - 1 );
                _tcsncat_s( szFindName, MAX_PATH,TEXT("\\"),
                    MAX_PATH - _tcslen(szFindName) - 1 );

                FindFileInDirectory( szFindName,filePathName);  
            }
        }
        else
        {
            if(::StrStrI(ff.cFileName,L"UserConfig.ini")!=NULL)
            {
                bResult = TRUE;
                filePathName = pszFullPath;
                filePathName += ff.cFileName;
                break;
            }
        }

        res = ::FindNextFile( findhandle, &ff );
    }

    ::FindClose( findhandle );
Exit0:
    return bResult;       
}

DWORD _DoGetFileSizeByFileName(const WCHAR* pFileName)
{
	if (!pFileName)
	{
		return 0;
	}

	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0;

	hFile = ::CreateFile(
		pFileName, 
		GENERIC_READ,
		0, 
		NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
		);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwFileSize = ::GetFileSize(hFile, NULL);
		::CloseHandle(hFile);
	}

	return dwFileSize;
}

DWORD _DoGetLineByBuf(WCHAR* const pszBuf,
					  DWORD dwlen,
					  std::vector<std::wstring>& vcStrline)
{
	if (!pszBuf)
	{
		return 0;
	}

	WCHAR* pszBeginPoint = pszBuf;
	WCHAR* pszCurrentPoint = pszBuf;
	WCHAR* pszTempBuf = NULL;
	DWORD dwlenSize = 0;
	DWORD dwLineCount = 0;

	while (*pszCurrentPoint != 0 && (unsigned)(pszCurrentPoint - pszBuf) <= dwlen)
	{
		if (*pszCurrentPoint == '\n')
		{
			dwlenSize = (DWORD)(pszCurrentPoint - pszBeginPoint);
			pszTempBuf = new WCHAR[dwlenSize + 1];
			if (!pszTempBuf)
			{
				break;
			}
			::ZeroMemory(pszTempBuf, (dwlenSize + 1)* 2);
			::memcpy(pszTempBuf, pszBeginPoint, dwlenSize * 2);

			pszBeginPoint = pszCurrentPoint + 1;

			vcStrline.push_back(pszTempBuf);
			dwLineCount++ ;

			delete []pszTempBuf;
			pszTempBuf = NULL;
		}

		pszCurrentPoint ++;
	}

	return dwLineCount;
}


BOOL _GetChormeBigDir(CAtlList<CString>& dirs)
{
    
   
	WCHAR szPath[MAX_PATH] = {0};
	BOOL bRet = FALSE;
	CString szPathFull;
	CString szPathFullEx;
	CString szTarget;
	int nPos = -1;
    CString strPath = L"";
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    dirs.RemoveAll();
    bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
    if(bRet)
    {
        wcscat(szAppDataPath,L"\\Downloads");
        if(szAppDataPath[0]==GetSystemDrive())
            dirs.AddTail(szAppDataPath);
    }

	bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_LOCAL_APPDATA, FALSE);

	//bRet = SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_DEFAULT, szPath);;
	szPathFull = szPath;
	szPathFullEx = szPath;
	szPathFullEx += _T("\\Google\\Chrome\\User Data\\Default\\Preferences");
	szPathFull += _T("\\Google\\Chrome\\User Data\\Default");

	WCHAR szRecord[MAX_PATH] = {0};
	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring>::iterator itor;

	const WCHAR* Point = NULL;
	HRESULT hrRet = E_FAIL;
	std::vector<std::wstring> vcStrline;
	BYTE* pBuf = NULL;
	FILE* pFile = NULL;
	DWORD dwRealReadSize = 0;

	DWORD dwFileSize  = _DoGetFileSizeByFileName(szPathFullEx);
	if (dwFileSize <= 0)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	pBuf = new BYTE[dwFileSize * 2 + 2];
	if (!pBuf)
	{
		hrRet = E_OUTOFMEMORY;
		goto _Exit;
	}

	::ZeroMemory(pBuf, dwFileSize * 2 + 2);

	pFile = ::_wfopen(szPathFullEx, L"rt,ccs=UTF-8");
	if (!pFile)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	dwRealReadSize = (DWORD)::fread(pBuf, sizeof(WCHAR), dwFileSize, pFile);
	if (dwRealReadSize == 0)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	WCHAR* pszInfo = (WCHAR*)pBuf;

	DWORD dwLineCount = _DoGetLineByBuf(pszInfo, (DWORD)::wcslen(pszInfo), vLogCache);

	for (itor = vLogCache.begin(); itor != vLogCache.end(); itor++)
	{
		szTarget = (*itor).c_str();
		nPos = szTarget.Find(L"default_directory");
		if (nPos != -1)
		{
			nPos = szTarget.Find(L":");
			szTarget = szTarget.Right(szTarget.GetLength() - nPos - 3);
			nPos = szTarget.Find(L"\"");
			szTarget = szTarget.Left(nPos);
			break;
		}
	}

// 	if (_wcsicmp(szRecord, L"") == 0)
// 	{
// 		bRet = FALSE;
// 		goto _Exit;
// 	}

    for (int nIndex=0; nIndex<szTarget.GetLength();)
    {
        if(szTarget.GetAt(nIndex)==L'\\'&&szTarget.GetAt(nIndex+1)==L'\\')
        {
            strPath += L"\\";
            nIndex += 2;
        }
        else
        {
            strPath += szTarget.GetAt(nIndex);
            nIndex++;
        }

    }

    if(strPath.GetAt(0)==GetSystemDrive()||strPath[0]==GetSystemDrive()-32||strPath[0]==GetSystemDrive()+32)
	    dirs.AddTail(strPath);
    else goto _Exit;
	hrRet = S_OK;
	bRet = TRUE;
_Exit:

	if (pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	if (pFile)
	{
		fclose(pFile);
		pFile = NULL;
	}
	return bRet;
}

BOOL _GetFirefoxBigDir(CAtlList<CString>& dirs)
{
	WCHAR szPath[MAX_PATH] = {0};
	BOOL bRet = FALSE;
	CString szPathFull;
	CString szPathFullEx;
	CString szTarget;
    CString strPath = L"";
	int nPos = -1;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    dirs.RemoveAll();
    SYSTEM_VERSION     m_eSysVer;
    KAEGetSystemVersion(&m_eSysVer);

    if(m_eSysVer == enumSystem_Win_7)
    {
        bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
        if(bRet)
        {
            CString tmpdir(szAppDataPath);
            tmpdir = tmpdir.Left(tmpdir.ReverseFind(L'\\'));
            tmpdir += L"\\downloads";
            if(tmpdir.GetAt(0)==GetSystemDrive())
                dirs.AddTail(tmpdir);
        }
    
    }
    else
    {
        bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
        if(bRet)
        {
            wcscat(szAppDataPath,L"\\下载");
            if(szAppDataPath[0]==GetSystemDrive())
                dirs.AddTail(szAppDataPath);
        }
    }

	bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
	szPathFull = szPath;
	szPathFullEx = szPath;
	szPathFullEx += _T("\\Mozilla\\Firefox");
	szPathFull += _T("\\Mozilla\\Firefox\\profiles.ini");

	WCHAR szRecord[MAX_PATH] = {0};
	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring>::iterator itor;

	GetPrivateProfileString(L"Profile0", L"Path", NULL, szRecord, MAX_PATH - 1, szPathFull);

	szPathFullEx += L"\\";
	szPathFullEx += szRecord;

	szPathFullEx += L"\\prefs.js";

	const WCHAR* Point = NULL;
	HRESULT hrRet = E_FAIL;
	std::vector<std::wstring> vcStrline;
	BYTE* pBuf = NULL;
	FILE* pFile = NULL;
	DWORD dwRealReadSize = 0;

	DWORD dwFileSize  = _DoGetFileSizeByFileName(szPathFullEx);
	if (dwFileSize <= 0)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	pBuf = new BYTE[dwFileSize * 2 + 2];
	if (!pBuf)
	{
		hrRet = E_OUTOFMEMORY;
		goto _Exit;
	}

	::ZeroMemory(pBuf, dwFileSize * 2 + 2);

	pFile = ::_wfopen(szPathFullEx, L"rt,ccs=UTF-8");
	if (!pFile)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	dwRealReadSize = (DWORD)::fread(pBuf, sizeof(WCHAR), dwFileSize, pFile);
	if (dwRealReadSize == 0)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	WCHAR* pszInfo = (WCHAR*)pBuf;

	DWORD dwLineCount = _DoGetLineByBuf(pszInfo, (DWORD)::wcslen(pszInfo), vLogCache);

	for (itor = vLogCache.begin(); itor != vLogCache.end(); itor++)
	{
		szTarget = (*itor).c_str();
		nPos = szTarget.Find(L"browser.download.dir");
		if (nPos != -1)
		{
			nPos = szTarget.Find(L",");
			szTarget = szTarget.Right(szTarget.GetLength() - nPos - 3);
			nPos = szTarget.Find(L"\"");
			szTarget = szTarget.Left(nPos);
			break;
		}
	}
	if (_wcsicmp(szTarget, L"") == 0)
	{
		return FALSE;
	}

    for (int nIndex=0; nIndex<szTarget.GetLength();)
    {
        if(szTarget.GetAt(nIndex)==L'\\'&&szTarget.GetAt(nIndex+1)==L'\\')
        {
            strPath += L"\\";
            nIndex += 2;
        }
        else
        {
            strPath += szTarget.GetAt(nIndex);
            nIndex++;
        }

    }
    if(strPath.GetAt(0)==GetSystemDrive()||strPath[0]==GetSystemDrive()-32||strPath[0]==GetSystemDrive()+32)
	    dirs.AddTail(strPath);
    else goto _Exit;
	hrRet = S_OK;

_Exit:

	if (pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	if (pFile)
	{
		fclose(pFile);
		pFile = NULL;
	}
	return TRUE;
}

BOOL _Get360BigDir(CAtlList<CString>& dirs)
{
	WCHAR szPath[MAX_PATH] = {0};
	BOOL bRet = FALSE;
	CString szPathFull;
     WCHAR szAppDataPath[MAX_PATH] = { 0 };
    dirs.RemoveAll();

    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\Downloads");
    dirs.AddTail(szAppDataPath);

	bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
	szPathFull = szPath;
	szPathFull += _T("\\360se\\extensions\\ExtDownload\\ExtDownload.ini");

	WCHAR szRecord[MAX_PATH] = {0};

	GetPrivateProfileString(L"Download", L"UserDefinePath", NULL, szRecord, MAX_PATH - 1, szPathFull);
	CString szFull = szRecord;

	if (_wcsicmp(szRecord, L"") == 0)
	{
		return FALSE;
	}
    if(szFull.GetAt(0)==GetSystemDrive()||szFull[0]==GetSystemDrive()-32||szFull[0]==GetSystemDrive()+32)
	    dirs.AddTail(szFull);
    else 
        return FALSE;
	return TRUE;
}

BOOL _GetMaxmonthBigDir(CAtlList<CString>& dirs)
{
	KSearchSoftwareStruct sss;
	wstring strPath;
	BOOL bRet = FALSE;
	CString szPathFull;
	WCHAR szPath[MAX_PATH] = {0};
    WCHAR szAppDataPath[MAX_PATH] = {0};
    dirs.RemoveAll();
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\MxDownload");
    dirs.AddTail(szAppDataPath);

	sss.pszMainFileName      = TEXT( "Maxthon.exe" );
	sss.hRegRootKey = HKEY_CURRENT_USER;
	sss.pszRegSubKey = _T("SOFTWARE\\Maxthon2");
	sss.pszPathValue = _T("Folder");

	bRet = SearchSoftwarePath( &sss, strPath );
	wcscpy(szPath, strPath.c_str());
	::PathRemoveFileSpec(szPath);
	szPathFull = szPath;

	szPathFull += _T("\\SharedAccount\\Config\\Downloader.ini");

	WCHAR szRecord[MAX_PATH] = {0};

	for (int i = 0; ;i++)
	{
		CString strTemp;
		strTemp.Format(_T("%d"), i);
		GetPrivateProfileString(L"COMBOX_HIS", strTemp, NULL, szRecord, MAX_PATH - 1, szPathFull);
		if (_wcsicmp(szRecord, L"") == 0)
		{
			break;
		}
		CString szFull = szRecord;
        if(szFull.GetAt(0)==GetSystemDrive()||szFull[0]==GetSystemDrive()-32||szFull[0]==GetSystemDrive()+32)
            dirs.AddTail(szFull);
       
	}
	return TRUE;
}

BOOL _GetTheworldBigDir(CAtlList<CString>& dirs)
{
	KSearchSoftwareStruct sss;
	wstring strPath;
	BOOL bRet = FALSE;
	CString szPathFull;
	WCHAR szPath[MAX_PATH] = {0};
    dirs.RemoveAll();

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\Downloads");
    dirs.AddTail(szAppDataPath);

	sss.pszMainFileName      = TEXT( "TheWorld.exe" );
	sss.hRegRootKey = HKEY_LOCAL_MACHINE;
	sss.pszRegSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\TheWorld.exe");
	sss.pszPathValue = _T("Path");
	sss.pszUninstallListName = _T("世界之窗浏览器");

	bRet = SearchSoftwarePath( &sss, strPath );
	wcscpy(szPath, strPath.c_str());
	::PathRemoveFileSpec(szPath);
	szPathFull = szPath;

	szPathFull += _T("\\TheWorld.ini");

	WCHAR szRecord[MAX_PATH] = {0};

	for (int i = 0; ;i++)
	{
		CString strTemp;
		strTemp.Format(_T("Path%d"), i);
		GetPrivateProfileString(L"Download", strTemp, NULL, szRecord, MAX_PATH - 1, szPathFull);
		if (_wcsicmp(szRecord, L"") == 0)
		{
			break;
		}
		CString szFull = szRecord;
        if(szFull.GetAt(0)==GetSystemDrive()||szFull[0]==GetSystemDrive()-32||szFull[0]==GetSystemDrive()+32)
		    dirs.AddTail(szFull);
	}
	return TRUE;
}

DWORD GetProcessIdFromName(LPCTSTR szProcessName)
{
	PROCESSENTRY32 pe;
	DWORD id = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if( !Process32First(hSnapshot,&pe) )
		return 0;
	while(1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if( Process32Next(hSnapshot,&pe)==FALSE )
			break;
		if(wcscmp(pe.szExeFile,szProcessName) == 0)
		{
			id = pe.th32ProcessID;
			break;
		}

	}
	CloseHandle(hSnapshot);
	return id;
}

BOOL _SetChormeBigDir(const CString& strDir)
{

	WCHAR szPath[MAX_PATH] = {0};
	BOOL bRet = FALSE;
	CString szPathFull;
	CString szPathFullEx;
	CString szTarget;
	int nPos = -1;
	bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_LOCAL_APPDATA, FALSE);

	//bRet = SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_DEFAULT, szPath);;
	szPathFull = szPath;
	szPathFullEx = szPath;
	szPathFullEx += _T("\\Google\\Chrome\\User Data\\Default\\Preferences");
	szPathFull += _T("\\Google\\Chrome\\User Data\\Default");

	WCHAR szRecord[MAX_PATH] = {0};
	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring> vLogCacheNew;
	std::vector<std::wstring>::iterator itor;

	const WCHAR* Point = NULL;
	HRESULT hrRet = E_FAIL;
	std::vector<std::wstring> vcStrline;
	BYTE* pBuf = NULL;
	FILE* pFile = NULL;
	DWORD dwRealReadSize = 0;

	DWORD dwFileSize  = _DoGetFileSizeByFileName(szPathFullEx);
	if (dwFileSize <= 0)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	pBuf = new BYTE[dwFileSize * 2 + 2];
	if (!pBuf)
	{
		hrRet = E_OUTOFMEMORY;
		goto _Exit;
	}

	::ZeroMemory(pBuf, dwFileSize * 2 + 2);

	pFile = ::_wfopen(szPathFullEx, L"rt,ccs=UTF-8");
	if (!pFile)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	dwRealReadSize = (DWORD)::fread(pBuf, sizeof(WCHAR), dwFileSize, pFile);
	if (dwRealReadSize == 0)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	WCHAR* pszInfo = (WCHAR*)pBuf;

	DWORD dwLineCount = _DoGetLineByBuf(pszInfo, (DWORD)::wcslen(pszInfo), vLogCache);

	for (itor = vLogCache.begin(); itor != vLogCache.end(); itor++)
	{
		szTarget = (*itor).c_str();
		nPos = szTarget.Find(L"default_directory");
		if (nPos != -1)
		{
			CString str;
			str.Format(_T("      \"default_directory\": \"%s\");"), strDir);
			szTarget = str;
		}
		wstring strTemp;
		strTemp = szTarget;
		vLogCacheNew.push_back(strTemp);
	}

	DeleteFile(szPathFullEx);

	FILE* pfile = NULL;

	{
		pfile = ::_wfopen(szPathFullEx, L"wt,ccs=UTF-8");
	}

	if (!pfile)
	{
		hrRet = S_OK;
		goto _Exit;
	}


	for (itor = vLogCacheNew.begin(); itor != vLogCacheNew.end(); itor++)
	{
		wstring szTargetEx; 
		szTargetEx = (*itor);
		DWORD dwSize = (DWORD)::fwrite(szTargetEx.c_str(), sizeof(WCHAR), ::wcslen(szTargetEx.c_str()), pfile);
		::fwrite(L"\n", sizeof(WCHAR), ::wcslen(L"\n"), pfile);
	}
_Exit:

	if (pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	if (pFile)
	{
		fclose(pFile);
		pFile = NULL;
	}
	return TRUE;
}
BOOL _SetFirefoxBigDir(const CString& strDir)
{
	WCHAR szPath[MAX_PATH] = {0};
	BOOL bRet = FALSE;
	CString szPathFull;
	CString szPathFullEx;
	CString szTarget;
	int nPos = -1;
	bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
	szPathFull = szPath;
	szPathFullEx = szPath;
	szPathFullEx += _T("\\Mozilla\\Firefox");
	szPathFull += _T("\\Mozilla\\Firefox\\profiles.ini");

	WCHAR szRecord[MAX_PATH] = {0};
	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring>::iterator itor;
	std::vector<std::wstring> vLogCacheNew;

	GetPrivateProfileString(L"Profile0", L"Path", NULL, szRecord, MAX_PATH - 1, szPathFull);

	szPathFullEx += L"\\";
	szPathFullEx += szRecord;

	szPathFullEx += L"\\prefs.js";

	const WCHAR* Point = NULL;
	HRESULT hrRet = E_FAIL;
	std::vector<std::wstring> vcStrline;
	BYTE* pBuf = NULL;
	FILE* pFile = NULL;
	DWORD dwRealReadSize = 0;

	DWORD dwFileSize  = _DoGetFileSizeByFileName(szPathFullEx);
	if (dwFileSize <= 0)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	pBuf = new BYTE[dwFileSize * 2 + 2];
	if (!pBuf)
	{
		hrRet = E_OUTOFMEMORY;
		goto _Exit;
	}

	::ZeroMemory(pBuf, dwFileSize * 2 + 2);

	pFile = ::_wfopen(szPathFullEx, L"rt,ccs=UTF-8");
	if (!pFile)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	dwRealReadSize = (DWORD)::fread(pBuf, sizeof(WCHAR), dwFileSize, pFile);
	if (dwRealReadSize == 0)
	{
		hrRet = E_FAIL;
		goto _Exit;
	}

	WCHAR* pszInfo = (WCHAR*)pBuf;

	DWORD dwLineCount = _DoGetLineByBuf(pszInfo, (DWORD)::wcslen(pszInfo), vLogCache);

	for (itor = vLogCache.begin(); itor != vLogCache.end(); itor++)
	{
		szTarget = (*itor).c_str();
		nPos = szTarget.Find(L"browser.download.dir");
		if (nPos != -1)
		{
			CString str;
			str.Format(_T("user_pref(\"browser.download.dir\", \"%s\");"), strDir);
			szTarget = str;
		}
		wstring strTemp;
		strTemp = szTarget;
		vLogCacheNew.push_back(strTemp);
	}

	DeleteFile(szPathFullEx);

	FILE* pfile = NULL;

	{
		pfile = ::_wfopen(szPathFullEx, L"wt,ccs=UTF-8");
	}

	if (!pfile)
	{
		hrRet = S_OK;
		goto _Exit;
	}


	for (itor = vLogCacheNew.begin(); itor != vLogCacheNew.end(); itor++)
	{
		wstring szTargetEx; 
		szTargetEx = (*itor);
		DWORD dwSize = (DWORD)::fwrite(szTargetEx.c_str(), sizeof(WCHAR), ::wcslen(szTargetEx.c_str()), pfile);
		::fwrite(L"\n", sizeof(WCHAR), ::wcslen(L"\n"), pfile);
	}


_Exit:

	if (pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	if (pFile)
	{
		fclose(pFile);
		pFile = NULL;
	}
	if (pfile)
	{
		fclose(pfile);
		pfile = NULL;
	}
	return TRUE;
}
BOOL _Set360BigDir(const CString& strDir)
{
	BOOL bRet = FALSE;

	WCHAR szPath[MAX_PATH] = {0};
	CString szPathFull;
	bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
	szPathFull = szPath;
	szPathFull += _T("\\360se\\extensions\\ExtDownload\\ExtDownload.ini");

	WCHAR szRecord[MAX_PATH] = {0};

	BOOL bErr = WritePrivateProfileString(L"Download", L"UserDefinePath", strDir, szPathFull);

	if ( !bErr )
	{
		bRet = FALSE;
		goto _exit_;
	}

	bRet = TRUE;

_exit_:
	return bRet;
}
BOOL _SetMaxmonthBigDir(const CString& strDir)
{
	KSearchSoftwareStruct sss;
	wstring strPath;
	BOOL bRet = FALSE;
	CString szPathFull;
	WCHAR szPath[MAX_PATH] = {0};
	sss.pszMainFileName      = TEXT( "Maxthon.exe" );
	sss.hRegRootKey = HKEY_CURRENT_USER;
	sss.pszRegSubKey = _T("SOFTWARE\\Maxthon2");
	sss.pszPathValue = _T("Folder");

	bRet = SearchSoftwarePath( &sss, strPath );
	wcscpy(szPath, strPath.c_str());
	::PathRemoveFileSpec(szPath);
	szPathFull = szPath;

	szPathFull += _T("\\SharedAccount\\Config\\Downloader.ini");

	WCHAR szRecord[MAX_PATH] = {0};

	for (int i = 0; i < 10;i++)
	{
		CString strTemp;
		strTemp.Format(_T("%d"), i);
		WritePrivateProfileString(L"COMBOX_HIS", strTemp, NULL, szPathFull);
	}
	WritePrivateProfileString(L"COMBOX_HIS", L"0", strDir, szPathFull);

	return TRUE;
}
BOOL _SetTheworldBigDir(const CString& strDir)
{
	KSearchSoftwareStruct sss;
	wstring strPath;
	BOOL bRet = FALSE;
	CString szPathFull;
	WCHAR szPath[MAX_PATH] = {0};
	sss.pszMainFileName      = TEXT( "TheWorld.exe" );
	sss.hRegRootKey = HKEY_LOCAL_MACHINE;
	sss.pszRegSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\TheWorld.exe");
	sss.pszPathValue = _T("Path");
	sss.pszUninstallListName = _T("世界之窗浏览器");

	bRet = SearchSoftwarePath( &sss, strPath );
	wcscpy(szPath, strPath.c_str());
	::PathRemoveFileSpec(szPath);
	szPathFull = szPath;

	szPathFull += _T("\\TheWorld.ini");

	WCHAR szRecord[MAX_PATH] = {0};

	for (int i = 0; i < 10;i++)
	{
		CString strTemp;
		strTemp.Format(_T("Path%d"), i);
		WritePrivateProfileString(L"Download", strTemp, NULL, szPathFull);
	}

	bRet = WritePrivateProfileString(L"Download", L"Path0", strDir, szPathFull);

	return bRet;
}


//////////////////////////////////////////////////////////////////////////
