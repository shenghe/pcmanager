#ifndef _SOFTMGR_QUERY_EDIT_H_
#define _SOFTMGR_QUERY_EDIT_H_

#define TIMER_ID_ON_CHAR_DELAY		1		

class INetMonitorLimitEditCallBack
{
public:
	virtual void OnEditEnterMsg( DWORD nEditId ) = 0;
	virtual void OnEditMouseWheelMsg(WPARAM wParam, LPARAM lParam) = 0;
};

class CBKNetMonitorLimitEdit: 
	public CWindowImpl<CBKNetMonitorLimitEdit, CEdit>
{
public:
	CBKNetMonitorLimitEdit()
	{
		m_strDefault	= _T("");
		m_bShowDefault	= TRUE;
		m_nDelayElaspe	= -1;
	}

public:
	BEGIN_MSG_MAP_EX(CBKNetMonitorLimitEdit)
// 		MSG_OCM_CTLCOLOREDIT(OnCtlColor)
// 		MSG_WM_KILLFOCUS(OnKillFocus)
// 		MSG_WM_SETFOCUS(OnSetFocus)
// 		MSG_WM_SETTEXT(OnSetText)
		MSG_WM_SETCURSOR(OnSetCursor)
// 		MSG_WM_CHAR(OnChar)
// 		MSG_WM_LBUTTONDOWN(OnLButtonDown)
// 		MSG_WM_TIMER(OnTimer)
// 		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
// 		if (::GetFocus() != m_hWnd )
// 		{
// 			DefWindowProc();
// 			SetMsgHandled(TRUE);
// 			SetSel(0,-1);
// 		}
// 		else
		{
			CString		strX;
			GetWindowText(strX);
			SetSel(strX.GetLength(),-1);
			SetMsgHandled(FALSE);
		}
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		if (nIDEvent==TIMER_ID_ON_CHAR_DELAY)
		{
			CString		strX;
			GetWindowText(strX);

			if (strX != m_strLastSearchValue )
			{	
				m_strLastSearchValue = strX;
				m_pEditCB->OnEditEnterMsg( GetDlgCtrlID() );
			}

			KillTimer(nIDEvent);
			return;
		}
		SetMsgHandled(FALSE);
	}

	LRESULT OnSetCursor(HWND hwndCtrl, UINT uHitTest, UINT uMouseMsg)
	{
		static HCURSOR hcur = LoadCursor ( NULL, IDC_IBEAM );
		if ( NULL != hcur )
		{
			SetCursor ( hcur );
			return TRUE;
		}
		else
		{
			SetMsgHandled(FALSE);
			return FALSE;
		}
	}

	LRESULT OnSetText( LPCWSTR szText )
	{
		if ( m_strDefault != szText)
		{ 
			m_bShowDefault = FALSE;
		}
		else
		{
			m_bShowDefault = TRUE;
		}

		SetMsgHandled(FALSE);
		return 0;
	//	return DefWindowProc( WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
	}

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
	{
		m_pEditCB->OnEditMouseWheelMsg(MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));

		return FALSE;
	}

	void OnSetFocus(HWND hWnd) 
	{
		m_bShowDefault = FALSE;

		SetMsgHandled(FALSE);

		CString strCurrent;
		GetWindowText( strCurrent);
		if ( m_strDefault == strCurrent)
		{
			SetWindowText( _T(""));
		}
	}

	void OnKillFocus(HWND hWnd) 
	{
		CString strCurrent;
		GetWindowText( strCurrent);
		if ( strCurrent.IsEmpty() )
		{
			m_bShowDefault = TRUE;
			SetWindowText( m_strDefault);
		}
		SetMsgHandled(FALSE);
	}

	LRESULT	OnCtlColor(HDC hDc, HWND hWnd)
	{
		if(hWnd == m_hWnd)
		{
			CDCHandle	dc;
			dc.Attach(hDc);
			if (m_bShowDefault)
			{
				dc.SetTextColor( RGB(102, 102, 102) );
			}
			else
			{
				dc.SetTextColor( RGB(0, 0, 0));
			}
			dc.Detach();
			SetMsgHandled(TRUE);
			return (LRESULT)GetStockObject(WHITE_BRUSH);
		}
		SetMsgHandled(FALSE);
		return NULL;
	}

	VOID SetDefaultString( LPCWSTR szDefault )
	{
		m_strDefault = szDefault;
		SetWindowText( m_strDefault);
		m_bShowDefault = TRUE;
		SetWindowText( _T(""));
		OnKillFocus(m_hWnd);
	}


	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (VK_RETURN == nChar)
		{
			CString		strX;
			GetWindowText(strX);

			if (strX != m_strLastSearchValue )
			{
				m_strLastSearchValue = strX;
			}

			m_pEditCB->OnEditEnterMsg( GetDlgCtrlID() );
			SetFocus();
			SetMsgHandled(TRUE);
			KillTimer(TIMER_ID_ON_CHAR_DELAY);
			return;
		}
		else
		{
			if ((nChar >= 0x30 && nChar <= 0x39) || 
				VK_ESCAPE == nChar )
			{
				if (m_nDelayElaspe != -1)
					SetTimer(TIMER_ID_ON_CHAR_DELAY,m_nDelayElaspe,NULL);

				SetMsgHandled(FALSE);
			}
			else
				return;
		}
	}

	void SetCallBack(INetMonitorLimitEditCallBack *pCB)
	{
		m_pEditCB = pCB;
	}

	void SetOnCharDelayTime(DWORD nTime=500)
	{
		m_nDelayElaspe = nTime;
	}

private:
	CString		m_strDefault;
	BOOL		m_bShowDefault;
	INetMonitorLimitEditCallBack *m_pEditCB;
	DWORD		m_nDelayElaspe;
	CString		m_strLastSearchValue;
};


#endif