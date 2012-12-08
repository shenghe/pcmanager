
#include "stdafx.h"
#include "trashpublicfunc.h"
#include <Shlwapi.h>
#include "filtertool.h"
#include "linkchecker.h"
#include "stubbornfiles.h"
#include "misc/KppUserLog.h"
#include "getenvpath.h"
#include <intshcut.h>
#include <algorithm>
#include "misc/wildcmp.h"
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

using namespace std;

void SplitString(CString str, WCHAR split, std::vector <CString>& vcResult)
{
    int iPos = 0; //分割符位置
    int iNums = 0; //分割符的总数
    CString strTemp = str;
    CString strRight;
    //先计算子字符串的数量

    CString strLeft;
    while (true)
    {
        iPos = strTemp.Find(split);
        if(iPos == -1)
            break;
        //左子串
        strLeft = strTemp.Left(iPos);
        //右子串
        strRight = strTemp.Mid(iPos + 1, strTemp.GetLength());
        strTemp = strRight;
        vcResult.push_back(strLeft);
    }
}


int KAEGetSystemVersion(SYSTEM_VERSION *pnSystemVersion)
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

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
		// Test for the Windows NT product family.
	case VER_PLATFORM_WIN32_NT:

		// Test for the specific product.
		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
			*pnSystemVersion = enumSystem_Win_Vista;

		// Test for the specific product.
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

		// Test for the Windows Me/98/95.
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


CString KasLangFormatFileName( LPCTSTR pszName )
{
    CString strConfigName;

    strConfigName = _T("cfg\\trashconfig.dat");
    return strConfigName;
}



CString KasLangGetTrashConfigFilePath()
{
    TCHAR szDir[MAX_PATH] = {0};
    int nRet = ::GetModuleFileName( NULL, szDir, MAX_PATH );
    if( nRet == 0 )
        return TEXT( "" );

    ::PathRemoveFileSpec( szDir );
    ::PathAddBackslash( szDir );


    CString strFile = KasLangFormatFileName( FILENAME_TRASHCONFIG );
    if( strFile.GetLength() == 0 )
        return TEXT( "" );


    return szDir + strFile;
}

void GetFileSizeTextString(__int64 qwFileSize, CString& strFileSize)
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

void GetFileCountText(UINT nCount, CString& strCount)
{
	CString strRes;
	UINT n = 0;
	int nTep;

	if (0 == nCount)
	{
		strRes = L"0";
		goto Clear0;
	}

	while (nCount)
	{
		if (n % 3 == 0 && n != 0)
			strRes = _T(",") + strRes;
		n++;
		nTep = nCount % 10;
		CString s;
		s.Format(L"%d",nTep);
		strRes = s + strRes;
		nCount /= 10;
	}
	
Clear0:
	strCount += strRes;
}

CString GetExtName(CString fileName)
{
    int pos=fileName.Find(L"."); //获取 . 的位置
    if(pos==-1){ //如果没有找到，直接返回该字符串
        return fileName;
    }else{
        return GetExtName(fileName.Mid(pos+1)); //找到了的话，往深层遍历，直到最底层
    }
}

HANDLE KscOpenFile(const TCHAR cszFileName[], int nMode)
{
    static unsigned uModeToAccess[3] = 
    {
        GENERIC_READ,                   // Mode = 0 : Read Only | Open_Exist
        GENERIC_READ | GENERIC_WRITE,   // Mode = 1 : Read/Write | Open_Exist
        GENERIC_READ | GENERIC_WRITE    // Mode = 2 : Read/Write | Create_Always
    };
    static unsigned uModeToShareAccess[3] = 
    {
        FILE_SHARE_READ | FILE_SHARE_WRITE, // Mode = 0 : Read Only | Open_Exist
        0,                                  // Mode = 1 : Read/Write | Open_Exist
        0                                   // Mode = 2 : Read/Write | Create_Always
    };
    static unsigned uModeToCreate[3] = 
    {
        OPEN_EXISTING,  // Mode = 0 : Read Only | Open_Exist
        OPEN_EXISTING,  // Mode = 1 : Read/Write | Open_Exist
        CREATE_ALWAYS   // Mode = 2 : Read/Write | Create_Always
    };

    static unsigned uModeToAttribute[3] = 
    {
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE,
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE,
        FILE_ATTRIBUTE_NORMAL
    };

    return ::CreateFile(
        cszFileName,
        uModeToAccess[nMode],
        uModeToShareAccess[nMode],
        NULL,
        uModeToCreate[nMode],
        uModeToAttribute[nMode],
        NULL
        );
}

