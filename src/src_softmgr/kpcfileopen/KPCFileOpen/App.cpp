//////////////////////////////////////////////////////////////////////////
//	
// FileName		:  app.cpp
// Comment		:  未知文件打开运行类
// Creator		:  Huang RuiKun (Nuk)
// Create time	:  2011-1-19
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "app.h"
#include "maindlg.h"
#include <tinyxml/tinyxml.h>
#include <common/utility.h>
#include <winhttp/WinHttpApi.h>
#include <stlsoft/memory/auto_buffer.hpp>
using namespace stlsoft;

CAppModule _Module;
CString g_strCmd;
CString g_strExt;
CString g_strTip;
CString g_strUrl;
static BOOL Startup();
static void OpenDefault();



BOOL IsExtExist()
{
	CPath path( g_strCmd );
	CString strExt;
	LONG lResult = ERROR_SUCCESS;

	strExt = path.GetExtension();
	if( strExt == TEXT("") )
		return FALSE;

	CRegKey regExt;
	lResult = regExt.Open( HKEY_CLASSES_ROOT, strExt );
	if (lResult != ERROR_SUCCESS)
		return FALSE;

	CString strSoftName;
	DWORD dwSoftNameLen = MAX_PATH + 1;
	lResult = regExt.QueryStringValue(L"", strSoftName.GetBuffer(dwSoftNameLen), &dwSoftNameLen);
	strSoftName.ReleaseBuffer();
	if( strSoftName == L"" )
		return FALSE;

	CRegKey regSoft;
	CString strKey;
	strKey.Format(L"%s\\shell\\open\\command", strSoftName);
	lResult = regSoft.Open( HKEY_CLASSES_ROOT, strKey );
	if (lResult != ERROR_SUCCESS)
		return FALSE;

	CString strSoftPath;
	DWORD dwSoftPathLen = MAX_PATH + 1;
	lResult = regSoft.QueryStringValue(L"", strSoftPath.GetBuffer(dwSoftPathLen), &dwSoftPathLen);
	strSoftPath.ReleaseBuffer();
	if( strSoftPath != L"" )
		return TRUE;

	return FALSE;
}

