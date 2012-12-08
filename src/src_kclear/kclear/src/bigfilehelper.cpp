#include "stdafx.h"
#include "bigfilehelper.h"
#include "kscbase/kscsys.h"
#include "getinstallpath.h"
#include "misc/kregfunction.h"
#include "misc/IniEditor.h"
#include <strsafe.h>
#include "linkchecker.h"

#define  BAIDU_URL L"http://www.baidu.com/s?wd=%s"

//////////////////////////////////////////////////////////////////////////

void bigfilehelper::ShowFileProperties(LPCTSTR lpFilePath)
{
    SHELLEXECUTEINFO shellInfo = { 0 };
    shellInfo.cbSize = sizeof(shellInfo);
    shellInfo.lpVerb = _T("properties");
    shellInfo.lpFile = lpFilePath;
    shellInfo.fMask = SEE_MASK_INVOKEIDLIST;
    ShellExecuteEx(&shellInfo);
}

BOOL bigfilehelper::BaiduFileinfo(LPCTSTR pszPath)
{
    SHELLEXECUTEINFO shExecInfo;
    CString strurl;
    CString strPath = pszPath;
    CString strFileName = strPath.Mid(strPath.ReverseFind(L'\\') + 1);
    strurl.Format(BAIDU_URL, strFileName);

    shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shExecInfo.fMask = NULL;
    shExecInfo.hwnd = NULL;
    shExecInfo.lpVerb = L"open";
    shExecInfo.lpFile = strurl;
    shExecInfo.lpParameters = NULL;
    shExecInfo.lpDirectory = NULL;
    shExecInfo.nShow = SW_NORMAL;
    shExecInfo.hInstApp = NULL;

    return ShellExecuteEx(&shExecInfo);
}