int KscCloseFile(HANDLE hFileHandle)
{
    return ::CloseHandle((HANDLE)hFileHandle) ? 0 : ::GetLastError();
}


BOOL FindFileInDirectory(LPCTSTR pszFullPath,
                         const std::vector<CString>& extArray, 
                         int nRecursive, 
                         int nStopIndex,
                         ITraverseFile *pTF,
                         CFileListData *pFilelist,
                         int nCurrentIndex,
                         std::vector<CString> &vFilt,
						 CString strScanItem)
{
    BOOL bResult = TRUE;
    WIN32_FIND_DATA ff = { 0 }; 

    if ( g_bStop[nStopIndex] )
    {
        bResult = TRUE;
        goto Exit0;
    }

	if (nCurrentIndex == WIN_HUISHOUZHAN)
	{
		SHQUERYRBINFO syscleInfo = {0};
		syscleInfo.cbSize = sizeof(SHQUERYRBINFO);
		CString str;
		str = L"双击查看详情";
		int iCount = SHQueryRecycleBin(NULL, &syscleInfo);
		FINDFILEDATA fileData;
		if (syscleInfo.i64NumItems == 0)
			goto Exit0;

		fileData.nCount = int(syscleInfo.i64NumItems & 0xFFFFFFFF);
		fileData.uFileSize = syscleInfo.i64Size;
		fileData.pszFileNmae = L"";
		fileData.pszPathName = str;
		fileData.pszExtName = L"";

		if (pTF != NULL)
		{
			pTF->TraverseFile(&fileData);
		}
		else if (pFilelist != NULL)
		{
			pFilelist->AddTrashFile(nCurrentIndex, &fileData);
		}
		goto Exit0;
	}

    if(CLinkChecker::Instance().IsDirectoryJunction(pszFullPath))
    {
        goto Exit0;
    }
    if (nCurrentIndex == SYS_TRASH)
    {
        FindFileByExtListInDirectory(pszFullPath, extArray,nStopIndex,pTF,pFilelist,nCurrentIndex,vFilt, strScanItem);
    }
    else
    {
        // 对本目录下的指定扩展名的文件作搜索
        for (int i = 0; i < (int)extArray.size() && !g_bStop[nStopIndex]; i++)
        {
            bResult = FindFileWithExtInDirectory(pszFullPath, extArray[i],nStopIndex,pTF,pFilelist,nCurrentIndex,vFilt, strScanItem);
            if (!bResult)
                continue;

        }
    }

     if ( nRecursive <= 0 || g_bStop[nStopIndex] )
         goto Exit0;

    // 递归搜索子目录
    TCHAR szFindName[FINDFILETRAVERSE_MAX_PATH] = {0};
    _tcsncpy_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszFullPath,  FINDFILETRAVERSE_MAX_PATH - 1 );
    _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, TEXT("*.*") , FINDFILETRAVERSE_MAX_PATH - _tcslen(pszFullPath) - 1 );
    HANDLE findhandle = ::FindFirstFile(szFindName, &ff); 

    if( findhandle == INVALID_HANDLE_VALUE )
    {
        bResult = FALSE;
        goto Exit0;
    }

    BOOL res = TRUE;

    while( res && !g_bStop[nStopIndex] )
    {
        if( ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  
        {
            if (( _tcscmp( ff.cFileName, _T( "."  ) ) != 0) 
                && ( _tcscmp( ff.cFileName, _T( ".." ) ) != 0 ))
            {
                memset(szFindName, 0, sizeof(szFindName));
                _tcsncpy_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszFullPath,
                    FINDFILETRAVERSE_MAX_PATH - 1 );
                _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, ff.cFileName, 
                    FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );
                _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH,TEXT("\\"),
                    FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );

                // %temp% 过滤cookies
                if (nCurrentIndex == WIN_USERTEMP && _wcsicmp(ff.cFileName , _T("cookies")) == 0)
                {
                    goto FindFileContinue;
                }
                else if (nCurrentIndex == BROWER_IE && _wcsicmp(ff.cFileName, _T("Enclosure")) == 0)
                {
                    goto FindFileContinue;
                }
                    
                FindFileInDirectory( szFindName, extArray, nRecursive-1,nStopIndex,pTF,pFilelist,nCurrentIndex,vFilt, strScanItem);  
            }
        }
FindFileContinue:
        res = ::FindNextFile( findhandle, &ff );
    }

    ::FindClose( findhandle );
Exit0:
    return bResult;       
}


