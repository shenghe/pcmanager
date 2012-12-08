#include "stdafx.h"
#include "kclearimpl.h"

//////////////////////////////////////////////////////////////////////////

CKClearImpl::CKClearImpl() : m_pDlgMain(NULL)
{
    
}

CKClearImpl::~CKClearImpl()
{

}

//////////////////////////////////////////////////////////////////////////

void CKClearImpl::Release()
{
    delete this;
}

HWND CKClearImpl::CreateKClear(HWND hParent)
{
    HWND retval = NULL;

    if (m_pDlgMain)
    {
        retval = m_pDlgMain->GetParent();
        goto clean0;
    }

    m_pDlgMain = new CKscMainDlg();
    if (!m_pDlgMain)
        goto clean0;

    retval = m_pDlgMain->Create(hParent);

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

BOOL CKClearImpl::Navigate(const wchar_t* szNavigate)
{
    BOOL retval = FALSE;

    if (!szNavigate)
        goto clean0;

    if (!m_pDlgMain)
        goto clean0;

    return m_pDlgMain->OnNavigate(szNavigate);

clean0:
    return retval;
}

BOOL CKClearImpl::PreNavigate(const wchar_t* szNavigate)
{
    BOOL retval = FALSE;

    if (!szNavigate)
        goto clean0;

    if (!m_pDlgMain)
        goto clean0;

    return m_pDlgMain->PreNavigate(szNavigate);

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////
/*
void CKClearImpl::OnSize(const RECT* lprcWnd)
{
    if (!lprcWnd)
        goto clean0;

    if (!m_pDlgMain)
        goto clean0;

    m_pDlgMain->SetWindowPos(NULL, lprcWnd, 0);

clean0:
    return;
}
*/
//////////////////////////////////////////////////////////////////////////
