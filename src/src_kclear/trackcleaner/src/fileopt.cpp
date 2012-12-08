#include "stdafx.h"
#include "fileopt.h"
#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"


void CFileOpt::DoFileFolder(LPCTSTR lpPath,CSimpleArray<CString>& vec_folder,BOOL bRecursion,BOOL bFullPath)
{
	try
	{	
		int len = (int)wcslen(lpPath);
		if(lpPath==NULL || len<=0) return;

		//NotifySys(NRS_DO_EVENTS, 0,0);

		CString strFilePath;
		strFilePath = lpPath;
		if (strFilePath.GetAt(len-1) != '\\')
		{
			strFilePath.Append(_T("\\"));
		}
		strFilePath.Append(_T("*"));

		WIN32_FIND_DATA fd;
		HANDLE hFindFile = FindFirstFile(strFilePath.GetBuffer(), &fd);
		if(hFindFile == INVALID_HANDLE_VALUE)
		{
			::FindClose(hFindFile); return;
		}
		
		CString strTempFilePath;
		BOOL bFinish = FALSE; BOOL bUserReture=TRUE; BOOL bIsDirectory;
		while(!bFinish)
		{
			strTempFilePath = lpPath;
			if (strTempFilePath.GetAt(len-1) != '\\')
			{
				strTempFilePath.Append(_T("\\"));
			}
			strTempFilePath.Append(fd.cFileName);

			bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

			if( bIsDirectory
				&& (wcscmp(fd.cFileName, _T("."))==0 || wcscmp(fd.cFileName, _T(".."))==0)) 
			{                
				bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
				continue;
			}

			if(bIsDirectory) //是目录
			{	
				if (bFullPath == FALSE)	//是否需要全路径
				{
					vec_folder.Add(fd.cFileName);
				}
				else
				{
					vec_folder.Add(strTempFilePath);
				}
				
				
				if (bRecursion ==TRUE)
				{
					DoFileFolder(strTempFilePath,vec_folder, bRecursion,bFullPath);
				}
			}

			bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
		}

		::FindClose(hFindFile);


	}
	catch(...)
	{
		return ;
	}

	return ;
}

void CFileOpt::DoFileEnumeration(LPCTSTR lpPath, BOOL bRecursion, BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData)
{
	//static BOOL s_bUserBreak = FALSE;
	try
	{
		//-------------------------------------------------------------------------
		if(s_bUserBreak) return;

		int len = (int)wcslen(lpPath);
		if(lpPath==NULL || len<=0) return;

		//NotifySys(NRS_DO_EVENTS, 0,0);
		
		CString strFilePath;
		strFilePath = lpPath;
		if (strFilePath.GetAt(len-1) != '\\')
		{
			strFilePath.Append(_T("\\"));
		}
		strFilePath.Append(_T("*"));

		WIN32_FIND_DATA fd;
		HANDLE hFindFile = FindFirstFile(strFilePath.GetBuffer(), &fd);
		if(hFindFile == INVALID_HANDLE_VALUE)
		{
			::FindClose(hFindFile); return;
		}


		CString strTempFilePath;; BOOL bUserReture=TRUE; BOOL bIsDirectory;

		BOOL bFinish = FALSE;
		while(!bFinish)
		{
			strTempFilePath = lpPath;
			if (strTempFilePath.GetAt(len-1) != '\\')
			{
				strTempFilePath.Append(_T("\\"));
			}
			strTempFilePath.Append(fd.cFileName);

			bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

			//如果是.或..
			if( bIsDirectory
				&& (wcscmp(fd.cFileName, _T("."))==0 || wcscmp(fd.cFileName, _T(".."))==0)) 
			{                
				bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
				continue;
			}
			
			if(pFunc && bEnumFiles!=bIsDirectory)
			{	
				LARGE_INTEGER large;
				large.LowPart = fd.nFileSizeLow;
				large.HighPart = fd.nFileSizeHigh;

				if (((USERDATA*)pUserData)->iType == IECLEAN_HISTORY)
				{ // 过滤 suggestedsites.dat 这个文件
					CString strFileName;
					strFileName = fd.cFileName;
					strFileName.MakeLower();
					if (strFileName == L"suggestedsites.dat")
					{
						bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
						continue;
					}
				}

				bUserReture = pFunc(strTempFilePath.GetBuffer(), pUserData,large);
				if(bUserReture==FALSE)
				{
					s_bUserBreak = TRUE; ::FindClose(hFindFile); return;
				}
			}

			//NotifySys(NRS_DO_EVENTS, 0,0);

			if(bIsDirectory && bRecursion) //是子目录
			{
				if (((USERDATA*)pUserData)->iType == WINSHISTORY_RECENTDOCFILE)
				{
					CString strFileName;
					strFileName = fd.cFileName;
					strFileName.MakeLower();
					if (strFileName == L"automaticdestinations" || strFileName == L"customdestinations")
					{
						bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
						continue;
					}
				}
				DoFileEnumeration(strTempFilePath.GetBuffer(), bRecursion, bEnumFiles, pFunc, pUserData);
			}

			bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
		}

		::FindClose(hFindFile);

		//-------------------------------------------------------------------------
	}
	catch(...)
	{
		/*ASSERT(0); */return; 
	}
}

