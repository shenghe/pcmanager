#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "ktrojanuihandler.h"


BOOL CKTrojanUIHandler::InitKTrojan(BOOL bPreLoad /* = FALSE */)
{
	BOOL retval = FALSE;
	CreateKTrojanObject_t CreateKTrojanObject = NULL;
	TCHAR szKTrojanPath[MAX_PATH] = { 0 };
	HRESULT hr;
	HMODULE hKTrojan = NULL;
	IKTrojan* piKTrojan = NULL;
	HWND hWnd = NULL;

	if (!m_hKTrajon)
	{
		::GetModuleFileName(NULL, szKTrojanPath, MAX_PATH);
		PathRemoveFileSpec(szKTrojanPath);
		PathAppend(szKTrojanPath, _T("kscan.dll"));
		hKTrojan = LoadLibrary(szKTrojanPath);
		if (!hKTrojan)
			goto clean0;

		InterlockedExchangePointer((void**)&m_hKTrajon, hKTrojan);
	}

	if (!m_piKTrajon)
	{
		CreateKTrojanObject = (CreateKTrojanObject_t)GetProcAddress(m_hKTrajon, "CreateKTrojanObject");
		if (!CreateKTrojanObject)
			goto clean0;

		hr = CreateKTrojanObject(__uuidof(IKTrojan), (void**)&piKTrojan);
		if (FAILED(hr))
			goto clean0;

		InterlockedExchangePointer((void**)&m_piKTrajon, piKTrojan);
	}
	if (!m_wndKTrojan.m_hWnd)
	{
		hWnd = m_piKTrajon->CreateKTrojan(m_dlg->GetViewHWND());
		InterlockedExchangePointer((void**)&m_wndKTrojan.m_hWnd, hWnd);
		m_piKTrajon->SetNotify(m_dlg);
		m_piKTrajon->Init();
	}

	retval = TRUE;

clean0:
	if (!retval)
	{
		UnInitKTrojan();
	}

	return retval;
}
HWND CKTrojanUIHandler::GetTrojanHwnd()
{
	return m_wndKTrojan.m_hWnd;
}
void CKTrojanUIHandler::UnInitKTrojan()
{
	if (m_wndKTrojan.m_hWnd)
	{
		m_wndKTrojan.DestroyWindow();//Ïú»Ù´°¿Ú
	}
	return;
}

void CKTrojanUIHandler::Init()
{
	if (!m_bInit)
	{
		m_bInit = TRUE;
		InitKTrojan();
		if( m_bMove )
			m_wndKTrojan.SetWindowPos(NULL, &m_rtClient, 0);
	}
	
}
void CKTrojanUIHandler::UnInit()
{
	UnInitKTrojan();
	m_bInit = FALSE;
	return;
}
void CKTrojanUIHandler::OnSize(UINT nType, CSize size)
{
	SetMsgHandled( FALSE );
	return ;
}

void CKTrojanUIHandler::OnKTrojanReSize(CRect rcWnd)
{
	if( m_wndKTrojan.IsWindow() )
	{
		m_wndKTrojan.MoveWindow(&rcWnd);
	}
	else
	{
		m_bMove = TRUE;
		m_rtClient = rcWnd;
	}
	return;
}

void CKTrojanUIHandler::Show(BOOL bShow)
{
	if (bShow)
	{
		if( m_wndKTrojan.IsWindow() )
			m_wndKTrojan.ShowWindow(SW_SHOW);
	}
	else
	{
		if( m_wndKTrojan.IsWindow() )
			m_wndKTrojan.ShowWindow(SW_HIDE);
	}
	
}
CBkNavigator* CKTrojanUIHandler::OnNavigate(CString &strChildName)
{
	if (m_piKTrajon)
	{
		m_piKTrajon->Navigate(strChildName);
	}
	return NULL;
}
STATE_CLOSE_CHECK CKTrojanUIHandler::CloseCheck()
{
	STATE_CLOSE_CHECK st = CLOSE_CHECK_CONTINUE;
	if (m_piKTrajon)
	{
		st = m_piKTrajon->CloseCheck();
	}
	return st;
}

VOID CKTrojanUIHandler::CloseSuccess(BOOL bSucClose)
{
	if (m_bInit)
	{
		if (m_piKTrajon)
			m_piKTrajon->CloseSuccess(bSucClose);
	}
	return;
}