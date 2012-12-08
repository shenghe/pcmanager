// safemoniternetflow.cpp : main source file for safemoniternetflow.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "src/kmaindlg.h"
#include "beikesafeperfopt.h"
#include "bkres/bkstringpool.h"
#include "KDump/KDump.h"
#include "common/KCheckInstance.h"
#include "src/beikesafemsgbox.h"
#include "winmod/winwow64.h"
#include "kpfw/msg_logger.h"
#include "Shlwapi.h"
#include "_idl_gen/bksafesvc.h"
#include <comproxy/bkservicecall.h>

#define _KSAFE_PERFOPT_MUTEX_NAME			_T("{068A7D1B-2B9D-4c23-836D-CC7CF326975F}_PERFOPT_MUTEX")
#define _KSAFE_PERFOPT_FILEMAPPTING_NAME	_T("{6E03B770-23B9-4b5b-B7D0-E2E83DBFE7CC}_PERFOPT_FILEMAP")

#define _CMDLINE_PARAM_CPU					_T("cpu")
#define _CMDLINE_PARAM_MEM					_T("mem")
#define _CMDLINE_PARAM_ONEKEY				_T("ONEKEYSPEEDUP")

typedef struct _MY_TOKEN_MANDATORY_LABEL {
	SID_AND_ATTRIBUTES Label;
} MY_TOKEN_MANDATORY_LABEL, *PMY_TOKEN_MANDATORY_LABEL;


typedef struct _MY_TOKEN_LINKED_TOKEN {
	HANDLE LinkedToken;
} MY_TOKEN_LINKED_TOKEN, *PMY_TOKEN_LINKED_TOKEN;


typedef enum _MY_TOKEN_INFORMATION_CLASS {
	MYTokenElevationType  = 18, // MaxTokenInfoClass
	MYTokenLinkedToken,
	MYTokenElevation,

	MYTokenHasRestrictions,
	MYTokenAccessInformation,
	MYTokenVirtualizationAllowed,
	MYTokenVirtualizationEnabled,
	MYTokenIntegrityLevel,
	MYTokenUIAccess,
	MYTokenMandatoryPolicy,
	MYTokenLogonSid,
	//MaxTokenInfoClass  // MaxTokenInfoClass should always be the last enum
} MY_TOKEN_INFORMATION_CLASS, *PMY_TOKEN_INFORMATION_CLASS;

#ifndef SECURITY_MANDATORY_HIGH_RID
#define SECURITY_MANDATORY_HIGH_RID                 (0x00003000L)
#endif 

#ifndef SE_GROUP_INTEGRITY
#define SE_GROUP_INTEGRITY                 (0x00000020L)
#endif

KAppModule _Module;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{

	KDump::Instance().Install();

	if (S_FALSE == _Module.Init(hInstance))
		return -1;

	KCheckInstance* _pInst = KCheckInstance::Instance();
	if (_pInst == NULL)
		return 0;

	int nRetv = _pInst->CheckFirstInstance(lpstrCmdLine, _KSAFE_PERFOPT_MUTEX_NAME, _KSAFE_PERFOPT_FILEMAPPTING_NAME);
	if (!nRetv)
	{
		_pInst->ClearFirstInstance();
		return 0;
	}

	if (!_Module.CheckIntegrityLevel())
		return -3;

	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	HRESULT hRet = ::CoInitializeSecurity(
		NULL, 
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
		);
	if (FAILED(hRet))
	{
		return -1;
	}

	hRet = _Module.ParseCommandLine(lpstrCmdLine);
	if (FAILED(hRet))
		return -2;

	_Module.Run();
	::CoUninitialize();

	_Module.Uninit();

	return 0;
}

