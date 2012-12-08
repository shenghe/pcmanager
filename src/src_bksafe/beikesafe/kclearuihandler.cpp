#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "kclearuihandler.h"

//////////////////////////////////////////////////////////////////////////

CBkNavigator* CKClearUIHandler::OnNavigate(CString &strChildName)
{
    if (m_piKClear)
    {
        m_piKClear->Navigate(strChildName);
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////

void CKClearUIHandler::OnKClearReSize(CRect rcWnd)
{
	if( m_wndKClear.IsWindow() )
		m_wndKClear.SetWindowPos(NULL, &rcWnd, 0);
	else
	{
		m_bMove = TRUE;
		m_rtClient = rcWnd;
	}
}

//////////////////////////////////////////////////////////////////////////

void CKClearUIHandler::Init()
{
	if (!m_bInit)
	{
		m_bInit = TRUE;
		InitKClear();
		if( m_bMove )
			m_wndKClear.SetWindowPos(NULL, &m_rtClient, 0);
	}
}

void CKClearUIHandler::UnInit()
{
    UnInitKClear();
	m_bInit = FALSE;
}

void CKClearUIHandler::Show()
{
	if( m_wndKClear.IsWindow() )
		m_wndKClear.ShowWindow(SW_SHOW);
}

void CKClearUIHandler::Hide()
{
	if( m_wndKClear.IsWindow() )
		m_wndKClear.ShowWindow(SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////

BOOL CKClearUIHandler::InitKClear(BOOL bPreLoad)
{
    BOOL retval = FALSE;
    CreateKClearObject_t CreateKClearObject = NULL;
    TCHAR szKClearPath[MAX_PATH] = { 0 };
    HRESULT hr;
    HMODULE hKClear = NULL;
    IKClear* piKClear = NULL;
    HWND hWnd = NULL;

    if (!m_hKClear)
    {
        ::GetModuleFileName(NULL, szKClearPath, MAX_PATH);
        PathRemoveFileSpec(szKClearPath);
        PathAppend(szKClearPath, _T("kclear.dll"));
        hKClear = LoadLibrary(szKClearPath);
        if (!hKClear)
            goto clean0;

        InterlockedExchangePointer((void**)&m_hKClear, hKClear);
    }

    if (!m_piKClear)
    {
        CreateKClearObject = (CreateKClearObject_t)GetProcAddress(m_hKClear, "CreateKClearObject");
        if (!CreateKClearObject)
            goto clean0;

        hr = CreateKClearObject(__uuidof(IKClear), (void**)&piKClear);
        if (FAILED(hr))
            goto clean0;

        InterlockedExchangePointer((void**)&m_piKClear, piKClear);
    }

    if (!m_wndKClear.m_hWnd)
    {
        if (!bPreLoad)
        {
            hWnd = m_piKClear->CreateKClear(m_dlg->GetViewHWND());
            InterlockedExchangePointer((void**)&m_wndKClear.m_hWnd, hWnd);
            //m_wndKClear.ShowWindow(SW_SHOW);
        }
    }

    retval = TRUE;

clean0:
    if (!retval)
    {
        UnInitKClear();
    }

    return retval;
}

void CKClearUIHandler::UnInitKClear()
{
    if (m_wndKClear.m_hWnd)
    {
        m_wndKClear.DestroyWindow();
    }

//     if (m_piKClear)
//     {
//         m_piKClear->Release();
//         m_piKClear = NULL;
//     }
// 
//     if (m_hKClear)
//     {
//         FreeLibrary(m_hKClear);
//         m_hKClear = NULL;
//     }
}

VOID CKClearUIHandler::CloseSuccess( BOOL bSucClose )
{
	if (bSucClose && m_bInit)
		UnInit();
}

void CKClearUIHandler::PreLoad()
{
    HANDLE hThread = NULL;

    hThread = (HANDLE)_beginthreadex(NULL, 0, PreLoadThread, this, 0, NULL);
    if (hThread)
    {
        ::CloseHandle(hThread);
        hThread = NULL;
    }
}

UINT CKClearUIHandler::PreLoadThread(LPVOID pParam)
{
    CKClearUIHandler* pThis = (CKClearUIHandler*)pParam;

    if (pThis)
    {
        pThis->InitKClear(TRUE);
    }

    _endthreadex(0);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
