#include "StdAfx.h"
#include "KRequestHttpFile.h"
#include "KStreamWrapper.h"


// 一个进程内只能有一个WinHttpWrapper对象，不能多次加载winhttp.dll wininet.dll;
WinHttpWrapper g_WinHttpWrapper;
BOOL		   g_bWinHttpInited = FALSE;

BOOL WinHttp_Initialize()
{
	g_bWinHttpInited = g_WinHttpWrapper.Initialize();
	return g_bWinHttpInited;
}

BOOL WinHttp_Uninitialize()
{
	return g_WinHttpWrapper.Uninitialize();
}

BOOL WinHttp_RequestHttpFile(IN LPCWSTR lpszUrl)
{
	BOOL bSuccess = FALSE;

	if (g_bWinHttpInited)
		bSuccess = g_WinHttpWrapper.SendHttpRequest(lpszUrl, NULL, NULL);

	return bSuccess;
}

BOOL WinHttp_RequestHttpFile(IN LPCWSTR lpszUrl, 
							 IN OUT std::string& strReturnData, 
							 IN IStatusCallback * _pIStatusCallback/* = NULL*/)
{
	BOOL bSuccess = FALSE;

	if (g_bWinHttpInited)
	{
		CMemOutStream _OutStream(strReturnData);

		bSuccess = g_WinHttpWrapper.SendHttpRequest(lpszUrl, &_OutStream, _pIStatusCallback);
	}

	return bSuccess;
}
