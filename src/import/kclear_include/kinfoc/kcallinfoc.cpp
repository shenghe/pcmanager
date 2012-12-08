#include <windows.h>
#include "KCallInfoc.h"
#include <shlwapi.h>
#include <process.h>
#include "commonfunc.h"

#define defKIS_CFG_VAL_ANTIVIRUS					L"SOFTWARE\\Kingsoft\\KISCommon\\KXEngine"
#define defKLIST_PROGRAMPATH						L"path"

#ifndef ENOENT
#define ENOENT 2
#endif

KCallInfoc::KCallInfoc(void)
{
	m_bIsInit = FALSE;
	m_pSendData = NULL;
	m_pAddData = NULL;
	m_pSend = NULL;
	m_pCheckAndDelFiles = NULL;
}

KCallInfoc::~KCallInfoc(void)
{

}

BOOL KCallInfoc::Init(IN const wchar_t * lpStrFilePath)
{
	std::wstring strFilePath;
	if (lpStrFilePath == NULL)
	{
		BOOL bRet = _GetKInfocPath(strFilePath);
		if (!bRet)
			return FALSE;
	}
	else
		strFilePath = lpStrFilePath;

	m_hMoudle = LoadLibraryW(strFilePath.c_str());
	if (m_hMoudle == NULL)
	{
		int nError = ::GetLastError();
		return FALSE;
	}


	m_pCheckAndDelFiles = (DllCheckAndDelFiles)GetProcAddress(m_hMoudle, "CheckAndDelFiles");
	if (m_pCheckAndDelFiles == NULL)
	{
		goto _exit_;
	}


	m_pSendData = (DllSetPublic)GetProcAddress(m_hMoudle, "SetPublic");
	if (m_hMoudle == NULL)
	{
		goto _exit_;
	}
	m_pSend = (DllSend)GetProcAddress(m_hMoudle, "SendData");
	if (m_pSend == NULL)
	{
		goto _exit_;
	}

	m_pAddData = (DllAddData)GetProcAddress(m_hMoudle, "AddData");
	if (m_pAddData == NULL)
	{
		goto _exit_;
	}

	m_bIsInit = TRUE;

_exit_:
	return m_bIsInit;
}

BOOL KCallInfoc::Unit()
{
	if (m_hMoudle != NULL)
	{
		::FreeLibrary(m_hMoudle);
		m_hMoudle = NULL;
	}

	m_bIsInit = FALSE;
	return TRUE;
}

BOOL KCallInfoc::SetPublicFile( IN const wchar_t* lpStrPublicFileName, IN const wchar_t* lpStrPublicData, IN const wchar_t * lpStrFilePath )
{
	std::wstring strFilePath;
	BOOL bRet = FALSE;

	if (lpStrFilePath == NULL)
	{
		BOOL bRet = _GetKInfocPath(strFilePath);
		if (!bRet)
			goto _exit_;
	}
	else
		strFilePath = lpStrFilePath;

	m_hMoudle = LoadLibraryW(strFilePath.c_str());
	if (m_hMoudle == NULL)
	{
		int nError = ::GetLastError();
		goto _exit_;
	}

	m_pSendData = (DllSetPublic)GetProcAddress(m_hMoudle, "SetPublic");
	if (m_hMoudle == NULL)
	{
		goto _exit_;
	}

	bRet = m_pSendData(lpStrPublicFileName, lpStrPublicData);
_exit_:

	if (m_hMoudle != NULL)
	{
		::FreeLibrary(m_hMoudle);
		m_hMoudle = NULL;
	}
	return bRet;
}


BOOL KCallInfoc::AddData( IN const wchar_t * lpStrActionName, IN const wchar_t * lpStrData, IN const wchar_t * lpStrFilePath )
{
	if (!m_bIsInit)
		if (!Init(lpStrFilePath))
			return FALSE;

	return m_pAddData(lpStrActionName, lpStrData);
}

BOOL KCallInfoc::SendData( enumSendMode enumMode, const wchar_t * lpStrFilePath )
{
	if (!m_bIsInit)
		if (!Init(lpStrFilePath))
			return FALSE;

	return m_pSend(enumMode);
}

void KCallInfoc::CheckAndDelFiles( IN int nNum, IN const wchar_t * lpStrFilePath )
{
	if (!m_bIsInit)
		if (!Init(lpStrFilePath))
			return ;

	m_pCheckAndDelFiles(nNum);
}