BOOL NeedServiceCall()
{
	BOOL bRet = FALSE, bNeedSvcCall = FALSE;
	HANDLE hToken = NULL;
	DWORD dwBytesNeeded = 0;
	MY_TOKEN_MANDATORY_LABEL *pTIL = NULL;

	if (!RunTimeHelper::IsVista())
		goto Exit0;

	bRet = ::OpenProcessToken(::GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
	if (NULL == hToken)
		return FALSE;

	bRet = ::GetTokenInformation(
		hToken,
		(TOKEN_INFORMATION_CLASS)MYTokenIntegrityLevel,
		NULL, 0, &dwBytesNeeded);

	pTIL = (MY_TOKEN_MANDATORY_LABEL *)new BYTE[dwBytesNeeded];
	if (!pTIL)
		goto Exit0;

	bRet = ::GetTokenInformation(
		hToken,
		(TOKEN_INFORMATION_CLASS)MYTokenIntegrityLevel,
		pTIL, dwBytesNeeded, &dwBytesNeeded);
	if (!bRet || !pTIL)
		goto Exit0;

	SID* pSid = static_cast<SID*>(pTIL->Label.Sid);
	if (!pSid)
		goto Exit0;

	if (SECURITY_MANDATORY_HIGH_RID > pSid->SubAuthority[0])
		bNeedSvcCall = TRUE;

Exit0:

	if (NULL != pTIL)
	{
		delete[] (LPBYTE)pTIL;
		pTIL = NULL;
	}

	if (hToken)
	{
		::CloseHandle(hToken);
		hToken = NULL;
	}

	return bNeedSvcCall;
}

HRESULT KAppModule::ParseCommandLine(LPCWSTR lpszCmdLine)
{
	HRESULT hRet = S_OK;

	BOOL bRet = m_CmdLine.Analyze(lpszCmdLine);
	if (!bRet)
		return E_FAIL;

	return hRet;
}

BOOL KAppModule::CheckIntegrityLevel()
{
	BOOL bNeedSvcCall = NeedServiceCall();
	CString strCmdLine;

	m_CmdLine.GetCmdLine(strCmdLine);

	if (!bNeedSvcCall)
		return TRUE;

	CBkServiceCall svccall;

	::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	HRESULT hRet = svccall.Initialize();
	if (FAILED(hRet))
	{
		//         CString strText;
		//         strText.Format(L"svccall.Initialize() Failed 0x%08X", hRet);
		//         ::MessageBox(0, strText, 0, 0);
		::CoUninitialize();
		goto _ShowUAC;
	}

	hRet = svccall.Execute(m_strAppFileName, strCmdLine, TRUE);

	svccall.Uninitialize();

	::CoUninitialize();

	if (FAILED(hRet))
	{
		//         CString strText;
		//         strText.Format(L"svccall.Execute() Failed 0x%08X", hRet);
		//         ::MessageBox(0, strText, 0, 0);
		goto _ShowUAC;
	}

	return FALSE;

_ShowUAC:

	BOOL bRet = m_CmdLine.Execute(m_strAppFileName, TRUE, FALSE);

	if (!bRet)
		::MessageBox(0, L"请使用管理员账户运行本软件", 0, 0);

	return FALSE;
}

HRESULT KAppModule::Init( HINSTANCE hInstance )
{
	HRESULT hRet = S_OK;
	DWORD dwRet = ::GetModuleFileName(NULL, m_strAppFileName.GetBuffer(MAX_PATH + 1), MAX_PATH);
	if (0 == dwRet)
	{
		m_strAppFileName.ReleaseBuffer(0);

		hRet = E_FAIL;
	}
	else
	{
		m_strAppFileName.ReleaseBuffer();

		m_strAppPath = m_strAppFileName;
		m_strAppPath.Truncate(m_strAppPath.ReverseFind(L'\\') + 1);
	}
	return hRet;
}

LPCTSTR KAppModule::GetAppFilePath()
{
	return m_strAppFileName;
}

void KAppModule::_InitUIResource()
{

	//BkFontPool::SetDefaultFont(BkString::Get(IDS_APP_FONT), -12);

	if ( TRUE /*_CmdLine.HasParam(L"{69DD4969-E6C4-42d9-A508-105DDA13CE40}")*/)
	{
		CString strPath;

		GetModuleFileName((HMODULE)&__ImageBase, strPath.GetBuffer(MAX_PATH + 10), MAX_PATH);
		strPath.ReleaseBuffer();
		strPath.Truncate(strPath.ReverseFind(L'\\') + 1);
		strPath += L"res\\safeflow";

		if ( PathFileExists(strPath) )
			BkResManager::SetResourcePath(strPath);
	}

	BkFontPool::SetDefaultFont(_T("宋体"), -12);

	BkSkin::LoadSkins(IDR_BK_SKIN_DEF);
	BkStyle::LoadStyles(IDR_BK_STYLE_DEF);
	BkString::Load(IDR_BK_STRING_DEF);
}

HRESULT KAppModule::Run()
{
	_InitUIResource();

	if ( m_hModRichEdit2 == NULL )
		m_hModRichEdit2 = ::LoadLibrary(_T("RICHED20.DLL"));
	
	if (m_CmdLine.HasParam(_CMDLINE_PARAM_CPU))
	{
		m_dwRunParamType = LISTBOX_COLUM_CPU;
	}
	else if (m_CmdLine.HasParam(_CMDLINE_PARAM_MEM))
	{
		m_dwRunParamType = LISTBOX_COLUM_MEM;
	}
	else if (m_CmdLine.HasParam(_CMDLINE_PARAM_ONEKEY))
	{
		m_dwRunParamType = RUN_UI_ONEKEYSPEED;
	}
	else
	{
		m_dwRunParamType = LISTBOX_COLUM_MEM;
	}

	KMainDlg dlgMain(m_dwRunParamType);
	dlgMain.DoModal();	
	return S_OK;
}

void KAppModule::Uninit()
{
	
}