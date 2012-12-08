#include "stdafx.h"
#include "KPath.h"
#include "kstringex.h"
#include "shellapi.h"


NS_BEGIN(KIS)
NS_BEGIN(IO)
NS_BEGIN(KPath)


int KPath::GetKafeOemFilePathA( LPSTR lpFilePath, IN OUT size_t& nSize )
{
	return CallOutA<int, LPWSTR, size_t&>(GetKafeOemFilePathW, lpFilePath, nSize);
}

int _GetFirstFileDirctory(IN std::wstring strOemPath, OUT std::wstring& strOemFilePath)
{
	BOOL bRet = FALSE;
	KWString strPathTemp;
	strOemFilePath = _T("");
	strPathTemp.assign(strOemPath);
	strPathTemp.append(L"*.*");
	WIN32_FIND_DATAW FindFileData = { 0 };

	{
		HANDLE hFind = ::FindFirstFile(strPathTemp.c_str(), &FindFileData);
		if (INVALID_HANDLE_VALUE != hFind)
		{
			do 
			{
				if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					&& _tcscmp(FindFileData.cFileName, L".") && _tcscmp(FindFileData.cFileName, L".."))
				{
					strPathTemp.Format(_T("%s%s\\"), strOemPath.c_str(), FindFileData.cFileName);
					_GetFirstFileDirctory(strPathTemp, strOemFilePath);
				}

				if(!((FindFileData.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY))
				{
					strPathTemp.Format(_T("%s%s"), strOemPath.c_str(), FindFileData.cFileName);
					strOemFilePath = strPathTemp;
					bRet = TRUE;
					break;
				}

			} while(FindNextFile(hFind, &FindFileData)!=0);

			::FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}

	return bRet;
}

int KPath::GetKafeOemFilePathW( LPWSTR lpFilePath, IN OUT size_t& nSize )
{
	std::wstring strOemPath;
	wchar_t szFilePath[MAX_PATH] = {0};
	DWORD dwRet = ::GetModuleFileName(GetModuleHandle(L"kispublic.dll"), szFilePath, MAX_PATH);
//	if (!CallFunc<int, LPWSTR, size_t&>(GetCurDirectoryW, strOemPath))
//		return FALSE;
	
	KWString strFilePathT = szFilePath;
	strFilePathT.PathRemoveFileSpec();

	strOemPath = strFilePathT;

	strOemPath.append(_T("oem\\"));
	
	std::wstring strOemFilePath;

	_GetFirstFileDirctory(strOemPath, strOemFilePath);

	if (strOemFilePath.empty())
		return FALSE;

	return JudgeAndCopy(strOemFilePath.c_str(), lpFilePath, nSize);
}

int KPath::GetCurDirectoryA( OUT LPSTR lpPath, IN OUT size_t& nSize )
{
	return CallOutA<int, LPWSTR, size_t&>(GetCurDirectoryW, lpPath, nSize);
}

int KPath::GetCurDirectoryW( OUT LPWSTR lpPath, IN OUT size_t& nSize )
{
	KWString strFilePath;
	std::wstring strFilePathW;
	int nRet = CallFunc<int, LPWSTR, size_t&>(GetCurFullPathW, strFilePathW);
	if (!nRet)		return FALSE;

	strFilePath = strFilePathW;
	strFilePath.PathRemoveFileSpec();

	return JudgeAndCopy(strFilePath.c_str(), lpPath, nSize);
}

int KPath::GetCurFullPathA( OUT LPSTR lpPath, IN OUT size_t& nSize )
{
	return CallOutA<int, LPWSTR, size_t&>(GetCurFullPathW, lpPath, nSize);
}

int KPath::GetCurFullPathW( OUT LPWSTR lpPath, IN OUT size_t& nSize )
{
	int nResult = FALSE;
	DWORD dwSize = 0;
	std::wstring strExeFile;
	WCHAR szModuleFileName[MAX_PATH + 1] = { 0 };

	dwSize = ::GetModuleFileNameW(NULL, szModuleFileName, MAX_PATH);
	if (dwSize <= MAX_PATH)
	{
		strExeFile = szModuleFileName;
		nResult = TRUE;
	}
	else 
	{
		if (ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
		{
			DWORD dwReSize = 0;
			std::wstring strBuffer;

			strBuffer.resize(dwSize);		//注意这里不能nSize + 1， 因为std::wstring的长度问题。
			dwReSize = ::GetModuleFileNameW(NULL, const_cast<WCHAR*>(strBuffer.c_str()), dwSize);

			if (dwReSize <= dwSize)
			{
				strExeFile.swap(strBuffer);
				nResult = TRUE;
			}
		} // if ERROR_INSUFFICIENT_BUFFER
	}

	return JudgeAndCopy(strExeFile.c_str(), lpPath, nSize);

}

NS_END(KPath)
NS_END(IO)
NS_END(KIS)





