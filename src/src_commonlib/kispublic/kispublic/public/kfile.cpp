#include "stdafx.h"
#include "kfile.h"
#include "kstringex.h"
#include "shellapi.h"
#pragma message("linking version.lib for kislive_common.lib")
#pragma comment(lib, "Version.lib")


NS_BEGIN(KIS)
NS_BEGIN(IO)
NS_BEGIN(KFile)

int KFile::FileExistsA( LPCSTR lpFilePath )
{
	KWString strFilePath = lpFilePath;
	return FileExistsW(strFilePath.c_str());
}

int KFile::FileExistsW( LPCWSTR lpFilePath )
{
	BOOL bFileExist = FALSE;
	if (lpFilePath != NULL)
	{
		DWORD dwState = ::GetFileAttributesW(lpFilePath);
		if ((dwState != ((DWORD)-1)))
			bFileExist = TRUE;
	}

	return bFileExist;
}

int KFile::GetFileSizeA( IN LPCSTR lpStrFilePath, OUT size_t& uFileSize )
{
	KWString strFilePath = lpStrFilePath;
	return GetFileSizeW(strFilePath.c_str(), uFileSize);
}

int KFile::GetFileSizeW( IN LPCWSTR lpStrFilePath, OUT size_t& uFileSize )
{
	size_t size = INVALID_FILE_SIZE;
	HANDLE	hFile = ::CreateFileW(
									lpStrFilePath,
									FILE_READ_ATTRIBUTES,
									FILE_SHARE_READ,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		size = ::GetFileSize(hFile, NULL);
		::CloseHandle(hFile);
	}

	if (size == INVALID_FILE_SIZE)
		return FALSE;

	uFileSize = size;

	return TRUE;
}

int KFile::DeleteFileA( IN LPCSTR lpFilePath, IN BOOL bDeleteReboot /*= TRUE*/ )
{
	KWString strFilePath = lpFilePath;
	return DeleteFileW(strFilePath.c_str(), bDeleteReboot);
}

