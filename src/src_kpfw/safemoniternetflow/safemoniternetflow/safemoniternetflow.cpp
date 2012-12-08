// safemoniternetflow.cpp : main source file for safemoniternetflow.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "src/kmaindlg.h"
#include "safemoniternetflow.h"
#include "bkres/bkstringpool.h"
#include "KDump/KDump.h"
#include "common/KCheckInstance.h"
#include "src/beikesafemsgbox.h"
#include "winmod/winwow64.h"
#include "kpfw/msg_logger.h"

KAppModule _Module;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{

	KDump::Instance().Install();

	if (S_FALSE == _Module.Init(hInstance))
		return -1;

	KCheckInstance* _pInst = KCheckInstance::Instance();
	if (_pInst == NULL)
		return 0;

	int nRetv = _pInst->CheckFirstInstance(lpstrCmdLine/*, NULL, _T("Kingsoft Antivirus KSG Update Mutex")*/);
	if (!nRetv)
	{
		_pInst->ClearFirstInstance();
		return 0;
	}
	
	// 启动的时候，自动起托盘
// 	WCHAR bufPath[MAX_PATH] = {0};
// 	::GetModuleFileName(NULL, bufPath, MAX_PATH);
// 	::PathRemoveFileSpecW(bufPath);
// 	::PathAppend(bufPath, TEXT("KSafeTray.exe"));
// 	::ShellExecute(NULL, NULL, bufPath, NULL, NULL, SW_HIDE);

	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	_Module.Run();
	::CoUninitialize();

	_Module.Uninit();

	return 0;
}

HRESULT KAppModule::Init( HINSTANCE hInstance )
{

	return S_OK;
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
/*
#if _DEBUG
	if (FALSE)
#else
	if (_CheckIs64BitOp())
#endif
	{
		//如果是64位操作系统，就退出。
		CBkSafeMsgBox2 dlg;
		dlg.ShowMutlLineMsg(BkString::Get(DefString33), BkString::Get(DefString7), MB_OK | MB_ICONEXCLAMATION);
		return S_OK;
	}*/
	
	KMainDlg dlgMain;

	if (!dlgMain.CheckServericeIsOK())
	{
		netmon_log(L"CheckServiceIsOK Failed！");
	}

	if (!dlgMain.CheckDriver())
	{
		netmon_log(L"CheckDriver failed.Netmon exit.");
	}
	else
	{
		netmon_log(L"Netmon dlg start ");
		int uRet = dlgMain.DoModal(NULL);
	}
	
	return S_OK;
}

void KAppModule::Uninit()
{
	
}

BOOL KAppModule::_CheckIsUniquely( void )
{
	HANDLE hEvent = CreateEvent(NULL, FALSE, TRUE, L"{BC1136C3-F8A3-4056-AD62-42938FF64969}_GUID_SAFE_FLOW");
	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
		return FALSE;

	return TRUE;
}

BOOL KAppModule::_CheckIs64BitOp( void )
{
	BOOL bIsWow64 = TRUE;
	if (!WinMod::CWinModule_kernel32::IsWow64Process(::GetCurrentProcess(), &bIsWow64))
		bIsWow64 = FALSE;

	return bIsWow64;
}