BOOL bigfilehelper::LocateFile(const CString& strFilePath)
{
    BOOL retval = FALSE;
    CString strShellCmd;
    TCHAR* szCmdLine = NULL;
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    BOOL fRetCode;

    si.cb = sizeof(si);

    if (wcschr(strFilePath, _T(' ')))
    {
        strShellCmd = _T("explorer.exe /e,/select,\"");
        strShellCmd += strFilePath;
        strShellCmd += _T("\""); 
    }
    else
    {
        strShellCmd = _T("explorer.exe /e,/select,");
        strShellCmd += strFilePath;
    }

    szCmdLine = _tcsdup(strShellCmd);

    fRetCode = CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (!fRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (pi.hThread)
    {
        CloseHandle(pi.hThread);
        pi.hThread = NULL;
    }

    if (pi.hProcess)
    {
        CloseHandle(pi.hProcess);
        pi.hProcess = NULL;
    }

    return retval;
}

BOOL bigfilehelper::Recycle(LPCTSTR pszPath, BOOL bDelete)
{
    BOOL retval = FALSE;
    SHFILEOPSTRUCT  shDelFile;
    int nRetCode;
    DWORD dwLastError;
    TCHAR szDelPath[MAX_PATH] = { 0 };
    HRESULT hRetCode;

    memset(&shDelFile, 0, sizeof(shDelFile));
    shDelFile.fFlags = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;
    if (!bDelete)
        shDelFile.fFlags |= FOF_ALLOWUNDO;

    if (!pszPath)
        goto clean0;

    hRetCode = StringCchCopy(szDelPath, MAX_PATH, pszPath);
    if (FAILED(hRetCode))
        goto clean0;

    shDelFile.wFunc = FO_DELETE;
    shDelFile.pFrom = szDelPath;
    shDelFile.pTo = NULL;

    ::SetLastError(0);
    nRetCode = SHFileOperation(&shDelFile);
    dwLastError = ::GetLastError();
    if (nRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

CHAR bigfilehelper::GetSystemDrive()
{
    CHAR szSysPath[MAX_PATH] = { 0 };

    ::GetSystemDirectoryA(szSysPath, MAX_PATH);
    return szSysPath[0];
}

BOOL bigfilehelper::GetSystemDrive(CString& strDrv)
{
    BOOL retval = FALSE;
    TCHAR szSysDir[MAX_PATH] = { 0 };

    if (!GetSystemDirectory(szSysDir, MAX_PATH))
        goto clean0;

    strDrv = CString(szSysDir).Left(3);

    retval = TRUE;

clean0:
    return retval;
}

BOOL bigfilehelper::GetAutoDestDirFromSize(CString& strDir)
{
    BOOL retval = FALSE;
    CAtlArray<TCHAR> buffer;
    TCHAR* pBuffer = NULL;
    DWORD dwSize;
    CAtlList<CString> logicalDrvs;
    CString strDrv;
    POSITION pos = NULL;
    POSITION max_size_pos = NULL;
    ULONGLONG uMaxSize = 0;
    DWORD dwSectorsPerCluster;
    DWORD dwBytesPerSector;
    DWORD dwNumberOfFreeClusters;
    DWORD dwTotalNumberOfClusters;
    CString strSysDrv;
    TCHAR szVolName[MAX_PATH+1] = { 0 };
    TCHAR szFileSystem[MAX_PATH+1] = { 0 };
    BOOL fRetCode;

    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    if (!GetSystemDrive(strSysDrv))
        goto clean0;

    buffer.SetCount(512);
    pBuffer = buffer.GetData();
    dwSize = (DWORD)buffer.GetCount();
    memset(pBuffer, 0, dwSize * sizeof(TCHAR));
    dwSize = GetLogicalDriveStrings(dwSize, buffer.GetData());

    if (dwSize > 2) 
    {
        strDrv = pBuffer;
        logicalDrvs.AddTail(strDrv);

        for (DWORD i = 3; i < dwSize; ++i) 
        {
            if (pBuffer[i] != 0 && pBuffer[i - 1] == 0) 
            {
                strDrv = pBuffer + i;
                logicalDrvs.AddTail(strDrv);
            }
        }
    }

    pos = logicalDrvs.GetHeadPosition();
    while (pos)
    {
        POSITION current = pos;
        CString _drv = logicalDrvs.GetNext(pos);
        _drv.MakeLower();
        if (_drv == _T("a:\\") || _drv == _T("b:\\"))
        {
            logicalDrvs.RemoveAt(current);
            continue;
        }

        UINT uType = GetDriveType(_drv);
        if (uType != DRIVE_FIXED &&
            uType != DRIVE_REMOVABLE)
        {
            logicalDrvs.RemoveAt(current);
            continue;
        }

        if (strSysDrv.CompareNoCase(_drv)==0)
        {
            logicalDrvs.RemoveAt(current);
            continue;
        }

        RtlZeroMemory(szVolName, sizeof(szVolName));
        RtlZeroMemory(szFileSystem, sizeof(szFileSystem));
        fRetCode = GetVolumeInformation(
            _drv,
            szVolName,
            MAX_PATH+1,
            NULL,
            NULL,
            NULL,
            szFileSystem,
            MAX_PATH+1
            );
        if (!fRetCode)
        {
            logicalDrvs.RemoveAt(current);
            continue;
        }
    }

    pos = logicalDrvs.GetHeadPosition();
    while (pos)
    {
        POSITION current = pos;
        const CString& _drv = logicalDrvs.GetNext(pos);
        BOOL fRetCode = GetDiskFreeSpace(
            _drv,
            &dwSectorsPerCluster,
            &dwBytesPerSector,
            &dwNumberOfFreeClusters,
            &dwTotalNumberOfClusters
            );
        if (!fRetCode)
            continue;

        ULONGLONG uCurrentFreeSize = (ULONGLONG)dwNumberOfFreeClusters * dwSectorsPerCluster * dwBytesPerSector;
        if (uCurrentFreeSize > uMaxSize)
        {
            max_size_pos = current;
            uMaxSize = uCurrentFreeSize;
        }
    }
    if (max_size_pos==NULL)
        goto clean0;
    strDir = logicalDrvs.GetAt(max_size_pos);
    strDir += _T("系统盘大文件");

    retval = TRUE;

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

void bigfilehelper::GetShortExtString(CString& strExt)
{
    strExt.TrimLeft(_T('.'));

    if (strExt.GetLength() > 6)
        strExt = strExt.Left(6) + _T("..");
}

void bigfilehelper::GetFileSizeString(ULONGLONG qwFileSize, CString& strFileSize)
{
    int nFlag;
    CString strEnd;

    if (qwFileSize < 1000)
    {
        strEnd = _T("B");
        strFileSize.Format(_T("%d"), qwFileSize);
    }
    else if (qwFileSize < 1000 * 1024)
    {
        strEnd = _T("KB");
        strFileSize.Format(_T("%0.3f"), (float)qwFileSize / 1024);
    }
    else if (qwFileSize < 1000 * 1024 * 1024)
    {
        strEnd = _T("MB");
        strFileSize.Format(_T("%0.3f"), (float)qwFileSize / (1024 * 1024));
    }
    else
    {
        strEnd = _T("GB");
        strFileSize.Format(_T("%0.3f"), (float)qwFileSize / (1024 * 1024 * 1024));
    }

    nFlag = strFileSize.Find(_T('.'));
    if (nFlag != -1)
    {
        if (nFlag >= 3)
        {
            strFileSize = strFileSize.Left(nFlag);
        }
        else
        {
            strFileSize = strFileSize.Left(4);
        }
    }

    strFileSize += strEnd;
}

//////////////////////////////////////////////////////////////////////////

BOOL bigfilehelper::EnumFiles(const CString& strPath, IEnumFileback* piEnumFileback)
{
    BOOL retval = FALSE;
    CString strFind = strPath + _T("*.*");
    HANDLE hFind = NULL;
    WIN32_FIND_DATA findData;
    BOOL bRetCode;
    BOOL bContinue;
    CLinkChecker& linkChecker = CLinkChecker::Instance();

    hFind = ::FindFirstFile(strFind, &findData); 
    if (INVALID_HANDLE_VALUE == hFind)
    {
        retval = TRUE;
        goto clean0;
    }

    bRetCode = TRUE;
    while (bRetCode)
    {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  
        {
            if (_tcscmp(findData.cFileName, _T(".")) &&
                _tcscmp(findData.cFileName, _T("..")))
            {
                CString strSubDir = strPath + findData.cFileName;

                if (!linkChecker.IsDirectoryJunction(strSubDir))
                {
                    strSubDir += _T("\\");
                    if (!EnumFiles(strSubDir, piEnumFileback))
                        goto clean0;
                }
            }
        }
        else
        {
            BigFileInfo fileinfo;

            fileinfo.strFilePath = strPath + findData.cFileName;
            fileinfo.qwFileSize = ((ULONGLONG)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;
            
            if (!linkChecker.IsFileSymlinkd(fileinfo.strFilePath))
            {
                bContinue = piEnumFileback->OnFindFile(fileinfo);
                if (!bContinue)
                    goto clean0;
            }
        }

        bRetCode = ::FindNextFile(hFind, &findData);
    }

    retval = TRUE;

clean0:
    if (hFind)
    {
        ::FindClose(hFind);
        hFind = NULL;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

void bigfilehelper::GetFileExt(const CString& strPath, CString& strExt)
{
    CString strFilePath = strPath;
    int nExt;

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
}

//////////////////////////////////////////////////////////////////////////

BOOL bigfilehelper::GetDesktopPath(CString& strPath)
{
    BOOL retval;
    TCHAR szFilePath[MAX_PATH] = { 0 };

    retval = SHGetSpecialFolderPath(NULL, szFilePath, CSIDL_DESKTOP, FALSE);
    if (retval)
    {
        PathAddBackslash(szFilePath);
        strPath = szFilePath;
    }

    return retval;
}

BOOL bigfilehelper::GetDocumentPath(CString& strPath)
{
    BOOL retval;
    TCHAR szFilePath[MAX_PATH] = { 0 };

    retval = SHGetSpecialFolderPath(NULL, szFilePath, CSIDL_MYDOCUMENTS, FALSE);
    if (retval)
    {
        PathAddBackslash(szFilePath);
        strPath = szFilePath;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

static DWORD _DoGetFileSizeByFileName(const WCHAR* pFileName)
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

static DWORD _DoGetLineByBuf(
    WCHAR* const pszBuf,
    DWORD dwlen,
    std::vector<std::wstring>& vcStrline
    )
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

//////////////////////////////////////////////////////////////////////////

BOOL bigfilehelper::GetChromeDownloadDirs(std::vector<CString>& vDirs)
{
    WCHAR szPath[MAX_PATH] = {0};
    BOOL bRet = FALSE;
    CString szPathFull;
    CString szPathFullEx;
    CString szTarget;
    int nPos = -1;
    CString strPath = L"";
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    
    bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
    if(bRet)
    {
        wcscat(szAppDataPath,L"\\Downloads");
        if(szAppDataPath[0]==GetSystemDrive())
            vDirs.push_back(szAppDataPath);
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
        vDirs.push_back(strPath);
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

// 获得Firefox的下载目录
BOOL bigfilehelper::GetFirefoxDownloadDirs(std::vector<CString>& vDirs)
{
    WCHAR szPath[MAX_PATH] = {0};
    BOOL bRet = FALSE;
    CString szPathFull;
    CString szPathFullEx;
    CString szTarget;
    CString strPath = L"";
    int nPos = -1;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    WinVersion winVer;

    winVer = KGetWinVersion();

    if(WINVERSION_WIN7 == winVer)
    {
        bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
        if(bRet)
        {
            CString tmpdir(szAppDataPath);
            tmpdir = tmpdir.Left(tmpdir.ReverseFind(L'\\'));
            tmpdir += L"\\downloads";
            if(tmpdir.GetAt(0)==GetSystemDrive())
                vDirs.push_back(tmpdir);
        }

    }
    else
    {
        bRet = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_PERSONAL, FALSE);
        if(bRet)
        {
            wcscat(szAppDataPath,L"\\下载");
            if(szAppDataPath[0]==GetSystemDrive())
                vDirs.push_back(szAppDataPath);
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
        vDirs.push_back(strPath);
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

// 获得360浏览器的下载目录
BOOL bigfilehelper::Get360DownloadDirs(std::vector<CString>& vDirs)
{
    WCHAR szPath[MAX_PATH] = {0};
    BOOL bRet = FALSE;
    CString szPathFull;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };

    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\Downloads");
    vDirs.push_back(szAppDataPath);

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
        vDirs.push_back(szFull);
    else 
        return FALSE;
    return TRUE;
}

// 获得世界之窗的下载目录
BOOL bigfilehelper::GetTheworldDownloadDirs(std::vector<CString>& vDirs)
{
    KSearchSoftwareStruct sss;
    wstring strPath;
    BOOL bRet = FALSE;
    CString szPathFull;
    WCHAR szPath[MAX_PATH] = {0};

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\Downloads");
    vDirs.push_back(szAppDataPath);

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

    for (int i = 0; i < 8; i++)
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
            vDirs.push_back(szFull);
    }
    return TRUE;
}

// 获得马桶的下载目录
BOOL bigfilehelper::GetMaxmonthDownloadDirs(std::vector<CString>& vDirs)
{
    KSearchSoftwareStruct sss;
    wstring strPath;
    BOOL bRet = FALSE;
    CString szPathFull;
    WCHAR szPath[MAX_PATH] = {0};
    WCHAR szAppDataPath[MAX_PATH] = {0};
    
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\MxDownload");
    vDirs.push_back(szAppDataPath);

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

    for (int i = 0; i < 8; i++)
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
            vDirs.push_back(szFull);

    }
    return TRUE;
}

// 获得FlashGet的下载目录
BOOL bigfilehelper::GetFlashGetDownloadDirs(std::vector<CString>& vDirs)
{
    WinVersion  winver = KGetWinVersion();
    IniEditor IniEdit;
    BOOL bRet = TRUE;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };

    WCHAR szAppPath[MAX_PATH] = {0};
    szAppPath[0] = GetSystemDrive();
    wcscat(szAppPath,L":\\Downloads");
    vDirs.push_back(szAppPath);

    if(WINVERSION_WIN7 == winver)
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
                vDirs.push_back(UserPath.c_str());
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
                    vDirs.push_back(UserPath.c_str());
                else bRet = FALSE;
            }

        }
    }
clean0:
    return bRet;
}

// 获得迅雷的下载目录
BOOL bigfilehelper::GetXunLeiDownloadDirs(std::vector<CString>& vDirs)
{
    BOOL bRet = TRUE;
    TCHAR szLongPathBuffer[MAX_PATH] = { 0 };
    DWORD len = sizeof(szLongPathBuffer);

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\TDDownload");
    vDirs.push_back(szAppDataPath);

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
            vDirs.push_back(szLongPathBuffer);
        else bRet = FALSE;

    }
    return bRet;
}

// 获得比特彗星的下载目录
BOOL bigfilehelper::GetBitCometDownloadDirs(std::vector<CString>& vDirs)
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

    WCHAR szAppDataPath[MAX_PATH] = {0};
    szAppDataPath[0] = GetSystemDrive();
    wcscat(szAppDataPath,L":\\downloads");
    vDirs.push_back(szAppDataPath);

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
            vDirs.push_back(strDirs);
        else goto clean0;
    }
    retval = TRUE;

clean0:
    return retval;
}

static BOOL FindUserConfigInDir(LPCTSTR pszFullPath,CString &filePathName)
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
            if (_tcscmp(ff.cFileName, _T( "."  )) && 
                _tcscmp(ff.cFileName, _T( ".." )))
            {
                memset(szFindName, 0, sizeof(szFindName));
                _tcsncpy_s( szFindName, MAX_PATH, pszFullPath,
                    MAX_PATH - 1 );
                _tcsncat_s( szFindName, MAX_PATH, ff.cFileName, 
                    MAX_PATH - _tcslen(szFindName) - 1 );
                _tcsncat_s( szFindName, MAX_PATH,TEXT("\\"),
                    MAX_PATH - _tcslen(szFindName) - 1 );

                FindUserConfigInDir( szFindName,filePathName);  
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

// 获得QQ旋风的下载目录
BOOL bigfilehelper::GetQQDownLoadDownloadDirs(std::vector<CString>& vDirs)
{
    BOOL retval = FALSE;
    BOOL fRetCode;
    WCHAR szAppDataPath[MAX_PATH] = { 0 };
    WCHAR szCachePath[MAX_PATH] = { 0 };
    CString szFilePath = L"";
    DWORD dwRetCode;

    WCHAR szAppPath[MAX_PATH] = {0};
    szAppPath[0] = GetSystemDrive();
    wcscat(szAppPath,L":\\qqdownload");
    vDirs.push_back(szAppPath);

    fRetCode = SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_APPDATA, FALSE);
    if (!fRetCode)
        goto clean0;

    PathAppend(szAppDataPath, _T("QQDownload\\"));
    if (GetFileAttributes(szAppDataPath) == INVALID_FILE_ATTRIBUTES)
        goto clean0;

    fRetCode = FindUserConfigInDir(szAppDataPath, szFilePath);
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
        vDirs.push_back(szCachePath);
    else goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL bigfilehelper::GetAllDownloadDirs(std::vector<CString>& vDirs)
{
    BOOL retval = FALSE;
    std::vector<CString> vEnumPaths;
    size_t i, j;

    vDirs.clear();

    vEnumPaths.clear();
    bigfilehelper::Get360DownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::GetChromeDownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::GetFirefoxDownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::Get360DownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::GetTheworldDownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::GetMaxmonthDownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::GetFlashGetDownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::GetXunLeiDownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::GetBitCometDownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    vEnumPaths.clear();
    bigfilehelper::GetQQDownLoadDownloadDirs(vEnumPaths);
    for (i = 0; i < vEnumPaths.size(); ++i)
    {
        BOOL bExist = FALSE;
        CString strPath = vEnumPaths[i];
        if (strPath[strPath.GetLength() - 1] != _T('\\'))
            strPath += _T("\\");
        for (j = 0; j < vDirs.size(); ++j)
        {
            if (vDirs[j].CompareNoCase(strPath) == 0)
            {
                bExist = TRUE;
                break;
            }
        }
        if (!bExist)
        {
            vDirs.push_back(strPath);
        }
    }

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL bigfilehelper::GetAllVols(std::vector<VolInfo>& vVols)
{
    BOOL retval = FALSE;
    CAtlArray<TCHAR> buffer;
    TCHAR* pBuffer = NULL;
    DWORD dwSize;
    std::vector<CString> logicalDrvs;
    CString strDrv;
    POSITION pos = NULL;
    POSITION max_size_pos = NULL;
    ULONGLONG uMaxSize = 0;
    DWORD dwSectorsPerCluster;
    DWORD dwBytesPerSector;
    DWORD dwNumberOfFreeClusters;
    DWORD dwTotalNumberOfClusters;
    TCHAR szVolName[MAX_PATH+1] = { 0 };
    TCHAR szFileSystem[MAX_PATH+1] = { 0 };
    BOOL fRetCode;
    VolInfo volInfo;
    size_t idx;

    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    buffer.SetCount(512);
    pBuffer = buffer.GetData();
    dwSize = (DWORD)buffer.GetCount();
    memset(pBuffer, 0, dwSize * sizeof(TCHAR));
    dwSize = GetLogicalDriveStrings(dwSize, buffer.GetData());

    if (dwSize > 2)
    {
        strDrv = pBuffer;
        logicalDrvs.push_back(strDrv);

        for (DWORD i = 3; i < dwSize; ++i) 
        {
            if (pBuffer[i] != 0 && pBuffer[i - 1] == 0) 
            {
                strDrv = pBuffer + i;
                logicalDrvs.push_back(strDrv);
            }
        }
    }

    for (idx = 0; idx < logicalDrvs.size(); ++idx)
    {
        CString _drv = logicalDrvs[idx];
        BOOL bRemovable = FALSE;

        if (_drv.CompareNoCase(_T("a:\\")) == 0 || 
            _drv.CompareNoCase(_T("b:\\")) == 0)
        {
            continue;
        }

        UINT uType = GetDriveType(_drv);
        if (uType != DRIVE_FIXED &&
            uType != DRIVE_REMOVABLE)
        {
            continue;
        }

        if (DRIVE_REMOVABLE == uType)
            bRemovable = TRUE;

        RtlZeroMemory(szVolName, sizeof(szVolName));
        RtlZeroMemory(szFileSystem, sizeof(szFileSystem));
        fRetCode = GetVolumeInformation(
            _drv,
            szVolName,
            MAX_PATH+1,
            NULL,
            NULL,
            NULL,
            szFileSystem,
            MAX_PATH+1
            );
        if (!fRetCode)
        {
            continue;
        }

        fRetCode = GetDiskFreeSpace(
            _drv,
            &dwSectorsPerCluster,
            &dwBytesPerSector,
            &dwNumberOfFreeClusters,
            &dwTotalNumberOfClusters
            );
        if (!fRetCode)
        {
            continue;
        }

        volInfo.cVol = (char)_drv[0];
        volInfo.strVolName = szVolName;
        if (volInfo.strVolName.IsEmpty())
            volInfo.strVolName = _T("本地磁盘");
        volInfo.qwTotalSize = (ULONGLONG)dwTotalNumberOfClusters * dwBytesPerSector * dwSectorsPerCluster;
        volInfo.qwFreeSize = (ULONGLONG)dwNumberOfFreeClusters * dwBytesPerSector * dwSectorsPerCluster;
        volInfo.bRemovable = bRemovable;
        vVols.push_back(volInfo);
    }

    retval = TRUE;

//clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

void bigfilehelper::CompressedPath(CString& strFilePath)
{
    if (strFilePath.GetLength() > 60)
    {
        CString strNewPath = strFilePath;
        strNewPath = strFilePath.Left(20);
        strNewPath +=L"...";
        strNewPath += strFilePath.Mid(strFilePath.GetLength() - 20, 20);
        strFilePath = strNewPath;
    }
}

//////////////////////////////////////////////////////////////////////////

HICON bigfilehelper::GetIconFromFilePath(const CString& strFilePath)
{
    SHFILEINFO fileInfo = { 0 };

    ::SHGetFileInfo(strFilePath, 0, &fileInfo, sizeof(fileInfo), SHGFI_ICON | SHGFI_SMALLICON);

    return fileInfo.hIcon;
}