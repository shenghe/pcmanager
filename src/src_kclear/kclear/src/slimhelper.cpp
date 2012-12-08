//////////////////////////////////////////////////////////////////////////
// Creator: Leo Jiang<JiangFengbing@gmail.com>
// Purpose: 删除文件,压缩文件
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "slimhelper.h"
#include "slimcallback.h"
#include "kscbase/kscsys.h"
//#include "misc/ntdll_lite.h"
#include <sddl.h>
#include <strsafe.h>
#include <atlsecurity.h>
#include <sfc.h>

//////////////////////////////////////////////////////////////////////////

const TCHAR slimhelper::g_kSlimFlag[] = _T("slimflag.txt");

//////////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////////

BOOL GrantFileAccess(const CString& strFilePath, 
                     CString& strOldSecurityDescriptor,
                     BOOL bDir = FALSE)
{
    BOOL retval = FALSE;
    PSECURITY_DESCRIPTOR pNewSecDescriptor = NULL;
    BOOL bRetCode;
    ATL::CSecurityDesc secDesc;
    CString strSecDesc;

    if (!AtlGetSecurityDescriptor(strFilePath, SE_FILE_OBJECT, &secDesc))
        goto clean0;

    secDesc.ToString(&strSecDesc);

    bRetCode = ConvertStringSecurityDescriptorToSecurityDescriptor(
        bDir ? _T("O:BAG:BAD:PAI(A;OICI;FA;;;BU)") : _T("O:BAG:BAD:(A;;FA;;;BU)"),
        SDDL_REVISION_1,
        &pNewSecDescriptor,
        NULL
        );
    if (!bRetCode)
        goto clean0;

    bRetCode = SetFileSecurity(
        strFilePath,
        OWNER_SECURITY_INFORMATION,
        pNewSecDescriptor
        );
    if (!bRetCode)
        goto clean0;

    bRetCode = SetFileSecurity(
        strFilePath,
        OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION,
        pNewSecDescriptor
        );
    if (!bRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (pNewSecDescriptor)
    {
        ::LocalFree(pNewSecDescriptor);
        pNewSecDescriptor = NULL;
    }

    return retval;
}

BOOL RestoreFileAccess(const CString& strFilePath, 
                       CString& strOldSecurityDescriptor)
{
    BOOL retval = FALSE;
    ATL::CSecurityDesc secDesc;
    ATL::CSid owner;
    ATL::CSid group;
    ATL::CDacl dacl;

    secDesc.FromString(strOldSecurityDescriptor);

    secDesc.GetOwner(&owner);
    secDesc.GetGroup(&group);
    secDesc.GetDacl(&dacl);
    
    if (!ATL::AtlSetOwnerSid(strFilePath, SE_FILE_OBJECT, owner))
        goto clean0;

    if (!ATL::AtlSetGroupSid(strFilePath, SE_FILE_OBJECT, group))
        goto clean0;

    if (!ATL::AtlSetDacl(strFilePath, SE_FILE_OBJECT, dacl))
        goto clean0;

    retval = TRUE;

clean0:
    return retval;
}

BOOL GrantDirAccess(const CString& strDirectory)
{
    BOOL retval = FALSE;
    CString strPath;
    CString strFind;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA findData;
    BOOL bRetCode;
    CString strOldSecDesc;

    if (strDirectory.IsEmpty())
        goto clean0;

    strPath = strDirectory;
    if (strPath[strPath.GetLength() - 1] != _T('\\'))
        strPath += _T("\\");

    strFind = strPath + _T("*.*");

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
                strSubDir += _T("\\");
                if (!GrantDirAccess(strSubDir))
                    goto clean0;
            }
        }
        else
        {
            CString strFullFilePath = strPath + findData.cFileName;
            ULONGLONG qwFileSize = ((ULONGLONG)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;

            GrantFileAccess(strFullFilePath, strOldSecDesc);
        }

        bRetCode = ::FindNextFile(hFind, &findData);
    }

    retval = TRUE;

