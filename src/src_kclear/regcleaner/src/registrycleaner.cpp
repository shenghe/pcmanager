// RegistryCleaner.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "registrycleaner.h"
#include "maindlg.h"
#include "guidef.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

CAppModule	_Module;
CAppModule* _ModulePtr = &_Module;
CMainDlg	g_mainDlg;

HINSTANCE g_hInstance;


HMODULE GetMoudleHandle()
{
	return _Module.m_hInst;
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	g_hInstance = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		gGuiFont.Init();
		_Module.Init(NULL, hModule);
		{
			KAppRes& appRes = KAppRes::Instance();
			std::string strXml;

			KuiFontPool::SetDefaultFont(_T("宋体"), -12);

			appRes.GetXmlData("IDR_KSC_SKIN", strXml);
			KuiSkin::LoadSkins(strXml);

			appRes.GetXmlData("IDR_KSC_STYLE", strXml);
			KuiStyle::LoadStyles(strXml);
		}
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if ( ::IsWindow(g_mainDlg) ){
			g_mainDlg.DestroyWindow();
		}
		gGuiFont.UnInit();
		_Module.Term();
		break;
	}

    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// 传入父窗口句柄，显示注册表清理的窗口
HWND __stdcall ShowDlg(HWND hwndParent)
{
	if ( !::IsWindow(hwndParent) )
	{
		g_mainDlg.DoModal();
	}
	else
	{
		if ( ::IsWindow(g_mainDlg) )
			return g_mainDlg;

		LONG	lStyle = 0;

		g_mainDlg.Create(hwndParent);
		g_mainDlg.ShowWindow(SW_SHOW);

		lStyle = ::GetWindowLong(g_mainDlg, GWL_STYLE);
		lStyle &= ~(DS_MODALFRAME | WS_CAPTION | WS_DLGFRAME | WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU );
		lStyle |= WS_CHILD;
		::SetWindowLong(g_mainDlg, GWL_STYLE, lStyle);
		::SetParent(g_mainDlg, hwndParent);
	}

	return g_mainDlg;
}
