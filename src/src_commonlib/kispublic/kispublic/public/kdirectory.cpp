#include "stdafx.h"
#include "kdirectory.h"
#include "public.h"


NS_BEGIN(KIS)
NS_BEGIN(IO)
NS_BEGIN(KDirectory)


int _CreateDirW( LPCWSTR lpFilePath )
{
	if (ValidPtrRange(lpFilePath))		return	FALSE;

	if (KIS::IO::KFile::FileExistsW(lpFilePath))		
		return TRUE;
	else	
		return ::CreateDirectoryW(lpFilePath, NULL);
}

int KDirectory::CreateDirCleanFileNameA( IN LPCSTR lpFileFullName )
{
	KWString strFilePath = lpFileFullName;
	return CreateDirCleanFileNameW(strFilePath.c_str());
}

int KDirectory::CreateDirCleanFileNameW( IN LPCWSTR lpFileFullName )
{
	KWString strFilePath = lpFileFullName;
	strFilePath.PathRemoveFileSpec();
	return CreateDirW(strFilePath.c_str());
}

int KDirectory::CreateDirA( IN LPCSTR lpStrFilePath )
{
	KWString strFilePath = lpStrFilePath;
	return CreateDirW(strFilePath.c_str());
}

int KDirectory::CreateDirW( IN LPCWSTR lpStrFilePath )
{
	BOOL bSuccess = FALSE;

	if (ValidPtrRange(lpStrFilePath))	return FALSE;

	bSuccess = _CreateDirW(lpStrFilePath);
	if (!bSuccess)
	{
		KWString strFilePath = lpStrFilePath;
		KWString strParentPath = _T("");
		for(;;)
		{
			int nPos = (int)strFilePath.Find('\\');
			if (nPos > -1)
			{
				strParentPath += strFilePath.Left(nPos + 1);
				if ((int)strFilePath.Find(':') > -1)
				{
					strFilePath = strFilePath.Mid(nPos + 1);
					continue;
				}
				
				strFilePath = strFilePath.Mid(nPos + 1);
				_CreateDirW(strParentPath.c_str());
			}
			else
				break;
		}
	}

	bSuccess = _CreateDirW(lpStrFilePath);

	return bSuccess;
}

int _DeleteDir(IN LPCWSTR lpStrDirPath, IN BOOL bDeleteReboot = TRUE)
{
	int nResult = false;
	int nRetCode = false;

	if (ValidPtrRange(lpStrDirPath))	return FALSE;

	// 传入不合法的文件夹路径，认为执行功能。
	nRetCode = KIS::IO::KFile::FileExistsW(lpStrDirPath);
	if (!nRetCode)		return TRUE;

	::SetFileAttributes(lpStrDirPath, NULL);
	nRetCode = ::RemoveDirectoryW(lpStrDirPath);
	if (!nRetCode && bDeleteReboot)
		KIS::IO::KFile::MoveFileRebootW(lpStrDirPath);

	return nRetCode;	
}


int KDirectory::DeleteDirA( IN LPCSTR lpFolderPath, IN BOOL bDeleteReboot /*= TRUE*/ )
{
	KWString strFolderPath = lpFolderPath;
	return DeleteDirW(strFolderPath.c_str(), bDeleteReboot);
}

int KDirectory::DeleteDirW( IN LPCWSTR lpFolderPath, IN BOOL bDeleteReboot /*= TRUE*/ )
{
	int nRetCode = false;

	KWString strRootPath;
	KWString strTempPath;
	WIN32_FIND_DATAW FindFileData = { 0 };

	if (ValidPtrRange(lpFolderPath))	return FALSE;

	nRetCode = _DeleteDir(lpFolderPath);
	if (nRetCode)		return TRUE;

	strRootPath.assign(lpFolderPath);
	strRootPath.AddPathEnd();

	strTempPath.assign(strRootPath);
	strTempPath.append(L"*.*");

	{
		HANDLE hFind = ::FindFirstFile(strTempPath.c_str(), &FindFileData);
		if (INVALID_HANDLE_VALUE != hFind)
		{
			do 
			{
				if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					&& _tcscmp(FindFileData.cFileName, L".") && _tcscmp(FindFileData.cFileName, L".."))
				{
					strTempPath.Format(_T("%s%s"), strRootPath.c_str(), FindFileData.cFileName);
					// 删除失败里断续执行删除动作。
					nRetCode = DeleteDirW(strTempPath.c_str());
					//KAV_PROCESS_ERROR(nRetCode);
				}

				if(!((FindFileData.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY))
				{
					strTempPath.Format(_T("%s%s"), strRootPath.c_str(), FindFileData.cFileName);
					// 删除失败里断续执行删除动作。
					nRetCode = KIS::IO::KFile::DeleteFileW(strTempPath.c_str());
				}

			} while(FindNextFile(hFind, &FindFileData)!=0);

			::FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}

	return _DeleteDir(lpFolderPath);
}

int KDirectory::CopyDirA( IN LPCSTR lpSrcDir, IN LPCSTR lpDstDir, IN int nFailedIfExist )
{
	KWString strFolderSrc = lpSrcDir;
	KWString strFolderDes = lpDstDir;
	return CopyDirW(strFolderSrc.c_str(), strFolderDes.c_str(), nFailedIfExist);
}

int KDirectory::CopyDirW( IN LPCWSTR lpSrcDir, IN LPCWSTR lpDstDir, IN int nFailedIfExist )
{
	int nResult = false;
	int nRetCode = false;

	WIN32_FIND_DATAW finddata = { 0 };
	HANDLE hFind = NULL;

	nRetCode = KIS::IO::KFile::FileExistsW(lpSrcDir);
	if (!nRetCode)		return FALSE;

	nRetCode = CreateDirW(lpDstDir);
	if (!nRetCode)		return FALSE;

	KWString strSrcTemp = lpSrcDir;
	KWString strDstTemp = lpDstDir;
	strSrcTemp.AddPathEnd();
	strDstTemp.AddPathEnd();

	strSrcTemp.append(L"*.*");

	hFind = ::FindFirstFile(strSrcTemp.c_str(), &finddata);
	if (hFind == INVALID_HANDLE_VALUE)	return FALSE;

	do 
	{
		if (wcscmp(finddata.cFileName, TEXT(".")) == 0 || 
			wcscmp(finddata.cFileName, TEXT("..")) == 0)
			continue;

		strSrcTemp.Format(_T("%s\\%s"), lpSrcDir, finddata.cFileName);
		strDstTemp.Format(_T("%s\\%s"), lpDstDir, finddata.cFileName);

		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CopyDirW(strSrcTemp.c_str(), strDstTemp.c_str(), nFailedIfExist);

			continue;
		}

		if (!nFailedIfExist)
			::SetFileAttributes(strDstTemp.c_str(), NULL);

		::CopyFile(strSrcTemp.c_str(), strDstTemp.c_str(), nFailedIfExist);

	} while (::FindNextFileW(hFind, &finddata));

	if (hFind)
		::FindClose(hFind);

	return TRUE;
}

NS_END(KDirectory)
NS_END(IO)
NS_END(KIS)



