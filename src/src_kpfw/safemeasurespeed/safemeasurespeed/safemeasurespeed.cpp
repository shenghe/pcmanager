// safemoniternetflow.cpp : main source file for safemoniternetflow.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include "resource.h"
#include "safemeasurespeed.h"
#include "bkres/bkres.h"
#include "src/kmaindlg.h"

#include "KDump/KDump.h"

KAppModule _Module;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{

	KDump::Instance().Install();

	if (S_FALSE == _Module.Init(hInstance))
		return -1;


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
	if (!_CheckIsUniquely())
		return S_FALSE;

	return S_OK;
}


void KAppModule::_InitUIResource()
{

	//BkFontPool::SetDefaultFont(BkString::Get(IDS_APP_FONT), -12);
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

	
	KMainDlg dlgMain;

	int uRet = dlgMain.DoModal(NULL);

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