BOOL FileFilter(LPCTSTR pszFileName)
{
    unsigned int uFilterCount = sizeof(g_pszFilterFiles) / sizeof(LPCTSTR);
    BOOL nRet = FALSE;
    for (unsigned int i = 0; i < uFilterCount; i++)
    {
        CString strFiltTmp(g_pszFilterFiles[i]);
        if (strFiltTmp.CompareNoCase(pszFileName) == 0)
        {
            nRet = TRUE;
            break;
        }
    }
    return nRet;
}

BOOL DirFilter(LPCTSTR pszDirName)
{
    unsigned int uFilterCount = sizeof(g_pszFilterDirs) / sizeof(LPCTSTR);
    BOOL bRet = FALSE;
    CString strDir(pszDirName);

    if (pszDirName == NULL)
        goto Exit0;

    if (strDir.GetLength()<=3)
    {
        bRet = TRUE;
        goto Exit0;
    }

    for (unsigned int i = 0; i < uFilterCount; i++)
    {
        if (strDir.CompareNoCase(g_pszFilterDirs[i]) == 0)
        {
            bRet = TRUE;
            break;
        }
    }
Exit0:
    return bRet;
}


// 查找指定扩展名的文件
BOOL FindFileWithExtInDirectory(LPCTSTR pszFullPath, 
                                LPCTSTR pszExt, 
                                int nStopIndex, 
                                ITraverseFile *pTF,
                                CFileListData *pFilelist,
                                int nCurrentIndex,
                                std::vector<CString> &vFilt,
								 CString strScanItem)
{
    BOOL				bResult = TRUE;
    TCHAR				szFindName[FINDFILETRAVERSE_MAX_PATH] = {0};
    WIN32_FIND_DATA		ff = { 0 };
    FINDFILEDATA		TraverseFileData = { 0 };
    HANDLE				findhandle = INVALID_HANDLE_VALUE;
    BOOL				res = TRUE;
    bool				bNeedCheckFilter = false;
    std::wstring		strExt;

	std::wstring        strExtEx;
	int nPos = -1;
	strExtEx = pszExt;

    if ( g_bStop[nStopIndex] )
        return FALSE;

    // 拼装查找路径
	nPos = (int)strExtEx.find('.');
	if (nPos != -1)
	{
		_tcsncpy_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszFullPath,  
			FINDFILETRAVERSE_MAX_PATH - 1 );
		_tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszExt, 
			FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );
	}
	else
	{
		_tcsncpy_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszFullPath,  
			FINDFILETRAVERSE_MAX_PATH - 1 );
		_tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, _T("*."), 
			FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );
		_tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszExt, 
			FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );
	}


    TraverseFileData.pszPathName = pszFullPath;
    findhandle = ::FindFirstFile( szFindName, &ff );


    if( findhandle == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }


    // 查找文件
    while( res && !g_bStop[nStopIndex] )
    {
        if( ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  
        {
        }
        else
        {
            TraverseFileData.pszExtName  = pszExt;
            TraverseFileData.pszFileNmae = ff.cFileName;

//             if (nCurrentIndex == WIN_PREFETCH)
//             { // 过滤目录创建时间未超过7×24小时的文件
//                 if (!IsFileTimeInner(ff.ftCreationTime, 7))
//                     goto Clean0;
//             }
            
            CString strFileFullName(TraverseFileData.pszPathName);
            strFileFullName += TraverseFileData.pszFileNmae;
            strFileFullName.MakeLower();
            if(CStubbornFiles::Instance().IsStubbornFile(strFileFullName))
                goto Clean0;            
           
            for(int ivIndex=0;ivIndex<(int)vFilt.size();ivIndex++)
            {
              
                if(CFilterTool::Instance()._DoFindFuncMap(vFilt.at(ivIndex),strFileFullName))
                    goto Clean0;
            }  

			if (!IsExtCompare(ff.cFileName, pszExt))
			{
				goto Clean0;
			}
			if (wcsicmp(ff.cFileName, L"suggestedsites.dat") == 0 && nCurrentIndex == BROWER_IE)
			{
				goto Clean0;
			}
            TraverseFileData.uFileSize   = ff.nFileSizeHigh;
            TraverseFileData.uFileSize   = TraverseFileData.uFileSize << 32;
            TraverseFileData.uFileSize  |= ff.nFileSizeLow;

			CString strLog = L"[Scan][";
			strLog += strScanItem;
			strLog += "]:";
			strLog += strFileFullName;
			KppUserLog::UserLogInstance().WriteSingleLog(strLog.GetBuffer());
            if (pTF != NULL)
                pTF->TraverseFile(&TraverseFileData);
            else if (pFilelist != NULL)
                pFilelist->AddTrashFile(nCurrentIndex, &TraverseFileData);
        }
Clean0:
        if(  g_bStop[nStopIndex] )
            break;

        res = ::FindNextFile( findhandle, &ff );
    }

    ::FindClose( findhandle );
    return bResult;       
}
BOOL FindFileByExtListInDirectory(LPCTSTR pszFullPath, 
                                  const std::vector<CString> &vExt, 
                                  int nStopIndex, 
                                  ITraverseFile *pTF,
                                  CFileListData *pFilelist,
                                  int nCurrentIndex,
                                  std::vector<CString> &vFilt,
                                  CString& strScanItem)
{
    BOOL				bResult = TRUE;
    TCHAR				szFindName[FINDFILETRAVERSE_MAX_PATH] = {0};
    WIN32_FIND_DATA		ff = { 0 };
    FINDFILEDATA		TraverseFileData = { 0 };
    HANDLE				findhandle = INVALID_HANDLE_VALUE;
    BOOL				res = TRUE;
    bool				bNeedCheckFilter = false;
    std::wstring		strExt;
 
    int nPos = -1;

    if ( g_bStop[nStopIndex] )
        return FALSE;

    _tcsncpy_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszFullPath,  
        FINDFILETRAVERSE_MAX_PATH - 1 );
    _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, _T("*.*"), 
        FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );

    TraverseFileData.pszPathName = pszFullPath;
    findhandle = ::FindFirstFile( szFindName, &ff );


    if( findhandle == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }


    // 查找文件
    while( res && !g_bStop[nStopIndex] )
    {
        if( ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  
        {
            goto Clean0;
        }
        else
        {
            size_t i;
            for (i = 0; i < vExt.size(); ++i)
            {
                LPCTSTR extStr = vExt[i];
                if (IsExtCompare(ff.cFileName, extStr))
                {
                    break;
                }
            }
            if (i == vExt.size())
            {
                goto Clean0;
            }

//            TraverseFileData.pszExtName  = pszExt;
            TraverseFileData.pszFileNmae = ff.cFileName;

            CString strFileFullName(TraverseFileData.pszPathName);
            strFileFullName += TraverseFileData.pszFileNmae;
            strFileFullName.MakeLower();
            if(CStubbornFiles::Instance().IsStubbornFile(strFileFullName))
                goto Clean0;

            for(int ivIndex=0;ivIndex<(int)vFilt.size();ivIndex++)
            {

                if(CFilterTool::Instance()._DoFindFuncMap(vFilt.at(ivIndex),strFileFullName))
                    goto Clean0;
            }  

//             if (!IsExtCompare(ff.cFileName, pszExt))
//             {
//                 goto Clean0;
//             }
            if (wcsicmp(ff.cFileName, L"suggestedsites.dat") == 0 && nCurrentIndex == BROWER_IE)
            {
                goto Clean0;
            }
            TraverseFileData.uFileSize   = ff.nFileSizeHigh;
            TraverseFileData.uFileSize   = TraverseFileData.uFileSize << 32;
            TraverseFileData.uFileSize  |= ff.nFileSizeLow;

            CString strLog = L"[Scan][";
            strLog += strScanItem;
            strLog += "]:";
            strLog += strFileFullName;
            KppUserLog::UserLogInstance().WriteSingleLog(strLog.GetBuffer());
            if (pTF != NULL)
                pTF->TraverseFile(&TraverseFileData);
            else if (pFilelist != NULL)
                pFilelist->AddTrashFile(nCurrentIndex, &TraverseFileData);
        }
Clean0:
        if(  g_bStop[nStopIndex] )
            break;

        res = ::FindNextFile( findhandle, &ff );
    }

    ::FindClose( findhandle );
    return bResult;
}

 BOOL DeleteFileWithExtInDirectory(LPCTSTR pszFullPath, 
                                   LPCTSTR pszExt,
                                   int nStopIndex,
                                   CFileListData *pFilelist,
                                   int nCurIndex,
                                   ICleanCallback *pCcb,
                                   CFileDelete *pDelFile,
                                   std::wstring strDesc,
                                   std::vector<CString> &vFilt)
 {
     BOOL				bResult = TRUE;
     TCHAR				szFindName[FINDFILETRAVERSE_MAX_PATH] = {0};
     WIN32_FIND_DATA		ff = { 0 };
     FINDFILEDATA		TraverseFileData = { 0 };
     HANDLE				findhandle = INVALID_HANDLE_VALUE;
     BOOL				res = TRUE;
     bool				bNeedCheckFilter = false;
     std::wstring		strExt;
 
     if (g_bStop[nStopIndex])
     {
         bResult = FALSE;
         goto Exit0;
     }

     if(pFilelist == NULL||nCurIndex < 0||pCcb == NULL)
     {
         bResult = FALSE;
         goto Exit0;
     }
 
     _tcsncpy_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszFullPath,  
         FINDFILETRAVERSE_MAX_PATH - 1 );
     _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, _T("*."), 
         FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );
     _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszExt, 
         FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );
 
     TraverseFileData.pszPathName = pszFullPath;
     findhandle = ::FindFirstFile( szFindName, &ff );
 
 
     if( findhandle == INVALID_HANDLE_VALUE )
     {
         return FALSE;
     }
 
 
     // 查找文件
     while (res&&!g_bStop[nStopIndex])
     {
         if( ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  
         {
         }
         else
         {
             TraverseFileData.pszExtName  = pszExt;
             TraverseFileData.pszFileNmae = ff.cFileName;
 
             TraverseFileData.uFileSize   = ff.nFileSizeHigh;
             TraverseFileData.uFileSize   = TraverseFileData.uFileSize << 32;
             TraverseFileData.uFileSize  |= ff.nFileSizeLow;

//              if (nCurIndex==0)
//              {
//                  if(!IsFileTimeInner(ff.ftLastAccessTime))
//                      goto Clean0;
//              }
             if (wcsicmp(ff.cFileName, L"suggestedsites.dat") == 0 && nCurIndex == BROWER_IE)
             {
                 goto Clean0;
             }

			 CString filepathname(TraverseFileData.pszPathName); 
             filepathname += TraverseFileData.pszFileNmae;
             filepathname.MakeLower();
             if(CStubbornFiles::Instance().IsStubbornFile(filepathname))
                 goto Clean0;

             for(int ivIndex=0;ivIndex<(int)vFilt.size();ivIndex++)
             {
                 if(CFilterTool::Instance()._DoFindFuncMap(vFilt.at(ivIndex),filepathname))
                     goto Clean0;
             } 

			 DWORD curSize = (DWORD)pFilelist->AddTrashFile(nCurIndex, &TraverseFileData);

			 pCcb->OnCleanEntry(
                 (int)strDesc.size(),
                 strDesc.c_str(),
				 filepathname.GetLength(),
				 filepathname.GetBuffer(),
				 (double)curSize / (1024 * 1024));

			 if(Delete_File_Success == pDelFile->TrashFileDelete(filepathname.GetBuffer()))
			 {
			 }
			 else
			 {
				 CStubbornFiles::Instance().AddFile(filepathname);
			 }
		 }
Clean0:
		 if(  g_bStop[nStopIndex] )
			 break;

		 res = ::FindNextFile( findhandle, &ff );
	 }

	 ::FindClose( findhandle );
	 CStubbornFiles::Instance().Sync();
Exit0:
     return bResult;       
 }

 BOOL DeleteFileInDirectory(LPCTSTR pszFullPath,
                            const std::vector<CString>& extArray, 
                            int nRecursive,
                            int nStopIndex,
                            CFileListData *pFilelist,
                            int nCurIndex,
                            ICleanCallback *pCcb,
                            CFileDelete *pDelFile,
                            std::wstring strDesc,
                            std::vector<CString> &vFilt)
 {
     BOOL bResult = FALSE;
     WIN32_FIND_DATA ff = { 0 }; 

     if (g_bStop[nStopIndex])
     {
         bResult = TRUE;
         goto Exit0;
     }

     for (int i = 0; i < (int)extArray.size() && !g_bStop[nStopIndex]; i++)
     {
         bResult = DeleteFileWithExtInDirectory(pszFullPath, 
                                         extArray[i],
                                         nStopIndex,
                                         pFilelist,
                                         nCurIndex,
                                         pCcb,
                                         pDelFile,
                                         strDesc,
                                         vFilt);
         if (!bResult)
             continue;
     }

     if (nRecursive <= 0||g_bStop[nStopIndex])
         goto Exit0;

     if(CLinkChecker::Instance().IsDirectoryJunction(pszFullPath))
     {
         goto Exit0;
     }

     // 递归搜索子目录
     TCHAR szFindName[FINDFILETRAVERSE_MAX_PATH] = {0};
     _tcsncpy_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszFullPath,  FINDFILETRAVERSE_MAX_PATH - 1 );
     _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, TEXT("*.*") , FINDFILETRAVERSE_MAX_PATH - _tcslen(pszFullPath) - 1 );
     HANDLE findhandle = ::FindFirstFile( szFindName, &ff ); 

     if (findhandle == INVALID_HANDLE_VALUE)
     {
         bResult = FALSE;
         goto Exit0;
     }

     BOOL res = TRUE;

     while (res&&!g_bStop[nStopIndex])
     {
         if( ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  
         {
             if (( _tcscmp( ff.cFileName, _T( "."  ) ) != 0) 
                 && ( _tcscmp( ff.cFileName, _T( ".." ) ) != 0 ))
             {
                 memset(szFindName, 0, sizeof(szFindName));
                 _tcsncpy_s( szFindName, FINDFILETRAVERSE_MAX_PATH, pszFullPath,
                     FINDFILETRAVERSE_MAX_PATH - 1 );
                 _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH, ff.cFileName, 
                     FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );
                 _tcsncat_s( szFindName, FINDFILETRAVERSE_MAX_PATH,TEXT("\\"),
                     FINDFILETRAVERSE_MAX_PATH - _tcslen(szFindName) - 1 );

                 DeleteFileInDirectory(szFindName,
                     extArray, 
                     nRecursive - 1,
                     nStopIndex,
                     pFilelist,
                     nCurIndex,
                     pCcb,
                     pDelFile,
                     strDesc,
                     vFilt);
             }
         }

         res = ::FindNextFile( findhandle, &ff );
     }

     ::FindClose( findhandle );
