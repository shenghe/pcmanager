#pragma once
#include <vector>

using namespace std;

struct APP_WND_STATE_ITEM
{
public:
	APP_WND_STATE_ITEM()
	{
		hWnd = NULL;
		hIcon = NULL;
		bHung = FALSE;
		dwPID = 0;
	}

	HWND    hWnd;				//窗口句柄
	WCHAR   szTitle[MAX_PATH];	//窗口标题
	HICON   hIcon;				//窗口图标
	BOOL    bHung;				//是否未响应窗口
	DWORD	dwPID;				//对应进程的ID
};

typedef APP_WND_STATE_ITEM* LPAPP_WND_STATE_ITEM;

class KAppWndStateDataMgr
{
public:
	KAppWndStateDataMgr()
	{

	}
	~KAppWndStateDataMgr()
	{
		Uninit();
	}

	HRESULT Init()
	{
		return S_OK;
	}

	HRESULT Uninit()
	{
		for (int i = 0;i++;i < m_vecAppWndState.size())
			delete m_vecAppWndState[i];
		m_vecAppWndState.clear();
		return S_OK;
	}

	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
	{
		HICON   hIcon;
		WCHAR   szText[MAX_PATH];
		BOOL    bHung = FALSE;
		HICON*  xhIcon = (HICON*)&hIcon;

		DWORD dwPID = -1;
		GetWindowThreadProcessId(hWnd, &dwPID);
		GetWindowTextW(hWnd, szText, MAX_PATH); /* Get the window text */

		if ((wcslen(szText) <= 0) || !IsWindowMustList(hWnd))
		{
			return TRUE;
		}

		/* Get the icon for this window */
		hIcon = GetIconByHwnd(hWnd);
		bHung = IsHungAppWindow(hWnd);
		AddOrUpdateHwnd(hWnd, szText, hIcon, bHung, dwPID);

		return TRUE;
	}

	static HICON GetIconByHwnd(HWND hWnd)
	{
		HICON hIcon = NULL;
		HICON*  xhIcon = (HICON*)&hIcon;
		SendMessageTimeoutW(hWnd, WM_GETICON, ICON_SMALL, 0, 0, 100, (PDWORD_PTR)xhIcon);
		if (!hIcon)
		{
			hIcon = (HICON)(LONG_PTR)GetClassLongPtrW(hWnd, GCL_HICONSM);
			if (!hIcon) 
				SendMessageTimeoutW(hWnd, WM_QUERYDRAGICON, 0, 0, 0, 100, (PDWORD_PTR)xhIcon);
			if (!hIcon) 
				SendMessageTimeoutW(hWnd, WM_GETICON,  ICON_SMALL, 0, 0, 100, (PDWORD_PTR)xhIcon);
			if (hIcon)
			{
				return hIcon;
			}
		}
		else
		{
			return hIcon;
		}
		return NULL;
	}

	static BOOL IsWindowMustList(HWND hWnd)
	{
		if (!::IsWindowVisible(hWnd))
		{
			return FALSE;
		}

		RECT rct = {0};
		::GetWindowRect(hWnd, &rct);
		if ((rct.left == rct.right))
		{
			return FALSE;
		}

		DWORD nExStyle = ((DWORD)GetWindowLong(hWnd, GWL_EXSTYLE));
		DWORD nStyle = ((DWORD)GetWindowLong(hWnd, GWL_STYLE));

		if (nExStyle & WS_EX_TOOLWINDOW)
		{
			return FALSE;
		}

		if (nExStyle & WS_EX_APPWINDOW)
		{
			return TRUE;
		}

		if (nStyle & WS_CAPTION)
		{
			return TRUE;
		}
		if (nStyle & WS_SYSMENU)
		{
			return TRUE;
		}
		return FALSE;
	}

	BOOL UpdateAppWndState()
	{
		vector<LPAPP_WND_STATE_ITEM>::iterator it;
		for (it = m_vecAppWndState.begin();it != m_vecAppWndState.end();it++)
			delete *it;

		m_vecAppWndState.clear();
		/*
		 * FIXME:
		 *
		 * Should this be EnumDesktopWindows() instead?
		 */
		return EnumWindows(EnumWindowsProc, 0);
	}

	static void AddOrUpdateHwnd(HWND hWnd, WCHAR *szTitle, HICON hIcon, BOOL bHung, DWORD dwPID)
	{
		LPAPP_WND_STATE_ITEM lpAppWndState = new APP_WND_STATE_ITEM;
		lpAppWndState->hWnd = hWnd;
		lpAppWndState->hIcon = hIcon;
		lpAppWndState->bHung = bHung;
		lpAppWndState->dwPID = dwPID;
		_tcscpy_s(lpAppWndState->szTitle, MAX_PATH, szTitle);
		m_vecAppWndState.push_back(lpAppWndState);
	}

	int GetAppWndStateData(vector<LPAPP_WND_STATE_ITEM>& vecData)
	{
		std::copy(m_vecAppWndState.begin(), m_vecAppWndState.end(), std::back_inserter(vecData));
		return vecData.size();
	}

private:
	CString								m_strModulePath;
	static vector<LPAPP_WND_STATE_ITEM> m_vecAppWndState;
/*	static CAtlMap<HWND, HICON>			m_mapWndIcon;*/
};

__declspec(selectany) vector<LPAPP_WND_STATE_ITEM> KAppWndStateDataMgr::m_vecAppWndState;
/*__declspec(selectany) CAtlMap<HWND, HICON>		   KAppWndStateDataMgr::m_mapWndIcon;*/