BOOL IsExtExistEx()
{
	LONG lResult = ERROR_SUCCESS;

	CPath path( g_strCmd );
	CString strExt;

	strExt = path.GetExtension();
	if( strExt == TEXT("") )
		return FALSE;

	CRegKey regCur;
	CString strKey;

	// for XP Current UserChoice
	strKey.Format(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s", strExt);
	lResult = regCur.Open(HKEY_CURRENT_USER, strKey );
	if (lResult != ERROR_SUCCESS)
		return FALSE;

	CString strSoftUser;
	DWORD dwSoftUserLen = MAX_PATH + 1;
	lResult = regCur.QueryStringValue(L"Application", strSoftUser.GetBuffer(dwSoftUserLen), &dwSoftUserLen);
	strSoftUser.ReleaseBuffer();
	if( strSoftUser != L"" )
		return TRUE;

	// for Win7 Current UserChoice
	regCur.Close();
	strKey.Format(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s\\UserChoice", strExt);
	lResult = regCur.Open(HKEY_CURRENT_USER, strKey );
	if (lResult != ERROR_SUCCESS)
		return FALSE;

	CString strSoftUser2;
	DWORD dwSoftUserLen2 = MAX_PATH + 1;
	lResult = regCur.QueryStringValue(L"Progid", strSoftUser2.GetBuffer(dwSoftUserLen2), &dwSoftUserLen2);
	strSoftUser2.ReleaseBuffer();
	if( strSoftUser2 != L"" )
		return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
	LPTSTR pszCmdLine = GetCommandLine();
	if( pszCmdLine )
	{
		int nNumArgs = 0;
		LPWSTR* ppszArgv = CommandLineToArgvW( pszCmdLine, &nNumArgs );
		if( ppszArgv && nNumArgs > 1 )
		{
			g_strCmd = ppszArgv[1];
		}

		if( ppszArgv )
			LocalFree( (HLOCAL)ppszArgv );
	}
	else
	{
		g_strCmd = lpstrCmdLine;
	}

	if( IsExtExist() )
	{
		OpenDefault();
		return 1;
	}
	else
	{
		if (IsExtExistEx())
		{
			OpenDefault();
			return 1;
		}
	}

	if(!Startup())
	{
		OpenDefault();
		return 1;
	}

	BkWinManager bkwinmgr;

	BkFontPool::SetDefaultFont(_T("宋体"), -12);

	CString strPath;

	GetModuleFileName((HMODULE)&__ImageBase, strPath.GetBuffer(MAX_PATH + 10), MAX_PATH);
	strPath.ReleaseBuffer();
	strPath.Truncate(strPath.ReverseFind(L'\\') + 1);
	strPath += L"res";

	BkResManager::SetResourcePath(strPath);

	BkSkin::LoadSkins(IDR_BK_SKIN_DEF);
	BkStyle::LoadStyles(IDR_BK_STYLE_DEF);
	BkString::Load(IDR_BK_STRING_DEF);

	CMainDlg dlg;

	dlg.DoModal(NULL);

	return 0;
}

namespace
{

//
// 导出SetOption函数
//
class WinHttpApi2 : public WinHttpApi
{
public:
	BOOL SetOption(HINTERNET h, DWORD o, LPVOID p, DWORD l)
	{
		BOOL bSuccess = FALSE;

		if (bUseWinHttp_)
			bSuccess = (WinHttpApi_.pWinHttpSetOption)(h, o, p, l);
		else
			bSuccess = (WinINetApi_.pInternetSetOption)(h, o, p, l);

		return bSuccess;
	}

	HINTERNET OpenSession2(LPCWSTR userAgent, LPCWSTR pProxyServer)
	{
		HINTERNET hSession = NULL;

		if (bUseWinHttp_)
			hSession = (WinHttpApi_.pWinHttpOpen)(userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, pProxyServer, WINHTTP_NO_PROXY_BYPASS, NULL/*WINHTTP_FLAG_ASYNC*/);
		else
			hSession = (WinINetApi_.pInternetOpen)(userAgent, INTERNET_OPEN_TYPE_PROXY, pProxyServer, NULL, 0);

		return hSession;
	}

	BOOL IsWinHttp() const
	{return bUseWinHttp_;}
};

// 查询文件类型信息
BOOL GetFileTypeInfo();
};

BOOL Startup()
{
	BOOL retVal = FALSE;

	do
	{
		// 1.解析命令行
		int pos = g_strCmd.ReverseFind(L'.');
		if(pos == -1) break;

		g_strExt = g_strCmd.Mid(pos + 1);
		g_strExt.MakeLower();
		if(g_strExt.IsEmpty()) break;

		// 2.根据后缀名获取类型信息
		if(!GetFileTypeInfo()) break;

		retVal = TRUE;
	}
	while(FALSE);

	return retVal;
}

void OpenDefault()
{
	TCHAR dstPath[MAX_PATH] = {0};
	::SHGetSpecialFolderPath(NULL, dstPath, CSIDL_WINDOWS, FALSE);
	::PathAddBackslash(dstPath);

	CString strParam;
	strParam.Format(L"%ssystem32\\shell32.dll,OpenAs_RunDLL %s", 
		dstPath,
		g_strCmd);
	CString strExe;
	strExe.Format(L"%ssystem32\\rundll32.exe", dstPath);
	::ShellExecute(NULL, L"open", strExe, strParam, NULL, SW_SHOWNORMAL);
}

namespace
{

BOOL GetFileTypeInfo()
{
	static const int QUERY_TIMEOUT = 5*1000;
	static const LPCWSTR QUERY_SERVER_NAME = L"baike.ijinshan.com";
	CString QUERY_OBJECT_NAME = L"/ksafe/client/unknow_file/" + g_strExt + L"/index.xml";

	BOOL retVal = FALSE;
	WinHttpApi2 httpApi;
	if(!httpApi.Initialize()) return FALSE;

	HINTERNET hSess = NULL;
	HINTERNET hConn = NULL;
	HINTERNET hHttp = NULL;

	do 
	{
		//
		// 加载代理数据
		//
		CString strIniPath;
		CAppPath::Instance().GetLeidianCfgPath(strIniPath);
		strIniPath += _T("\\bksafe.ini");

		static const int BUF_SIZE = 64;

		UINT validate = 0;
		wchar_t szUser[BUF_SIZE] = {0};
		wchar_t szPwd[BUF_SIZE] = {0};
		wchar_t szProxy[BUF_SIZE * 2] = {0};

		UINT type = ::GetPrivateProfileIntW(L"proxy", L"type", 0, strIniPath);
		if(type == 1)
		{
			// 改成无代理
			type = 0;

			do
			{
				UINT port = ::GetPrivateProfileIntW(L"proxy", L"port", 65536, strIniPath);
				if(port == 65536) break;

				wchar_t szHost[BUF_SIZE] = {0};
				::GetPrivateProfileStringW(L"proxy", L"host", NULL, szHost, BUF_SIZE, strIniPath);
				if(szHost[0] == L'\0') break;

				validate = ::GetPrivateProfileIntW(L"proxy", L"validate", -1, strIniPath);
				if(validate == static_cast<UINT>(-1)) break;

				INTERNET_PROXY_INFO proxy = {0};
				proxy.dwAccessType = INTERNET_OPEN_TYPE_PROXY;

				swprintf_s(szProxy, BUF_SIZE * 2, L"%s:%d", szHost, port);
				proxy.lpszProxy = szProxy;

				if(validate != 0)
				{
					::GetPrivateProfileStringW(L"proxy", L"user", NULL, szUser, BUF_SIZE, strIniPath);
					if(szUser[0] == L'\0') break;

					::GetPrivateProfileStringW(L"proxy", L"password", NULL, szPwd, BUF_SIZE, strIniPath);
					if(szPwd[0] == L'\0') break;
				}

				// 加载数据成功
				type = 1;
			}
			while(FALSE);
		}

		//
		// 设置代理
		//
		if(type == 1)
		{
			hSess = httpApi.OpenSession2(L"king_guard_softmgr", szProxy);
		}
		else
		{
			hSess = httpApi.OpenSession(L"king_guard_softmgr");
		}		
		if(hSess == NULL) break;

		httpApi.SetTimeouts(hSess, QUERY_TIMEOUT, QUERY_TIMEOUT, QUERY_TIMEOUT, QUERY_TIMEOUT);

		hConn = httpApi.Connect(hSess, QUERY_SERVER_NAME, INTERNET_DEFAULT_PORT);
		if(hConn == NULL) break;

		hHttp = httpApi.OpenRequest(hConn, L"GET", QUERY_OBJECT_NAME, INTERNET_SCHEME_HTTP);
		if(hHttp == NULL) break;

		//
		// 设置代理用户名与密码
		//
		if(type == 1  && validate == 1)
		{
			if(httpApi.IsWinHttp())
			{
				httpApi.SetOption(hHttp, WINHTTP_OPTION_PROXY_USERNAME, szUser, wcslen(szUser));
				httpApi.SetOption(hHttp, WINHTTP_OPTION_PROXY_PASSWORD, szPwd, wcslen(szPwd));
			}
			else
			{
				httpApi.SetOption(hHttp, INTERNET_OPTION_PROXY_USERNAME, szUser, wcslen(szUser));
				httpApi.SetOption(hHttp, INTERNET_OPTION_PROXY_PASSWORD, szPwd, wcslen(szPwd));
			}
		}

		BOOL result = httpApi.SendRequest(hHttp, NULL, 0);
		if(!result) break;

		result = httpApi.EndRequest(hHttp);
		if(!result) break;

		// 查询状态
		DWORD cbReturn = 0;
		DWORD cbSize = sizeof(cbReturn);
		result = httpApi.QueryInfo(hHttp, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, reinterpret_cast<char*>(&cbReturn), &cbSize);
		if(!result || cbReturn == HTTP_STATUS_NOT_FOUND) 
			break;

		// 查询长度
		cbReturn = 0;
		cbSize = sizeof(cbReturn);
		result = httpApi.QueryInfo(hHttp, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, reinterpret_cast<char*>(&cbReturn), &cbSize);
		if(!result || cbReturn == 0) break;

		auto_buffer<char> buffer(cbReturn + 2 + 1);
		if(buffer.empty()) break;

		DWORD cbRecved = 0;
		result = httpApi.ReadData(hHttp, &buffer[0], cbReturn, &cbRecved);
		if(!result || cbRecved != cbReturn) break;

		// 在结尾加\r\n\0空字符
		buffer[cbReturn] = '\r';
		buffer[cbReturn + 1] = '\n';
		buffer[cbReturn + 2] = '\0';

		// 开始解析
		TiXmlDocument xmlDoc;
		if(xmlDoc.Parse(&buffer[0]) == NULL) break;

		TiXmlElement *pElem = xmlDoc.FirstChildElement("file_type");
		if(pElem == NULL) break;

		pElem = pElem->FirstChildElement("description");
		if(pElem == NULL) break;

		g_strTip = CA2W(pElem->GetText(), CP_UTF8);
		if(g_strTip.IsEmpty()) break;

		g_strUrl = L"http://baike.ijinshan.com/ksafe/client/unknow_file/" + g_strExt + L"/index.html";
		retVal = TRUE;
	}
	while(FALSE);

	if(hHttp != NULL)
		httpApi.CloseInternetHandle(hHttp);

	if(hConn != NULL)
		httpApi.CloseInternetHandle(hConn);

	if(hSess != NULL)
		httpApi.CloseInternetHandle(hSess);

	httpApi.Uninitialize();
	return retVal;
}

}