Exit0:
     return bResult;       
 }

 void FileTimeToTime_t( FILETIME ft, time_t *t )
 {
   
     ULARGE_INTEGER ui;

     ui.LowPart = ft.dwLowDateTime;
     ui.HighPart = ft.dwHighDateTime;

     *t = ((ULONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
 }

 DWORD GetDiffDays( SYSTEMTIME local,  FILETIME ftime )
 {
     FILETIME flocal;
     time_t ttlocal;
     time_t ttfile;

     SystemTimeToFileTime( &local, &flocal );

     FileTimeToTime_t( flocal, &ttlocal );
     FileTimeToTime_t( ftime, &ttfile );

     time_t difftime = ttlocal - ttfile;

     return (DWORD)(difftime / (24 * 3600L)) ;// 除以每天24小时3600秒
 }


 BOOL IsFileTimeInner(FILETIME tCurFile, int nDays)
 {
    BOOL bRet = FALSE;
    SYSTEMTIME stLocal;   
    GetSystemTime(&stLocal);  
    DWORD dwDiff = GetDiffDays(stLocal,tCurFile);

    if(dwDiff >= (DWORD)nDays)
        bRet = TRUE;

    return bRet;

 }

 BOOL IsExtCompare(std::wstring strFileName, std::wstring strExt)
 {
	 BOOL bRet = FALSE;
	 int nSize = 0;
	 int nFileSize = 0;
	 std::wstring strSubName;
	 nSize = (int)strExt.size();
	 nFileSize = (int)strFileName.size();
	 if (strExt.compare(L"*") == 0)
	 {
		 bRet = TRUE;
		 goto clean0;
	 }
	 if (nFileSize < nSize)
	 {
		 goto clean0;
	 }
	strSubName = strFileName.substr(nFileSize-nSize, nSize);
	std::transform(strSubName.begin(), strSubName.end(), strSubName.begin(), ::tolower);
	std::transform(strExt.begin(), strExt.end(), strExt.begin(), ::tolower);
	bRet = wildcmp(strExt.c_str(), strSubName.c_str());	
// 	if (strSubName.compare(strExt) == 0)
// 	{
// 		bRet = TRUE;
// 	}
// 	else
// 	{
// 		bRet = FALSE;
// 	}
clean0:
	return bRet;
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
         if(wcsicmp(pe.szExeFile,szProcessName) == 0)
         {
             id = pe.th32ProcessID;
             break;
         }

     }
     CloseHandle(hSnapshot);
     return id;
 }
 DWORD GetProcessPathFromName(LPCTSTR szProcessName, CString& strPath)
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
         if(wcsicmp(pe.szExeFile,szProcessName) == 0)
         {
             id = pe.th32ProcessID;
             const DWORD dwProcessId = id;
             TCHAR modulePath[MAX_PATH];
             // 打开进程句柄
             HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
             if (NULL != hProcess)
             {
                 DWORD cbNeeded;
                 HMODULE hMod;
                 // 获取路径
                 if (::EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
                     ::GetModuleFileNameEx(hProcess, hMod, modulePath, MAX_PATH);
                 
                 strPath = modulePath;

                 CloseHandle(hProcess);
             }

             break;
         }

     }
     CloseHandle(hSnapshot);
     return id;
 }
 BOOL IsItemProcessRunning(int nCurIndex)
 {
    BOOL bRet = FALSE;
    std::vector<CString> vecPro;
    std::vector<CString>::iterator ite;
    CString strPath;
    
    GetProcessNamesByID(nCurIndex, vecPro);
    
    for (ite = vecPro.begin(); ite != vecPro.end(); ++ite)
    {
        bRet = CFilterTool::Instance().CheckProcessIsRunning(*ite);
        if (bRet)
            goto Clear0;
    }
        
Clear0:
    return bRet;
 }

 BOOL GetProcessNamesByID(UINT nCurIndex, std::vector<CString>& vecPro)
 {
     BOOL bRet = FALSE;

     switch (nCurIndex)
     {
     case BROWER_IE:
         vecPro.push_back(L"iexplore.exe");
         vecPro.push_back(L"360se.exe");
         vecPro.push_back(L"TTRAVE~1.EXE");
         vecPro.push_back(L"TTraveler.exe");
         vecPro.push_back(L"TheWorld.exe");
         vecPro.push_back(L"maxthon.exe");
         vecPro.push_back(L"sogouexplorer.exe");
         break;

     case VIDEO_SOGOU:
     case BROWER_SOGO:
         vecPro.push_back(L"sogouexplorer.exe");
         break;

     case BROWER_FIREFOX:
         vecPro.push_back(L"firefox.exe");
         break;

     case BROWER_OPERA:
         vecPro.push_back(L"opera.exe");
         break;

     case BROWER_MATHRON:
         vecPro.push_back(L"maxthon.exe");
         break;

     case BROWER_SAFARI:
         vecPro.push_back(L"Safari.exe");
         break;

     case BROWER_CHROME:
         vecPro.push_back(L"chrome.exe");
         break;

     case VIDEO_KU6:
         vecPro.push_back(L"Ku6SpeedUpper.exe");
         break;

    case VIDEO_TUDOU:
        vecPro.push_back(L"TudouVa.exe");
        break;

    case VIDEO_YOUKU:
        vecPro.push_back(L"ikucmc.exe");
        break;

    case VIDEO_FENGXING:
        vecPro.push_back(L"Funshion.exe");
        break;

    case VIDEO_XUNLEI:
        vecPro.push_back(L"XMP.exe");
        break;

    case VIDEO_QQLIVE:
        vecPro.push_back(L"QQLive.exe");
        break;

    case VIDEO_PPTV:
        vecPro.push_back(L"PPLive.exe");
        break;

    case WIN_REMOTE_DESKTOP:
        vecPro.push_back(L"mstsc.exe");
        break;

    case SOFT_PICASA:
        vecPro.push_back(L"Picasa3.exe");
        break;

    case WIN_LIVE_MGR:
        vecPro.push_back(L"msnmsgr.exe");
        break;

    case VIDEO_QVOD:
        vecPro.push_back(L"QvodPlayer.exe");
        break;

    case VIDEO_STORM:
        vecPro.push_back(L"Storm.exe");
        break;
    
    case VIDEO_PIPI:
        vecPro.push_back(L"PIPIPlayer.exe");
        break;
        
    case VIDEO_PPS:
        vecPro.push_back(L"PPStream.exe");
        break;

    case VIDEO_QQMUSIC:
        vecPro.push_back(L"QQMusic.exe");
        break;

    case VIDEO_KUWO:
        vecPro.push_back(L"KwMusic.exe");
        break;

    case VIDEO_KUGOO:
        vecPro.push_back(L"KuGoo.exe");
        break;

    case VIDEO_TTPLAYER:
        vecPro.push_back(L"TTPlayer.exe");
        break;

    default:
         {
             bRet = FALSE;
             goto Clear0;
         }
     }

     bRet = TRUE;
Clear0:
     return bRet;
 }