int KFile::DeleteFileW( IN LPCWSTR lpFilePath, IN BOOL bDeleteReboot /*= TRUE*/ )
{

	SHFILEOPSTRUCT sf = {0};
	if (lpFilePath == NULL)		return FALSE;


	int nRetCode = FileExistsW(lpFilePath);
	if(!nRetCode)	return FALSE;

	//如果需要删除的是文件夹，则删除该文件夹
// 	nRetCode = KISUP::IO::Directory::IsDirExist(lpStrFilePath);
// 	if (nRetCode)
// 		KISUP::IO::Directory::DeleteDir(lpStrFilePath);

	::SetFileAttributes(lpFilePath, FILE_ATTRIBUTE_NORMAL);
	nRetCode = ::DeleteFileW(lpFilePath);
	if (!nRetCode && bDeleteReboot)
		nRetCode = ::MoveFileExW(lpFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	
	return nRetCode;
}

int KFile::MoveFileRebootA( IN LPCSTR lpFilePath )
{
	KWString strFilePath = lpFilePath;
	return MoveFileRebootW(strFilePath.c_str());
}

int KFile::MoveFileRebootW( IN LPCWSTR lpFilePath )
{
	int nRetCode = ::MoveFileExW(lpFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	return nRetCode;
}


int KFile::CopyFileA( IN LPCSTR lpFilePathSrc, IN LPCSTR lpFilePathDes, IN BOOL bFailIfExist )
{
	KWString strFilePathSrc = lpFilePathSrc;
	KWString strFilePathDes = lpFilePathDes;
	return CopyFileW(strFilePathSrc.c_str(), strFilePathDes.c_str(), bFailIfExist);
}

int KFile::CopyFileW( IN LPCWSTR lpFilePathSrc, IN LPCWSTR lpFilePathDes, IN BOOL bFailIfExist )
{

	if (lpFilePathDes == NULL || lpFilePathSrc == NULL)
		return FALSE;

	int nRetCode = FileExistsW(lpFilePathSrc);
	if (!nRetCode)		return FALSE;

	::SetFileAttributes(lpFilePathDes, NULL);

	nRetCode = ::CopyFile(lpFilePathSrc, lpFilePathDes, bFailIfExist);
	if (!nRetCode)		return FALSE;

	return TRUE;
}



int KFile::GetFileVersionDWORDA( IN LPCSTR lpFileName, IN OUT DWORD *pdwFileVersionMS, IN OUT DWORD *pdwFileVersionLS )
{
	KWString strFilePath = lpFileName;
	return GetFileVersionDWORDW(strFilePath.c_str(), pdwFileVersionMS, pdwFileVersionLS);
}

int KFile::GetFileVersionDWORDW( IN LPCWSTR lpFileName, IN OUT DWORD *pdwFileVersionMS, IN OUT DWORD *pdwFileVersionLS )
{
	int nResult             = false;
	int nRetCode            = false;
	DWORD dwHandle          = 0;
	DWORD dwFileInfoSize    = 0;

	VS_FIXEDFILEINFO *pFixFileInfo	= NULL;
	char *pszFileInfo				= NULL;
	UINT uLen						= 0;

	if (ValidPtrRange(lpFileName))				return FALSE;
	if (ValidPtrRange(pdwFileVersionMS))		return FALSE;
	if (ValidPtrRange(pdwFileVersionLS))		return FALSE;

	*pdwFileVersionMS = 0;
	*pdwFileVersionLS = 0;

	dwFileInfoSize = ::GetFileVersionInfoSizeW((WCHAR *)lpFileName, &dwHandle);
	if (dwFileInfoSize <= 0)	return	FALSE;
	//KAV_PROCESS_ERROR(dwFileInfoSize);

	pszFileInfo = new char[dwFileInfoSize];
	if (ValidPtrRange(pszFileInfo))		return FALSE;

	nRetCode = ::GetFileVersionInfoW((WCHAR *)lpFileName, dwHandle, dwFileInfoSize, pszFileInfo);
	if (!nRetCode)		return FALSE;

	nRetCode = ::VerQueryValueW(pszFileInfo, L"\\", (LPVOID *)&pFixFileInfo, &uLen);
	if (!nRetCode)		return FALSE;
	
	if (uLen > 0)
	{
		*pdwFileVersionMS = pFixFileInfo->dwProductVersionMS;
		*pdwFileVersionLS = pFixFileInfo->dwProductVersionLS;
	}    

	if (pszFileInfo != NULL)
	{
		delete[] pszFileInfo;
		pszFileInfo = NULL;
	}

	return TRUE;
}

int KFile::GetFileVersionStrA( IN LPCSTR lpFilePath, IN OUT LPSTR lpVersion, IN int& nVersionSize )
{
	KWString strFilePath = lpFilePath;
	if (lpVersion == NULL)
		return GetFileVersionStrW(strFilePath.c_str(), NULL, nVersionSize);
	else
	{
		wchar_t* pSzFilePath = new wchar_t[nVersionSize];
		if (pSzFilePath != NULL)
			::ZeroMemory(pSzFilePath, nVersionSize * sizeof(wchar_t)/sizeof(char));
		
		BOOL bRet = GetFileVersionStrW(strFilePath.c_str(), pSzFilePath, nVersionSize);
		if (!bRet)
		{
			KAString strFilePathA = pSzFilePath;

#if _MSC_VER <= 1200 // vc6.0
			strcpy(lpVersion, strFilePathA.c_str());
#else
			strcpy_s(lpVersion, nVersionSize, strFilePathA.c_str());
#endif
		}
		
		DeletePointer(pSzFilePath);

		return bRet;
	}

	return FALSE;
}

int _GetVersionStringFromInt(IN int nMajorVersion, IN int nMinorVersion, IN OUT LPTSTR szVersion, IN int nVersionLen)
{
	int nResult  = false;
	int nLen    = 0;
	TCHAR szVersionBuffer[MAX_PATH] = { 0 };
	DWORD dw1 = 0, dw2 = 0, dw3 = 0, dw4 = 0;

	if (nMajorVersion < 0)		return FALSE;
	if (nMinorVersion < 0)		return FALSE;
	if (nVersionLen < 0)		return FALSE;
	if (ValidPtrRange(szVersion))	return FALSE;

	dw1 = nMajorVersion >> 16;
	dw2 = nMajorVersion & 0x0FFFF;
	dw3 = nMinorVersion >> 16;
	dw4 = nMinorVersion & 0x0FFFF;

#if _MSC_VER <= 1200 // vc6.0
	_stprintf(szVersionBuffer, _T("%04d.%02d.%02d.%02d"), dw1, dw2, dw3, dw4);
#else
	_stprintf_s(szVersionBuffer, MAX_PATH, _T("%04d.%02d.%02d.%02d"), dw1, dw2, dw3, dw4);
#endif

	nLen = (int)_tcslen(szVersionBuffer);
	if (nLen > nVersionLen)		return FALSE;

#if _MSC_VER <= 1200 // vc6.0
	wcscpy(szVersion, szVersionBuffer);
#else	
	wcscpy_s(szVersion, nVersionLen, szVersionBuffer);
#endif 

	return TRUE;
}

int KFile::GetFileVersionStrW( IN LPCWSTR lpFilePath, IN OUT LPWSTR lpVersion, IN int& nVersionSize )
{
	DWORD dwFileVersionMS = 0; 
	DWORD dwFileVersionLS = 0;

	if (ValidPtrRange(lpFilePath))		return FALSE;

	int nRetCode = GetFileVersionDWORDW(lpFilePath, &dwFileVersionMS, &dwFileVersionLS);
	if (!nRetCode)		return FALSE;

	nRetCode = _GetVersionStringFromInt(dwFileVersionMS, dwFileVersionLS, lpVersion, nVersionSize);
	
	return nRetCode;
}

NS_END(KFile)
NS_END(IO)
NS_END(KIS)




