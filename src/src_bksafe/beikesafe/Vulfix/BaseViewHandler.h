#pragma once

template<typename T>
class CBaseViewHandler
{
public:
	CBaseViewHandler(T &refWin) : m_RefWin(refWin) 
	{
	}
	
	BOOL SetPanelXml(UINT uItemID, UINT uResID)
	{
		return m_RefWin.SetPanelXml(uItemID, uResID);
	}

	BOOL SetPanelXml(UINT uItemID, LPCSTR lpszXml)
	{
		return m_RefWin.SetPanelXml(uItemID, lpszXml);
	}

	BOOL SetItemText(UINT uItemID, LPCTSTR lpszText)
	{
		return m_RefWin.SetItemText(uItemID, lpszText);
	}
	
	BOOL GetItemRect(UINT uItemID, RECT &rcItem)
	{
		return m_RefWin.GetItemRect(uItemID, rcItem);
	}

	BOOL SetItemAttribute(UINT uItemID, LPCSTR lpszAttrib, LPCSTR lpszValue)
	{
		return m_RefWin.SetItemAttribute(uItemID, lpszAttrib, lpszValue);
	}

	BOOL SetItemIntAttribute(UINT uItemID, LPCSTR lpszAttrib, int nValue)
	{
		return m_RefWin.SetItemIntAttribute(uItemID, lpszAttrib, nValue);
	}

	BOOL SetItemDWordAttribute(UINT uItemID, LPCSTR lpszAttrib, DWORD dwValue)
	{
		return m_RefWin.SetItemDWordAttribute(uItemID, lpszAttrib, dwValue);
	}

	BOOL SetItemColorAttribute(UINT uItemID, LPCSTR lpszAttrib, COLORREF crValue)
	{
		return m_RefWin.SetItemColorAttribute(uItemID, lpszAttrib, crValue);
	}

	BOOL GetItemCheck(UINT uItemID)
	{
		return m_RefWin.GetItemCheck(uItemID);
	}

	BOOL SetItemCheck(UINT uItemID, BOOL bCheck)
	{
		return m_RefWin.SetItemCheck(uItemID, bCheck);
	}

	BOOL EnableItem(UINT uItemID, BOOL bEnable)
	{
		return m_RefWin.EnableItem(uItemID, bEnable);
	}

	BOOL IsItemEnable(UINT uItemID, BOOL bCheckParent = FALSE)
	{
		return m_RefWin.IsItemEnable(uItemID, bCheckParent);
	}

	BOOL SetItemVisible(UINT uItemID, BOOL bVisible)
	{
		return m_RefWin.SetItemVisible(uItemID, bVisible);
	}

	BOOL IsItemVisible(UINT uItemID, BOOL bCheckParent = FALSE)
	{
		return m_RefWin.IsItemVisible(uItemID, bCheckParent);
	}

	BOOL SetRichText(UINT uItemID, UINT uResID)
	{
		return m_RefWin.SetRichText(uItemID, uResID);
	}

	BOOL SetRichText(UINT uItemID, LPCWSTR lpszXml)
	{
		return m_RefWin.SetRichText(uItemID, lpszXml);
	}

	BOOL SetRichText(UINT uItemID, LPCSTR lpszXml)
	{
		return m_RefWin.SetRichText(uItemID, lpszXml);
	}
	
	BOOL SetTabCurSel(UINT uItemID, int nPage)
	{
		return m_RefWin.SetTabCurSel(uItemID, nPage);
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

    void Redraw()
    {
        m_RefWin.Redraw();
    }
	
protected:
	T &m_RefWin;
};