//  HRESULT FindLinkArgument(const WCHAR *pLnkFilePath, 
// 	 BOOL &bHave)
//  {
// 	 HRESULT hRetResult = E_FAIL;
// 	 bHave = FALSE;
// 	 IShellLink* pLink = NULL;
// 	 IPersistFile* pPersisFile = NULL;
// 	 WIN32_FIND_DATA wfd = {0};
// 	 WCHAR szShellLinkArgument[MAX_PATH] = {0};
// 	 std:: wstring outRealPath = L"";
// 
// 	 if (!pLnkFilePath || 0x00 == pLnkFilePath[0])
// 	 {
// 		 goto _abort;
// 	 }
// 
// 	 ::CoInitialize(NULL);
// 
// 	 if (SUCCEEDED(CoCreateInstance(
// 		 CLSID_ShellLink, 
// 		 NULL, 
// 		 CLSCTX_INPROC_SERVER, 
// 		 IID_IShellLink, 
// 		 (void **) &pLink)))
// 	 {
// 		 hRetResult = pLink->QueryInterface(IID_IPersistFile, (void **)&pPersisFile);
// 		 if (FAILED(hRetResult))
// 		 {
// 			 goto _abort;
// 		 }
// 
// 		 hRetResult = pPersisFile->Load(
// 			 pLnkFilePath, 
// 			 STGM_READ);
// 		 if (SUCCEEDED(hRetResult))
// 		 {  
// 			 hRetResult = pLink->Resolve(NULL, SLR_ANY_MATCH|SLR_NO_UI);
// 			 if (FAILED(hRetResult))
// 			 {
// 				 goto _abort;
// 			 }
// 
// 			 hRetResult = pLink->GetPath(szShellLinkArgument, MAX_PATH,(WIN32_FIND_DATA *)&wfd, 
// 				 SLGP_UNCPRIORITY);
// 			
// 			 if (wcslen(szShellLinkArgument)>0)
// 			 {
// 				 outRealPath = szShellLinkArgument;
// 				 if(::PathFileExists(outRealPath.c_str()))
// 					 bHave = TRUE;
// 				 hRetResult = S_OK;
// 			 }
// 			 else
// 			 {
// 				 pLink->GetWorkingDirectory(szShellLinkArgument,MAX_PATH);
// 				 if(::PathIsDirectory(szShellLinkArgument))
// 					bHave = TRUE;
// 				 hRetResult = S_OK;
//  				 				
// 			 }
// 			
// 		 }
// 		 else
// 		 {
// 			 hRetResult = E_FAIL;
// 		 }
// 	 }
// 
// _abort:
// 
// 	 if (pPersisFile)
// 	 {
// 		 pPersisFile->Release();
// 		 pPersisFile = NULL;
// 	 }
// 
// 	 if (pLink)
// 	 {
// 		 pLink->Release();
// 		 pLink = NULL;
// 	 }
// 
// 	 ::CoUninitialize();
// 
// 	 return hRetResult;
//  }
// 
// 
//  HRESULT FindLinkUrl(const WCHAR *pLnkFilePath, 
// 	 BOOL &bHave)
//  {
// 	 HRESULT hRetResult = E_FAIL;
// 	 bHave = FALSE;
// 	 IUniformResourceLocator * pLink = NULL;
// 	 IPersistFile* pPersisFile = NULL;
// 	 WCHAR szShellLinkArgument[MAX_PATH] = {0};
// 
// 	 if (!pLnkFilePath || 0x00 == pLnkFilePath[0])
// 	 {
// 		 goto _abort;
// 	 }
// 
// 	 ::CoInitialize(NULL);
// 
// 	 if (SUCCEEDED(CoCreateInstance(
// 		 CLSID_InternetShortcut	, 
// 		 NULL, 
// 		 CLSCTX_INPROC_SERVER, 
// 		 IID_IUniformResourceLocator, 
// 		 (void **) &pLink)))
// 	 {
// 		 hRetResult = pLink->QueryInterface(IID_IPersistFile, (void **)&pPersisFile);
// 		 if (FAILED(hRetResult))
// 		 {
// 			 goto _abort;
// 		 }
// 
// 		 hRetResult = pPersisFile->Load(
// 			 pLnkFilePath, 
// 			 STGM_READ);
// 		 if (SUCCEEDED(hRetResult))
// 		 {  
// 			 if (FAILED(hRetResult))
// 			 {
// 				 goto _abort;
// 			 }
// 
// 			 hRetResult = pLink->GetURL((LPWSTR*)&szShellLinkArgument);
// 
// 			 if (wcslen(szShellLinkArgument)>0)
// 			 {
// 				 bHave = TRUE;
// 				 hRetResult = S_OK;
// 			 }
// 			 else
// 			 {
// 
// 			 }
// 
// 		 }
// 		 else
// 		 {
// 			 hRetResult = E_FAIL;
// 		 }
// 	 }
// 
// _abort:
// 
// 	 if (pPersisFile)
// 	 {
// 		 pPersisFile->Release();
// 		 pPersisFile = NULL;
// 	 }
// 
// 	 if (pLink)
// 	 {
// 		 pLink->Release();
// 		 pLink = NULL;
// 	 }
// 
// 	 ::CoUninitialize();
// 
// 	 return hRetResult;
//  }
// 
