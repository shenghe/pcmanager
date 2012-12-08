#include "stdafx.h"
#include "kscmain.h"
#include "ksctoolvector.h"
#include "kscbase/kscfilepath.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////

KscToolVector::KscToolVector()
{
	m_fSingleMode = FALSE;
	memset(&m_hTools, 0, sizeof(m_hTools));
}

KscToolVector::~KscToolVector()
{
}

//////////////////////////////////////////////////////////////////////////

BOOL KscToolVector::InitTools()
{
//	LoadTool(1, L"trashcleaner.dll");
    LoadTool(3, L"regcleaner.dll");

	return TRUE;
}

void KscToolVector::UnInitTools()
{
	for (int i = 0; i < ARRAYSIZE(m_hTools); ++i)
	{
		if (m_hTools[i].hWnd)
		{
			CloseWindow(m_hTools[i].hWnd);
			m_hTools[i].hWnd = NULL;
		}

		if (m_hTools[i].hDll)
		{
			FreeLibrary(m_hTools[i].hDll);
			m_hTools[i].hDll = NULL;
		}
	}
}

BOOL KscToolVector::SwitchTool(int nIndex)
{
	BOOL retval = FALSE;
	int i;

	if (nIndex > ARRAYSIZE(m_hTools) - 1)
		goto clean0;

    if (!m_hTools[nIndex].hWnd && m_hTools[nIndex].pShowDlg)
	{
		m_hTools[nIndex].hWnd = m_hTools[nIndex].pShowDlg(m_hWnd);
		if (!m_hTools[nIndex].hWnd)
			goto clean0;

		UpdateToolSize(m_hTools[nIndex].hWnd);
	}

	for (i = 0; i < ARRAYSIZE(m_hTools); ++i)
	{
		if (m_hTools[i].hWnd)
		{
			::ShowWindow(m_hTools[i].hWnd, SW_HIDE);
		}
	}

    if (m_hTools[nIndex].hWnd)
    {
        ::ShowWindow(m_hTools[nIndex].hWnd, SW_SHOW);
        ShowWindow(SW_SHOW);
    }
    else
    {
        ShowWindow(SW_HIDE);
    }

	retval = TRUE;

clean0:
	return retval;
}

BOOL KscToolVector::LoadTool(int nIndex, const std::wstring& strDll)
{
	BOOL retval = FALSE;
	KFilePath appPath = KFilePath::GetFilePath(NULL);
	KFilePath dllPath;
	HMODULE hDll = NULL;
	ShowDlg_t ShowDlg = NULL;

	appPath.RemoveFileSpec();

	dllPath = appPath;
	dllPath.Append(strDll);
	hDll = ::LoadLibraryW(dllPath);
	if (!hDll)
		goto clean0;

	ShowDlg = (ShowDlg_t)::GetProcAddress(hDll, "ShowDlg");
	if (!ShowDlg)
		goto clean0;

	m_hTools[nIndex].hDll = hDll;
	m_hTools[nIndex].pShowDlg = ShowDlg;
	m_hTools[nIndex].hWnd = NULL;

	retval = TRUE;

clean0:
	return retval;
}

//////////////////////////////////////////////////////////////////////////

void KscToolVector::OnSize(UINT nType, CSize size)
{
	int i;

	GetClientRect(&m_rcClient);

	for (i = 0; i < ARRAYSIZE(m_hTools); ++i)
	{
		if (m_hTools[i].hWnd)
		{
			::SetWindowPos(m_hTools[i].hWnd, NULL, 0, 0, m_rcClient.right, 
				m_rcClient.bottom, 0);
		}
	}
}

BOOL KscToolVector::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	GetClientRect(&m_rcClient);
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////

void KscToolVector::UpdateToolSize(HWND hWnd)
{
	::SetWindowPos(hWnd, NULL, 0, 0, m_rcClient.right, 
		m_rcClient.bottom, 0);
}

//////////////////////////////////////////////////////////////////////////
