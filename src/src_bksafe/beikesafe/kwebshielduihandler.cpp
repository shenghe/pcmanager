#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "kwebshielduihandler.h"



void CKWebShieldUIHandler::Show(BOOL bShow)
{
	if (bShow)
	{
		if( m_wndWebShieldUI.IsWindow() )
		{
			m_wndWebShieldUI.ShowWindow(SW_SHOW);
		}
	}
	else
	{
		if( m_wndWebShieldUI.IsWindow() )
		{
			m_wndWebShieldUI.ShowWindow(SW_HIDE);
		}
	}
}


CBkNavigator* CKWebShieldUIHandler::OnNavigate(CString &strChildName)
{
    if (m_piWebShieldUI)
    {
        m_piWebShieldUI->Navigate(strChildName);
    }

	if (m_dlg && (0 == strChildName.CompareNoCase(L"IEFix1")))
	{
		m_dlg->OnNavigate(strChildName);
	}

    return NULL;
}

void CKWebShieldUIHandler::OnWebShieldReSize(CRect rcWnd)
{
	if( m_wndWebShieldUI.IsWindow() )
	{
		m_wndWebShieldUI.MoveWindow(&rcWnd);
	}
	else
	{
		m_bMove = TRUE;
		m_rtClient = rcWnd;
	}
}

void CKWebShieldUIHandler::OnSize(UINT nType, CSize size)
{
	SetMsgHandled( FALSE );
	return ;
}

int  CKWebShieldUIHandler::InitCtrl(void)
{
	InitWebShieldUI(TRUE);

	if (!m_bInit)
	{
		m_bInit = TRUE;
		
		if( m_bMove )
		{
			m_wndWebShieldUI.SetWindowPos(NULL, &m_rtClient, 0);
		}
	}

	return 1;
}
  
void CKWebShieldUIHandler::InitCtrlDelay(void)
{
	InitWebShieldUI(TRUE);

	if (m_piWebShieldUI)
	{
		m_piWebShieldUI->InitCtrlDelay();
	}
}

void CKWebShieldUIHandler::ReloadEngData(void)
{
	if (m_piWebShieldUI)
	{
		m_piWebShieldUI->ReloadEngData();
	}
}

void CKWebShieldUIHandler::ModifyMonitorState(int nType)
{
	if (m_piWebShieldUI)
	{
		m_piWebShieldUI->ModifyMonitorState(nType);
	}
}

LRESULT CKWebShieldUIHandler::OnKwsShowSetting(void)
{
	LRESULT lResult = NULL;

	if (m_piWebShieldUI)
	{
		lResult = m_piWebShieldUI->OnKwsShowSetting();
	}

	return lResult;
}

LRESULT	CKWebShieldUIHandler::OnKwsOpenDLSPage(void)
{
	LRESULT lResult = NULL;

	if (m_piWebShieldUI)
	{
		lResult = m_piWebShieldUI->OnKwsOpenDLSPage();
	}

	return lResult;
}

BOOL CKWebShieldUIHandler::InitWebShieldUI(BOOL bPreLoad)
{
    BOOL    bResult = FALSE;
	HRESULT hResult = E_FAIL;
	HMODULE hModule = NULL;
	HWND    hWnd    = NULL;
	TCHAR   szPath[MAX_PATH] = { 0 };

	IKWebShieldUI* piWebShieldUI = NULL;
    CreateWebShieldUIObject_t CreateObject = NULL;

    if (!m_hModule)
    {
        ::GetModuleFileName(NULL, szPath, MAX_PATH);
		::PathRemoveFileSpec(szPath);
		::PathAppend(szPath, _T("ksshield.dll"));
        hModule = LoadLibrary(szPath);
        if (!hModule)
		{
            goto Exit0;
		}

        InterlockedExchangePointer((void**)&m_hModule, hModule);
    }

    if (!m_piWebShieldUI)
    {
        CreateObject = (CreateWebShieldUIObject_t)GetProcAddress(m_hModule, "CreateWebShieldUIObject");
        if (!CreateObject)
		{
            goto Exit0;
		}

        hResult = CreateObject(__uuidof(IKWebShieldUI), (void**)&piWebShieldUI);
        if (FAILED(hResult))
		{
            goto Exit0;
		}

        InterlockedExchangePointer((void**)&m_piWebShieldUI, piWebShieldUI);
    }

    if (!m_wndWebShieldUI.m_hWnd)
    {
        hWnd = m_piWebShieldUI->CreateWebShieldUI(m_dlg->GetViewHWND());
        InterlockedExchangePointer((void**)&m_wndWebShieldUI.m_hWnd, hWnd);
    }

    bResult = TRUE;

Exit0:
    if (!bResult)
    {
        UnInitWebShieldUI();
    }

    return bResult;
}

void CKWebShieldUIHandler::UnInitWebShieldUI()
{
    if (m_wndWebShieldUI.m_hWnd)
    {
        m_wndWebShieldUI.DestroyWindow();
    }
}