clean0:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        ::FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    GrantFileAccess(strDirectory, strOldSecDesc, TRUE);

    return retval;
}

class FileSizeConver
{
public:
    static FileSizeConver& Instance()
    {
        static FileSizeConver singleton;
        return singleton;
    }

    ULONGLONG FileSizeOnDisk(ULONGLONG dwFileSize)
    {
        return ((dwFileSize + m_dwClusterSize - 1) / m_dwClusterSize) * m_dwClusterSize;
    }

private:
    FileSizeConver() 
    {
        TCHAR szSys[MAX_PATH] = { 0 };
        DWORD dwSectorsPerCluster;
        DWORD dwBytesPerSector;

        GetWindowsDirectory(szSys, MAX_PATH);
        szSys[3] = _T('\0');
        GetDiskFreeSpace(
            szSys, 
            &dwSectorsPerCluster, 
            &dwBytesPerSector,
            NULL,
            NULL
            );
        m_dwClusterSize = dwSectorsPerCluster * dwBytesPerSector;
    }

    ~FileSizeConver() 
    {
    }

    DWORD m_dwClusterSize;
};

//////////////////////////////////////////////////////////////////////////

} // namespce

//////////////////////////////////////////////////////////////////////////

BOOL slimhelper::IsCompressed(const CString& strFilePath, BOOL& bCompressed)
{
    BOOL retval = FALSE;
    DWORD dwAttributes;

    dwAttributes = ::GetFileAttributes(strFilePath);
    if (INVALID_FILE_ATTRIBUTES == dwAttributes)
        goto clean0;

    if (dwAttributes & FILE_ATTRIBUTE_COMPRESSED
        || dwAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
    {
        bCompressed = TRUE;
    }
    else
    {
        bCompressed = FALSE;
    }

    retval = TRUE;

clean0:
    return retval;
}

BOOL slimhelper::CompressFile(const CString& strFilePath,
                              ULONGLONG qwFileSize,
                              DWORD dwFileAttributes,
                              ISystemSlimCallBack* piCallback)
{
    BOOL retval = FALSE;
    HANDLE hFileHandle = INVALID_HANDLE_VALUE;
    BOOL bRetCode;
    USHORT uCompress = COMPRESSION_FORMAT_DEFAULT;
    DWORD dwReturn;
    ULARGE_INTEGER tempSize;
    HRESULT hr;
    BOOL bCompressed = FALSE;
    ULONGLONG qwOldFileSize = qwFileSize;
    CString strOldSecurityDescriptor;
    BOOL bIsDir = FALSE;

    if (!piCallback)
        goto clean0;

    piCallback->OnBeginProcessItem(strFilePath);

    if (INVALID_FILE_ATTRIBUTES == dwFileAttributes)
    {
        dwFileAttributes = ::GetFileAttributes(strFilePath);
        if (INVALID_FILE_ATTRIBUTES == dwFileAttributes)
            goto clean0;
    }

    if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        bIsDir = TRUE;

    if (dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED
        || dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
    {
        bCompressed = TRUE;
    }

    if (bCompressed)
        goto clean0;

    if (0 == qwOldFileSize)
    {
        CAtlFile file;
        hr = file.Create(strFilePath, 
            FILE_GENERIC_READ, 
            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
            OPEN_EXISTING);
        if (FAILED(hr))
            goto clean0;

        tempSize.LowPart = GetFileSize((HANDLE)file, &tempSize.HighPart);
        qwOldFileSize = tempSize.QuadPart;
    }

    qwOldFileSize = FileSizeConver::Instance().FileSizeOnDisk(qwOldFileSize);

    if (!GrantFileAccess(strFilePath, strOldSecurityDescriptor, bIsDir))
        goto clean0;

    hFileHandle = ::CreateFile(strFilePath,
                               FILE_GENERIC_READ|FILE_GENERIC_WRITE,
                               FILE_SHARE_READ|FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if (INVALID_HANDLE_VALUE == hFileHandle)
        goto clean0;

    bRetCode = ::DeviceIoControl(hFileHandle,
                                 FSCTL_SET_COMPRESSION,
                                 &uCompress,
                                 sizeof(uCompress),
                                 NULL,
                                 0,
                                 &dwReturn,
                                 NULL);
    if (!bRetCode)
        goto clean0;

    ::CloseHandle(hFileHandle);
    hFileHandle = INVALID_HANDLE_VALUE;

    tempSize.LowPart = ::GetCompressedFileSize(strFilePath, &tempSize.HighPart);
    qwFileSize = tempSize.QuadPart;
    qwFileSize = FileSizeConver::Instance().FileSizeOnDisk(qwFileSize);

    piCallback->OnEndProcessItem(strFilePath, qwOldFileSize - qwFileSize);

    retval = TRUE;

clean0:
    if (hFileHandle != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(hFileHandle);
        hFileHandle = INVALID_HANDLE_VALUE;
    }

    if (strOldSecurityDescriptor.GetLength())
    {
        RestoreFileAccess(strFilePath, strOldSecurityDescriptor);
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

BOOL slimhelper::CompressDirectory(const CString& strDirectory, 
                                   ISystemSlimCallBack* piCallback,
                                   int nLevel)
{
    BOOL retval = FALSE;
    CString strPath;
    CString strFind;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA findData;
    BOOL bRetCode;
    BOOL bContinue;

    if (strDirectory.IsEmpty() || !piCallback)
        goto clean0;

    strPath = strDirectory;
    if (strPath[strPath.GetLength() - 1] != _T('\\'))
        strPath += _T("\\");

    strFind = strPath + _T("*.*");

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
                strSubDir += _T("\\");
                if (!CompressDirectory(strSubDir, piCallback, nLevel + 1))
                        goto clean0;
            }
        }
        else
        {
            CString strFullFilePath = strPath + findData.cFileName;
            ULONGLONG qwFileSize = ((ULONGLONG)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;
            
            CompressFile(strFullFilePath, qwFileSize, findData.dwFileAttributes, piCallback);

            bContinue = piCallback->OnIdle();
            if (!bContinue)
                goto clean0;
        }

        bRetCode = ::FindNextFile(hFind, &findData);
    }

    retval = TRUE;

clean0:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        ::FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    if (0 == nLevel)
    {
        CompressFile(strDirectory, 4, INVALID_FILE_ATTRIBUTES, piCallback);
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////
// 删除单个文件
BOOL slimhelper::DeleteFile(const CString& strFilePath,
                            ULONGLONG qwFileSize,
                            DWORD dwFileAttributes,
                            ISystemSlimCallBack* piCallback)
{
    BOOL retval = FALSE;
    BOOL bRetCode;
    CString strOldSecurityDescriptor;
    HRESULT hr;
    ULARGE_INTEGER tempSize;

    if (!piCallback)
        goto clean0;

    if (!piCallback->OnBeginProcessItem(strFilePath))
        goto clean0;

    if (INVALID_FILE_ATTRIBUTES == dwFileAttributes)
    {
        dwFileAttributes = ::GetFileAttributes(strFilePath);
        if (INVALID_FILE_ATTRIBUTES == dwFileAttributes)
            goto clean0;
    }

    if (dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED
        || dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
    {
        tempSize.LowPart = GetCompressedFileSize(strFilePath, &tempSize.HighPart);
        qwFileSize = tempSize.QuadPart;
    }
    else
    {
        if (0 == qwFileSize)
        {
            CAtlFile file;
            hr = file.Create(strFilePath, 
                FILE_GENERIC_READ, 
                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                OPEN_EXISTING);
            if (FAILED(hr))
                goto clean0;

            tempSize.LowPart = GetFileSize((HANDLE)file, &tempSize.HighPart);
            qwFileSize = tempSize.QuadPart;
        }
    }

    qwFileSize = FileSizeConver::Instance().FileSizeOnDisk(qwFileSize);

    if (!GrantFileAccess(strFilePath, strOldSecurityDescriptor))
        goto clean0;

    bRetCode = ::DeleteFile(strFilePath);
    if (!bRetCode)
    {
        if (!::MoveFileEx(strFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
        {
            goto clean0;
        }
    }

    piCallback->OnEndProcessItem(strFilePath, qwFileSize);

    retval = TRUE;

clean0:
    return retval;
}

BOOL slimhelper::DeleteDirectory(const CString& strDirectory,
                                 ISystemSlimCallBack* piCallback,
                                 BOOL bKeepRootDir,
                                 int nLevel) 
{
    BOOL retval = FALSE;
    CString strPath;
    CString strFind;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA findData;
    BOOL bRetCode;
    BOOL bContinue;

    if (strDirectory.IsEmpty() || !piCallback)
        goto clean0;

    strPath = strDirectory;
    if (strPath[strPath.GetLength() - 1] != _T('\\'))
        strPath += _T("\\");

    strFind = strPath + _T("*.*");

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
                strSubDir += _T("\\");
                if (!DeleteDirectory(strSubDir, piCallback, bKeepRootDir, nLevel + 1))
                    goto clean0;
            }
        }
        else
        {
            CString strFullFilePath = strPath + findData.cFileName;
            ULONGLONG qwFileSize = ((ULONGLONG)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;
            
            if (KGetWinVersion() <= WINVERSION_SERVER_2003)
            {
                if (!::SfcIsFileProtected(NULL, strFullFilePath))
                    DeleteFile(strFullFilePath, qwFileSize, findData.dwFileAttributes, piCallback);
            }
            else
            {
                DeleteFile(strFullFilePath, qwFileSize, findData.dwFileAttributes, piCallback);
            }

            bContinue = piCallback->OnIdle();
            if (!bContinue)
                goto clean0;
        }

        bRetCode = ::FindNextFile(hFind, &findData);
    }

    retval = TRUE;

clean0:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        ::FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
    // 尝试删除空目录,但不删除最顶上的目录
    if (nLevel != 0 || (nLevel == 0 && bKeepRootDir == FALSE))
    {
        BOOL bRemove = RemoveDirectory(strPath);
//        ATLASSERT(bRemove);
    }
//     else
//     {
//         // 创建瘦身后的标记文件
//         CString strFlag = strPath + g_kSlimFlag;
//         CAtlFile file;
//         file.Create(strFlag, 
//             FILE_GENERIC_WRITE, 
//             FILE_SHARE_READ|FILE_SHARE_WRITE,
//             CREATE_ALWAYS);
//     }
    return retval;
}

BOOL slimhelper::RecyclePath(const CString& strFilePath, BOOL bKeepRootDir)
{
    BOOL retval = FALSE;
    CString strOldSecurityDescriptor;
    HRESULT hr;
    int nRetCode;
    ULARGE_INTEGER tempSize;
    DWORD dwFileAttributes;
    BOOL bIsDir = FALSE;
    SHFILEOPSTRUCTW fileopt = { 0 };
    ULONGLONG qwFileSize = 0;
    TCHAR* szDelPath = new TCHAR[MAX_PATH * 2];
    CString strFlagFile;

    RtlZeroMemory(szDelPath, sizeof(TCHAR) * MAX_PATH * 2);
    StringCchCopy(szDelPath, MAX_PATH * 2, strFilePath);

    fileopt.pFrom = szDelPath;
    fileopt.wFunc = FO_DELETE;
    fileopt.fFlags = FOF_SILENT|FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_ALLOWUNDO;

    dwFileAttributes = ::GetFileAttributes(strFilePath);
    if (INVALID_FILE_ATTRIBUTES == dwFileAttributes)
        goto clean0;

    if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        bIsDir = TRUE;

    GrantFileAccess(strFilePath, strOldSecurityDescriptor, bIsDir);
    //if (!GrantFileAccess(strFilePath, strOldSecurityDescriptor))
    //    goto clean0;

    strFlagFile = strFilePath + _T("\\"); 
    strFlagFile += g_kSlimFlag;

    if (bIsDir)
    {
        ::DeleteFile(strFlagFile);
        nRetCode = SHFileOperationW(&fileopt);
        if (32 == nRetCode)
            goto clean0;

        if (0x78 == nRetCode || 5 == nRetCode)
        {
            GrantDirAccess(strFilePath);
            ::DeleteFile(strFlagFile);
            nRetCode = SHFileOperationW(&fileopt);;
        }

        if (!nRetCode)
        {
            if (bKeepRootDir)
            {
                ::CreateDirectory(strFilePath, NULL);
                // 创建瘦身后的标记文件
//                 CAtlFile file;
//                 file.Create(strFlagFile, 
//                     FILE_GENERIC_WRITE, 
//                     FILE_SHARE_READ|FILE_SHARE_WRITE,
//                     CREATE_ALWAYS);
            }
            retval = TRUE;
        }
        goto clean0;
    }

    if (dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED
        || dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
    {
        tempSize.LowPart = GetCompressedFileSize(strFilePath, &tempSize.HighPart);
        qwFileSize = tempSize.QuadPart;
    }
    else
    {
        CAtlFile file;
        hr = file.Create(strFilePath, 
                         FILE_GENERIC_READ, 
                         FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                         OPEN_EXISTING);
        if (FAILED(hr))
            goto clean0;

        tempSize.LowPart = GetFileSize((HANDLE)file, &tempSize.HighPart);
        qwFileSize = tempSize.QuadPart;
    }

    qwFileSize = FileSizeConver::Instance().FileSizeOnDisk(qwFileSize);

    nRetCode = SHFileOperationW(&fileopt);
    if (nRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (szDelPath)
    {
        delete[] szDelPath;
        szDelPath = NULL;
    }

    return retval;
}
//////////////////////////////////////////////////////////////////////////

BOOL slimhelper::ScanFile(const CString& strFilePath, 
                          ULONGLONG qwFileSize,
                          DWORD dwFileAttributes,
                          ISystemSlimCallBack* piCallback)
{
    BOOL retval = FALSE;
    HANDLE hFind = NULL;
    HRESULT hr;
    ULARGE_INTEGER tempSize;
    ULONGLONG qwSaveSize = 0;
    BOOL bCompressed = FALSE;

    if (!piCallback)
        goto clean0;

    if (INVALID_FILE_ATTRIBUTES == dwFileAttributes)
    {
        dwFileAttributes = GetFileAttributes(strFilePath);
        if (INVALID_FILE_ATTRIBUTES == dwFileAttributes)
            goto clean0;
    }

    if (dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED
        || dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
    {
        bCompressed = TRUE;
    }

    if (bCompressed)
    {
        tempSize.LowPart = GetCompressedFileSize(strFilePath, &tempSize.HighPart);
        qwFileSize = tempSize.QuadPart;
    }
    else
    {
        if (0 == qwFileSize)
        {
            CAtlFile file;

            hr = file.Create(
                strFilePath, 
                FILE_GENERIC_READ, 
                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                OPEN_EXISTING
                );
            if (FAILED(hr))
                goto clean0;

            tempSize.LowPart = GetFileSize((HANDLE)file, &tempSize.HighPart);
            qwFileSize = tempSize.QuadPart;
        }
    }

    qwFileSize = FileSizeConver::Instance().FileSizeOnDisk(qwFileSize);

    piCallback->OnScanItem(strFilePath, qwFileSize, bCompressed);

    retval = TRUE;

clean0:
    if (hFind)
    {
        FindClose(hFind);
        hFind = NULL;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

BOOL slimhelper::ScanDirectory(const CString& strDirectory,
                               ISystemSlimCallBack* piCallback)
{
    BOOL retval = FALSE;
    CString strPath;
    CString strFind;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA findData;
    BOOL bRetCode;
    BOOL bContinue;

    if (strDirectory.IsEmpty() || !piCallback)
        goto clean0;

    strPath = strDirectory;
    if (strPath[strPath.GetLength() - 1] != _T('\\'))
        strPath += _T("\\");

    strFind = strPath + _T("*.*");

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
                strSubDir += _T("\\");
                if (!ScanDirectory(strSubDir, piCallback))
                    goto clean0;
            }
        }
        else
        {
            CString strFullFilePath = strPath + findData.cFileName;
            ULONGLONG qwFileSize = ((ULONGLONG)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;
            
            if (KGetWinVersion() <= WINVERSION_SERVER_2003)
            {
                if (!::SfcIsFileProtected(NULL, strFullFilePath))
                    ScanFile(strFullFilePath, qwFileSize, findData.dwFileAttributes, piCallback);
            }
            else
            {
                ScanFile(strFullFilePath, qwFileSize, findData.dwFileAttributes, piCallback);
            }

            bContinue = piCallback->OnIdle();
            if (!bContinue)
                goto clean0;
        }

        bRetCode = ::FindNextFile(hFind, &findData);
    }

    retval = TRUE;

clean0:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////
void slimhelper::GetFileSizeTextString(__int64 qwFileSize, CString& strFileSize)
{
    int nFlag;
    CString strEnd;

    strFileSize = L"";

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
BOOL slimhelper::LocateFile(const CString& strFilePath)
{
    BOOL retval = FALSE;
    CString strShellCmd;
    TCHAR* szCmdLine = NULL;
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    BOOL fRetCode;
    WIN32_FILE_ATTRIBUTE_DATA wfad;

    BOOL bGetAtt = GetFileAttributesEx(strFilePath, GetFileExInfoStandard, &wfad);
    if (bGetAtt)
    {
        if (wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
            wfad.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        {
            ShellExecute(NULL, L"open", strFilePath, NULL, NULL, SW_SHOWNORMAL);
            retval = TRUE;
            goto clean0;
        }
    }
    
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
//////////////////////////////////////////////////////////////////////////

BOOL slimhelper::GainSecurityPrivilege1()
{
    BOOL retval = FALSE;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tp;
    BOOL fRetCode;

    fRetCode = ::OpenProcessToken(
        ::GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES, 
        &hToken
        );
    if (!fRetCode)
        goto clean0;

    fRetCode = ::LookupPrivilegeValue(
        NULL, 
        SE_SECURITY_NAME, 
        &tp.Privileges[0].Luid
        );
    if (!fRetCode)
        goto clean0;

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    fRetCode = ::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
    if (!fRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (hToken) 
    {
        ::CloseHandle(hToken);
        hToken = NULL;
    }
    return retval;
}


BOOL slimhelper::GainSecurityPrivilege2()
{
    BOOL retval = FALSE;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tp;
    BOOL fRetCode;

    fRetCode = ::OpenProcessToken(
        ::GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES, 
        &hToken
        );
    if (!fRetCode)
        goto clean0;

    fRetCode = ::LookupPrivilegeValue(
        NULL, 
        SE_TAKE_OWNERSHIP_NAME, 
        &tp.Privileges[0].Luid
        );
    if (!fRetCode)
        goto clean0;

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    fRetCode = ::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
    if (!fRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (hToken) 
    {
        ::CloseHandle(hToken);
        hToken = NULL;
    }
    return retval;
}

//////////////////////////////////////////////////////////////////////////