void CFileOpt::DoFileEnumeration(LPCTSTR lpPath,CSimpleArray<CString>& vec_wildCard ,BOOL bRecursion, BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData)
{	

	try
	{
		if(s_bUserBreak) return;
		int len = (int)wcslen(lpPath);
		if(lpPath==NULL || len<=0) return;
		
	
		//NotifySys(NRS_DO_EVENTS, 0,0);
		//-------------------------------------------------------------------------
		//枚举通配符文件
		for (int i=0; i<vec_wildCard.GetSize();i++)
		{

			CString strFilePath;
			strFilePath = lpPath;
			if (strFilePath.GetAt(len-1) != '\\')
			{
				strFilePath.Append(_T("\\"));
			}
			strFilePath.Append(vec_wildCard[i].GetBuffer());

			WIN32_FIND_DATA fd;
			HANDLE hFindFile = FindFirstFile(strFilePath.GetBuffer(), &fd);
			if(hFindFile == INVALID_HANDLE_VALUE)
			{
				::FindClose(hFindFile); continue;
			}

			CString tempPath; BOOL bUserReture=TRUE; BOOL bIsDirectory;

			BOOL bFinish = FALSE;
			while(!bFinish)
			{
				bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
				//如果是文件
				if(pFunc && (FALSE==bIsDirectory))
				{	
					LARGE_INTEGER large;
					large.LowPart = fd.nFileSizeLow;
					large.HighPart = fd.nFileSizeHigh;
					
					CString strFileFullPath;
					strFileFullPath.Append(lpPath);

					int iLen = strFileFullPath.GetLength();
					if (strFileFullPath.GetAt(len -1)!= '\\')
					{
						strFileFullPath.Append(_T("\\"));
					}
	
					strFileFullPath.Append(fd.cFileName);

					bUserReture = pFunc(strFileFullPath.GetBuffer(), pUserData,large);
					if(bUserReture==FALSE)
					{
						s_bUserBreak = TRUE; ::FindClose(hFindFile); return;
					}
				}

				bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
				
			}
			FindClose(hFindFile);

		}

		//-------------------------------------------------------------------------
		try
		{
			//-------------------------------------------------------------------------
			if(s_bUserBreak) return;

			int len = (int)wcslen(lpPath);
			if(lpPath==NULL || len<=0) return;

			//NotifySys(NRS_DO_EVENTS, 0,0);
			
			CString strFilePath;
			strFilePath = lpPath;
			if (strFilePath.GetAt(len-1)!= '\\')
			{
				strFilePath.Append(_T("\\"));
			}
			strFilePath.Append(_T("*"));
			


			WIN32_FIND_DATA fd;
			HANDLE hFindFile = FindFirstFile(strFilePath.GetBuffer(), &fd);
			if(hFindFile == INVALID_HANDLE_VALUE)
			{
				::FindClose(hFindFile); return;
			}

			CString strTempFilePath;; BOOL bUserReture=TRUE; BOOL bIsDirectory;

			BOOL bFinish = FALSE;
			while(!bFinish)
			{	

				strTempFilePath = lpPath;
				if (strTempFilePath.GetAt(len-1) != '\\')
				{
					strTempFilePath.Append(_T("\\"));
				}
				strTempFilePath.Append(fd.cFileName);
	
				bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

				//如果是.或..
				if( bIsDirectory
					&& (wcscmp(fd.cFileName, _T("."))==0 || wcscmp(fd.cFileName, _T(".."))==0)) 
				{                
					bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
					continue;
				}

				//if(pFunc && bEnumFiles!=bIsDirectory)
				//{	
				//	LARGE_INTEGER large;
				//	large.LowPart = fd.nFileSizeLow;
				//	large.HighPart = fd.nFileSizeHigh;

				//	bUserReture = pFunc(tempPath, pUserData,large);
				//	if(bUserReture==FALSE)
				//	{
				//		s_bUserBreak = TRUE; ::FindClose(hFindFile); return;
				//	}
				//}

				//NotifySys(NRS_DO_EVENTS, 0,0);

				if(bIsDirectory && bRecursion) //是子目录
				{
					DoFileEnumeration(strTempFilePath.GetBuffer(),vec_wildCard ,bRecursion, bEnumFiles, pFunc, pUserData);
				}

				bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
			}

			::FindClose(hFindFile);

			//-------------------------------------------------------------------------
		}
		catch(...)
		{
			/*ASSERT(0); */return; 
		}
		


	}
	catch(...)
	{
		return ;
	}
}