BOOL KCallInfoc::_GetKInfocPath( OUT std::wstring& strInfocPath )
{
	HMODULE hModule = ::GetModuleHandle(NULL); 
	if (hModule == NULL)
		return FALSE;

	WCHAR szPath[MAX_PATH] = L"";
	DWORD dwReturn = ::GetModuleFileNameW(hModule, szPath, MAX_PATH);
	if (dwReturn == 0)
		return FALSE;

	int nLen = (int)wcslen(szPath);
	for (int i = nLen - 1; i >= 0; i--)
	{
		if (szPath[i] == '\\')
		{
			szPath[i + 1] = '\0';
			break;
		}
	}

	strInfocPath = szPath;
	strInfocPath += L"report\\kinfoc.dll";

	if (_waccess_s(strInfocPath.c_str(), 0) != ENOENT)
		return TRUE;
	else
		return _CheckFileFromReg(strInfocPath);

	return FALSE;
}

BOOL KCallInfoc::_CheckFileFromReg(OUT std::wstring& strFilePath)
{
	WCHAR szInstallPath[MAX_PATH] = {0};
	if (!_ReadOneRegKey(szInstallPath, MAX_PATH))
		return FALSE;
	
	wcscat_s(szInstallPath, MAX_PATH, L"\\operation\\cas\\");
	wcscat_s(szInstallPath, MAX_PATH, L"kinfoc.dll");

	if (_waccess_s(szInstallPath, 0) == ENOENT)
		return FALSE;

	strFilePath = szInstallPath;
	return TRUE;
}

BOOL KCallInfoc::_ReadOneRegKey( OUT wchar_t * lpszResult, IN long dwOutMaxSize )
{
	BOOL bResult = FALSE;
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey = NULL;
	DWORD dwCount = MAX_PATH;
	DWORD dwType = REG_BINARY;
	LPBYTE pBuf = NULL;
	DWORD dwSize = 0;

	do 
	{
		lRet = ::RegOpenKeyExW(
								HKEY_LOCAL_MACHINE,
								defKIS_CFG_VAL_ANTIVIRUS,
								0,
								KEY_QUERY_VALUE,
								&hKey
								);

		if (lRet != ERROR_SUCCESS)
		{
			lRet = ::RegOpenKeyExW(
									HKEY_CURRENT_USER,
									defKIS_CFG_VAL_ANTIVIRUS,
									0,
									KEY_QUERY_VALUE,
									&hKey
									);
		}

		if (lRet != ERROR_SUCCESS || hKey == NULL)
			break;

		lRet = ::RegQueryValueExW(hKey, defKLIST_PROGRAMPATH, 0, &dwType, NULL, &dwSize);

		if (ERROR_SUCCESS == lRet)
		{
			pBuf = new BYTE[max(dwSize, 4)];
			lRet = ::RegQueryValueExW(hKey, defKLIST_PROGRAMPATH, 0, &dwType, pBuf, &dwSize);

			wcscpy_s(lpszResult, dwOutMaxSize, (WCHAR *)pBuf);
			bResult = TRUE;
		}


	} while(0);

	if (pBuf != NULL)
	{
		delete[] pBuf;
		pBuf = NULL;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = NULL;
	}

	return bResult;
}


BOOL KCallInfoc::AddDataEx(const wchar_t *lpStrActionName, const wchar_t *lpStrData)
{
	std::wstring str;
	wstring tid1;
	wstring tid2;
	wstring PID;
	wstring versiontype;

	if (!m_bIsInit)
	{
		if (!Init())
		{
			return FALSE;
		}
	}

	tid1 = ReadString(L"Channel", L"tid1");
	tid2 = ReadString(L"Channel", L"tid2");
	PID = ReadString(L"Version", L"Version");
	versiontype = ReadString(L"packetid", L"versiontype");

	str += L"tid1=";
	str += tid1;
	str += L"&";
	str += L"tid2=";
	str += tid2;
	str += L"&";
	str += L"PID=";
	str += PID;
	str += L"&";
	str += L"versiontype=";
	str += versiontype;
	str += L"&";
	str += lpStrData;
	AddData(lpStrActionName, str.c_str());

	Unit();

	return TRUE;
}