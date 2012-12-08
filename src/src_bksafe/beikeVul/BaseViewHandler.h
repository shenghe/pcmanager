#pragma once

template<typename T>
class CBaseViewHandler
{
public:
	CBaseViewHandler(T &refWin) : m_RefWin(refWin) 
	{
	}
	
	BOOL SetPanelXml(UINT uItemID, UINT uResID, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetPanelXml(uItemID, uResID, bRedraw);
	}

	BOOL SetPanelXml(UINT uItemID, LPCSTR lpszXml, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetPanelXml(uItemID, lpszXml, bRedraw);
	}

	BOOL SetItemText(UINT uItemID, LPCTSTR lpszText, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetItemText(uItemID, lpszText, bRedraw);
	}
	
	BOOL GetItemRect(UINT uItemID, RECT &rcItem)
	{
		return m_RefWin.GetItemRect(uItemID, rcItem);
	}

	BOOL SetItemAttribute(UINT uItemID, LPCSTR lpszAttrib, LPCSTR lpszValue, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetItemAttribute(uItemID, lpszAttrib, lpszValue, bRedraw);
	}

	BOOL SetItemIntAttribute(UINT uItemID, LPCSTR lpszAttrib, int nValue, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetItemIntAttribute(uItemID, lpszAttrib, nValue, bRedraw);
	}

	BOOL SetItemDWordAttribute(UINT uItemID, LPCSTR lpszAttrib, DWORD dwValue, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetItemDWordAttribute(uItemID, lpszAttrib, dwValue, bRedraw);
	}

	BOOL SetItemColorAttribute(UINT uItemID, LPCSTR lpszAttrib, COLORREF crValue, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetItemColorAttribute(uItemID, lpszAttrib, crValue, bRedraw);
	}

	BOOL GetItemCheck(UINT uItemID)
	{
		return m_RefWin.GetItemCheck(uItemID);
	}

	BOOL SetItemCheck(UINT uItemID, BOOL bCheck, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetItemCheck(uItemID, bCheck, bRedraw);
	}

	BOOL EnableItem(UINT uItemID, BOOL bEnable, BOOL bRedraw = TRUE)
	{
		return m_RefWin.EnableItem(uItemID, bEnable, bRedraw);
	}

	BOOL IsItemEnable(UINT uItemID, BOOL bCheckParent = FALSE)
	{
		return m_RefWin.IsItemEnable(uItemID, bCheckParent);
	}

	BOOL SetItemVisible(UINT uItemID, BOOL bVisible, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetItemVisible(uItemID, bVisible, bRedraw);
	}

	BOOL IsItemVisible(UINT uItemID, BOOL bCheckParent = FALSE)
	{
		return m_RefWin.IsItemVisible(uItemID, bCheckParent);
	}

	BOOL SetTabCurSel(UINT uItemID, int nPage, BOOL bRedraw = TRUE)
	{
		return m_RefWin.SetTabCurSel(uItemID, nPage, bRedraw);
	}

	HWND GetViewHWND()
	{
		return m_RefWin.GetViewHWND();
	}
	
	BOOL PostMessage( UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		return ::PostMessage( m_RefWin.m_hWnd, Msg, wParam, lParam );
	}

	BOOL ShowWindow( int nCmdShow)
	{
		return ::ShowWindow( m_RefWin.m_hWnd, nCmdShow );
	}

	UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
	{
		return ::SetTimer( m_RefWin.m_hWnd, nIDEvent, uElapse, lpTimerFunc );
	}

	BOOL KillTimer( UINT_PTR uIDEvent)
	{
		return ::KillTimer( m_RefWin.m_hWnd, uIDEvent );
	}
	
	void SetRangeItemVisible(int nBegin, int nEnd, int nCurrent)
	{
		ATLASSERT(nBegin<=nEnd);
		for(int i=0; i<=(nEnd-nBegin); ++i)
		{
			int n = i + nBegin;
			SetItemVisible( n, i==nCurrent, n==nEnd);
		}
	}
	
protected:
	T &m_RefWin;
};