void CFileOpt::DoFileEnumerationEx(LPCTSTR lpPath,CSimpleArray<FILEEXTS_IN>& vec_wildCard ,BOOL bRecursion, BOOL bEnumFiles, EnumerateFuncEx pFunc, void* pUserData)
{	
	
	LARGE_INTEGER large;
	large.HighPart =0;
	large.LowPart =0;
	
	//用于扫描路径显示
#define PATHMAX__ 2048
	TCHAR szBuf[PATHMAX__];
	wmemset(szBuf,0,PATHMAX__);
	if (NULL==GetShortPathName(lpPath,szBuf,PATHMAX__))
	{
		pFunc(SCANING(FILEGARBAGE_EXTS),lpPath,pUserData,large);
	}
	else
	{
		pFunc(SCANING(FILEGARBAGE_EXTS),szBuf,pUserData,large);
	}

	try
	{	CString strTmp;

		if(s_bUserBreak) return;
		int len = (int)wcslen(lpPath);
		if(lpPath==NULL || len<=0) return;
		

		//NotifySys(NRS_DO_EVENTS, 0,0);
		//-------------------------------------------------------------------------
		//枚举通配符文件

		for (int i=0;i<vec_wildCard.GetSize();i++)
		{

			CString strFilePath;
			strFilePath = lpPath;
			if (strFilePath.GetAt(len-1) != '\\')
			{
				strFilePath.Append(_T("\\"));
			}
			strFilePath.Append(vec_wildCard[i].strFileExts);
			
			
			WIN32_FIND_DATA fd;
			HANDLE hFindFile = FindFirstFile(strFilePath, &fd);
			if(hFindFile == INVALID_HANDLE_VALUE)
			{	
				::FindClose(hFindFile); 
		
				continue;
			}
			
			CString strFile = fd.cFileName;
			
			int iLen = strFile.GetLength();

			int iExtLen = vec_wildCard[i].strFileExts.GetLength();
		
			if ((vec_wildCard[i].strFileExts.GetAt(iExtLen-1) != 
				strFile.GetAt(iLen-1))
				)
			{	
				::FindClose(hFindFile);
				continue;
			}
			

			CString tempPath; BOOL bUserReture=TRUE; BOOL bIsDirectory;

			BOOL bFinish = FALSE;
			while(!bFinish)
			{
				bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
				//如果是文件
				if(pFunc && FALSE==bIsDirectory)
				{	

					LARGE_INTEGER large;
					large.LowPart = fd.nFileSizeLow;
					large.HighPart = fd.nFileSizeHigh;

					CString strFileFullPath;
					strFileFullPath.Append(lpPath);
					
					int iLen = strFileFullPath.GetLength();
					if(strFileFullPath.GetAt(iLen-1)!='\\')
					{
						strFileFullPath.Append(_T("\\"));
					}

					strFileFullPath.Append(fd.cFileName);
					
					//CString str;
					//str.Format(_T("扫描:%s\n"),strFilePath);
					//OutputDebugString(str); 
					
					//返回正在扫描的路径



					bUserReture = pFunc(vec_wildCard[i].iType,strFileFullPath, pUserData,large);
					if(bUserReture==FALSE)
					{
						s_bUserBreak = TRUE; 
						::FindClose(hFindFile); 
						return;
					}
				}

				bFinish = (FindNextFile(hFindFile, &fd) == FALSE);

			}

			::FindClose(hFindFile);

		}


		//-------------------------------------------------------------------------
		try
		{

			if(s_bUserBreak) return;

			int len = (int)wcslen(lpPath);
			if(lpPath==NULL || len<=0) return;

			//NotifySys(NRS_DO_EVENTS, 0,0);

			CString strFilePath;
			strFilePath = lpPath;
			if (strFilePath.GetAt(len-1) != '\\')
			{
				strFilePath.Append(_T("\\"));
			}
			strFilePath.Append(_T("*"));


			WIN32_FIND_DATA fd;
			HANDLE hFindFile = FindFirstFile(strFilePath, &fd);
			if(hFindFile == INVALID_HANDLE_VALUE)
			{
				::FindClose(hFindFile);
				return;
			}


			CString strTempFilePath;; BOOL bUserReture=TRUE; BOOL bIsDirectory;
			
			BOOL bFinish = FALSE;
			while(!bFinish)
			{	

				strTempFilePath = lpPath;
				if (strTempFilePath.GetAt(len-1) != '\\')
				{
					strTempFilePath.Append(_T("\\"));
				}
				strTempFilePath.Append(fd.cFileName);

				bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

				//如果是.或..
				if( bIsDirectory
					&& (wcscmp(fd.cFileName, _T("."))==0 || wcscmp(fd.cFileName, _T(".."))==0)) 
				{                
					bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
					continue;
				}

				//if(pFunc && bEnumFiles!=bIsDirectory)
				//{	
				//	LARGE_INTEGER large;
				//	large.LowPart = fd.nFileSizeLow;
				//	large.HighPart = fd.nFileSizeHigh;

				//	bUserReture = pFunc(tempPath, pUserData,large);
				//	if(bUserReture==FALSE)
				//	{
				//		s_bUserBreak = TRUE; ::FindClose(hFindFile); return;
				//	}
				//}

				//NotifySys(NRS_DO_EVENTS, 0,0);

				if(bIsDirectory && bRecursion) //是子目录
				{
					DoFileEnumerationEx(strTempFilePath,vec_wildCard ,bRecursion, bEnumFiles, pFunc, pUserData);
				}



				bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
			}

			::FindClose(hFindFile);

			//-------------------------------------------------------------------------
		}
		catch(...)
		{	 

			/*ASSERT(0); */return; 
		}



	}
	catch(...)
	{
		return ;
	}

}

