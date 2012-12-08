#include "StdAfx.h"
#include "Iptypes.h"
#include "Iphlpapi.h"
#pragma comment(lib, "Iphlpapi.lib")
#include "common.h"
using namespace BEIKESAFEEXAM;
#include "Wininet.h"
#pragma comment(lib, "wininet.lib")
#include "strsafe.h"
#include <winmod/wintrustverifier.h>
using namespace WinMod;


CCommon::CCommon(void)
{
}

CCommon::~CCommon(void)
{
}

BOOL CCommon::NetGatewayExists()
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;

	BOOL bExists = FALSE;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
	if(pAdapterInfo)
	{
		dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen);
		if(dwRetVal == ERROR_BUFFER_OVERFLOW )
		{
			free(pAdapterInfo);

			pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
			if(pAdapterInfo)
				dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen);
		}

		if( dwRetVal == NO_ERROR)
		{
			pAdapter = pAdapterInfo;
			while (pAdapter)
			{
				if(pAdapter->GatewayList.IpAddress.String[0])
				{
					bExists = TRUE;
					break;
				}
				pAdapter = pAdapter->Next;
			}
		}

		free(pAdapterInfo);
	}

	return bExists;
}

BOOL CCommon::HaveInternetConnection()
{
	DWORD dwFlag;

	//	如果函数返回FALSE,则肯定没有连接
	if( InternetGetConnectedState(&dwFlag, 0) )
	{
		//	必须有下列任意一种连接，才认为是有连接
		//	因为INTERNET_CONNECTION_CONFIGURED被置位也可能是没有连接
		if( (dwFlag&INTERNET_CONNECTION_LAN) ||
			(dwFlag&INTERNET_CONNECTION_MODEM) ||
			(dwFlag&INTERNET_CONNECTION_PROXY) )
			return TRUE;
	}

	return NetGatewayExists();
}

void CCommon::OutPutLogToFileA(LPCSTR pszFormat, ...)
{
	USES_CONVERSION;
	va_list args;
	va_start(args, pszFormat);
	char szBuf[1000] = {'\0'};
	StringCbVPrintfA(szBuf, sizeof(szBuf), (pszFormat), args);

	char pszModFileName[1024] = {0};
	::GetModuleFileNameA(NULL, pszModFileName, sizeof(pszModFileName));
	PathAppendA(pszModFileName, "..\\DebugLog.log");
	FILE *pFile = NULL;//fopen(pszModFileName, ("a+b"));
	if (0 != fopen_s(&pFile, pszModFileName, "a+b"))
		return;

	if(pFile == NULL)
		return;
	SYSTEMTIME timeCur;
	GetLocalTime(&timeCur);
	char szTime[100] = {0};
	StringCbPrintfA(szTime, sizeof(szTime), ("%d-%d-%d %d:%d:%d:%d  "), timeCur.wYear, timeCur.wMonth, timeCur.wDay,timeCur.wHour, timeCur.wMinute, timeCur.wSecond, timeCur.wMilliseconds);

	fwrite((szTime), strlen(szTime), 1, pFile);
	fwrite((szBuf),strlen(szBuf), 1, pFile);
	fwrite(("\r\n"), 2, 1,pFile);
	fclose(pFile);
}

void CCommon::OutPutLogToFileW(LPCWSTR pszFormat, ...)
{
	USES_CONVERSION;
	va_list args;
	va_start(args, pszFormat);
	char szBuf[1000] = {'\0'};
	StringCbVPrintfA(szBuf, sizeof(szBuf), W2A(pszFormat), args);

// 	CString strPath;
// 	CAppPath::Instance().GetLeidianLogPath(strPath, TRUE);
	WCHAR pszModuleFileName[1024] = {0};
	::GetModuleFileName(NULL, pszModuleFileName, sizeof(pszModuleFileName));
	//strPath += TEXT("\\IEFix.log");
	PathAppend(pszModuleFileName, TEXT("..\\DebugLog.log"));
	FILE *pFile = fopen(W2A(pszModuleFileName), ("a+b"));
	//strPath.ReleaseBuffer(MAX_PATH);

	if(pFile == NULL)
		return;
	SYSTEMTIME timeCur;
	GetLocalTime(&timeCur);
	char szTime[100] = {0};
	StringCbPrintfA(szTime, sizeof(szTime), ("%d-%d-%d %d:%d:%d:%d  "), timeCur.wYear, timeCur.wMonth, timeCur.wDay,timeCur.wHour, timeCur.wMinute, timeCur.wSecond, timeCur.wMilliseconds);

	fwrite((szTime), strlen(szTime), 1, pFile);
	fwrite((szBuf),strlen(szBuf), 1, pFile);
	fwrite(("\r\n"), 2, 1,pFile);
	fclose(pFile);
}