BOOL CFileOpt::ConvetPath(CString& strSourcPath)
{	
	if (strSourcPath == _T(""))
	{
		return TRUE;
	}

	try
	{
		if(strSourcPath.GetAt(0) == '%')
		{	
			int iEnd = strSourcPath.Find('%',1);
			CString strEnvPath = strSourcPath.Mid(1,iEnd-1);
			CString strPath = strSourcPath.Mid(iEnd+2);

			wchar_t* pEnv;
			pEnv = _wgetenv(strEnvPath.GetBuffer());
			if (pEnv!=NULL)
			{
				strEnvPath = pEnv;
				int iLen = strEnvPath.GetLength();
				if (strEnvPath.GetAt(iLen-1)!='\\')
				{
					strEnvPath.Append(_T("\\"));
				}

				strEnvPath.Append(strPath.GetBuffer());
				strSourcPath = strEnvPath;
			}

		}
	}
	catch(...)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CFileOpt::DeleteDirectory(LPCTSTR *DirName)
{
	//CFileFind tempFind;
	//CString strFileFind;
	//strFileFind.Format(_T("%s\\*.*"),DirName);

	//BOOL IsFinded=(BOOL)tempFind.FindFile(strFileFind.GetBuffer());
	//while(IsFinded)
	//{
	//	IsFinded=(BOOL)tempFind.FindNextFile();
	//	if(!tempFind.IsDots())
	//	{	
	//		CString strFountFileName;
	//		strFountFileName.Append(tempFind.GetFileName().GetBuffer());

	//		if(tempFind.IsDirectory())
	//		{	
	//			CString strDir;
	//			strDir.Format(_T("%s\\%s"),DirName,strFountFileName.GetBuffer());
	//			DeleteDirectory(strDir.GetBuffer());
	//		}
	//		else
	//		{
	//			CString strDir;
	//			strDir.Format(_T("%s\\%s"),DirName,strFountFileName.GetBuffer());
	//			DeleteFile(strDir.GetBuffer());
	//		}
	//	}
	//}
	//tempFind.Close();

	//if(!RemoveDirectory(DirName))
	//{
	//	//删除失败
	//	return FALSE;
	//}

	return TRUE;
}

void CFileOpt::DeleteEmptyDirectories(LPCTSTR dir)
{

	WIN32_FIND_DATA finder;
	HANDLE hFileFind;

	CString strSearch;
	strSearch = dir;
	int iLen = strSearch.GetLength();
	
	if (iLen<=0)
		return ;

	if (strSearch.GetAt(iLen) != '\\' )
	{
		strSearch.Append(_T("\\"));
	}
	strSearch.Append(_T("*.*"));


	hFileFind = FindFirstFile(strSearch, &finder);

	if (hFileFind != INVALID_HANDLE_VALUE)
	{
		do
		{	
			CString strPath;
			strPath = dir;
			int iLen = strPath.GetLength();
			if (iLen<=0)
			{
				FindClose(hFileFind);
				return ;
			}

			if (strPath.GetAt(iLen) != '\\' )
			{
				strPath.Append(_T("\\"));
			}
			strPath.Append(finder.cFileName);


			if ((finder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				&& wcscmp(finder.cFileName, _T("."))
				&& wcscmp(finder.cFileName, _T("..")))
			{
				
				CString strSubDir;
				strSubDir.Append(strPath);
				strSubDir.Append(_T("\\"));

				DeleteEmptyDirectories(strSubDir);
				// AfxMessageBox(strSubDir);
				RemoveDirectory(strPath);
			}

		} while (FindNextFile(hFileFind, &finder) != 0);

		FindClose(hFileFind);
	}
} 