void CCommon::OutPutLogToDebugView(LPCSTR pszFormat, ...)
{
	USES_CONVERSION;
	va_list args;
	va_start(args, pszFormat);
	char szBuf[1000] = {'\0'};
	StringCbVPrintfA(szBuf, sizeof(szBuf), (pszFormat), args);

	OutputDebugStringA(szBuf);
}

BOOL CCommon::IsWin64()
{
	BOOL bRet = FALSE;
	static HMODULE hKernel32 = NULL;
	if( !hKernel32 )
		hKernel32 = ::LoadLibrary(TEXT("Kernel32.DLL"));
	if( !hKernel32 )
		return FALSE;

	typedef BOOL (WINAPI *FunctionIsWow64Process)(HANDLE hProcess, PBOOL Wow64Process);
	FunctionIsWow64Process pfnIsWow64Process = NULL;
	pfnIsWow64Process = (FunctionIsWow64Process)GetProcAddress(hKernel32, "IsWow64Process");

	if (NULL == pfnIsWow64Process)
		return FALSE;

	HANDLE hCurrentProcess = GetCurrentProcess();
	pfnIsWow64Process(hCurrentProcess, &bRet);
	return bRet;
}

int CCommon::VerifyFile(CAtlArray<CString>& arrFile)
{
	DWORD dwWinTrust = 0;
	size_t nCount = arrFile.GetCount();
	CWinTrustVerifier verifier;
	verifier.TryLoadDll();
	CString strFile;
	HRESULT hr = S_OK;
	for (size_t i = 0; i < nCount; i++)
	{
		strFile = arrFile[i];
		dwWinTrust = verifier.VerifyFile(strFile, NULL);
		if (dwWinTrust <= 0x0000FFFF)
			hr = AtlHresultFromWin32(dwWinTrust);
		else
			hr = dwWinTrust;

		if ( FAILED(hr))
		{
			CKSafeExamLog::GetLogPtr()->WriteLog(L"VerifyFile Failed: File = %s, hr = 0x%x", strFile, hr);
			dwWinTrust = -1;
			break;
		}

		strFile.Empty();
	}

	return dwWinTrust;
}

int CCommon::ParsexmlCmdLine(LPCTSTR pszCmdLine, CAtlArray<CString>& arrStr)
{
	int nRet = -1;
	if (NULL == pszCmdLine)
		return nRet;

	arrStr.RemoveAll();
	LPTSTR _pszCmdLine = StrDupW(pszCmdLine);
	if (NULL == _pszCmdLine)
		return nRet;

	LPTSTR pValue = _pszCmdLine;

	LPTSTR pos = NULL;
	LPTSTR NewPos = NULL;
	NewPos = StrStr(_pszCmdLine, TEXT("\","));
	while(NULL != NewPos && _tcslen(NewPos) > 2)
	{
		pos = NewPos+2;
		*NewPos = 0;
		_pszCmdLine += 1;
		arrStr.Add((_pszCmdLine));
		_pszCmdLine = pos;
		NewPos = StrStr(_pszCmdLine, TEXT("\","));
	}
	if (NULL != _pszCmdLine && _tcslen(_pszCmdLine) > 1)
	{
		_pszCmdLine += 1;
		pos = StrRChr(_pszCmdLine, NULL, TEXT('\"'));
		if (NULL != pos)
			*pos = 0;
		arrStr.Add(_pszCmdLine);
	}

	nRet = 0;
	if (NULL != pValue)
	{
		LocalFree(pValue);
		pValue = NULL;
	}
	return nRet;
}

/*
int CCommon::ConvertKWSIDToExamID(int nKwsID)
{
	int nExamID = -1;
	switch(nKwsID)
	{
	case 0:
		{//检测浏览器主页是否被恶意篡改指向异常网址
			nExamID = 11;
		}
		break;
	case 1:
		{//检测是否存在流氓软件推广的异常桌面图标
			nExamID = 40;
		}
		break;
	case 2:
		{//检测IE浏览器快捷方式是否被恶意篡改
			nExamID = 12;
		}
		break;
	case 3:
		{//检测常用桌面图标是否被恶意篡改
			nExamID = 39;
		}
		break;
	case 4:
		{//检测Internet选项和IE浏览器功能是否存在异常
			nExamID = 21;
		}
		break;
	case 5:
		{//检测浏览器默认搜索引擎是否存在异常
			nExamID = 16;
		}
		break;
	case 6:
		{//检测非IE的其他浏览器的快捷方式是否被恶意篡改
			nExamID = 38;
		}
		break;
	case 7:
		{//检测收藏夹是否存在恶意推广的网址
			nExamID = 36;
		}
		break;
	case 8:
		{//检测http协议及网页文件关联是否正常
			nExamID = 22;
		}
		break;
	case 9:
		{//检测是否存在其他的系统异常问题
			nExamID = 37;
		}
		break;
	case 10:
		{
		}
		break;
	case 11:
		{
		}
		break;
	case 12:
		{
		}
		break;
	}
	return nExamID;
}*/
