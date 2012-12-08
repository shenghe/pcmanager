#pragma once

#include <atlcoll.h>
#include <wtlhelper/whwindow.h>

class CBkSafeTip
	: public CBkDialogImpl<CBkSafeTip>
	, public CWHRoundRectFrameHelper<CBkSafeTip>
{
public:
	CBkSafeTip() 
		: m_bInit(FALSE),
		  m_bShow(FALSE)
	{

	}

	static CBkSafeTip& GetGlobalTip()
	{
		static CBkSafeTip	tips;
		return tips;
	}

	static BOOL Show(LPCWSTR lpszText,CPoint pt,HWND hWndParent=::GetActiveWindow())
	{
		return GetGlobalTip().ShowX(lpszText,pt,hWndParent);
	}

	static BOOL Hide()
	{
		return GetGlobalTip().HideX();
	}

	BOOL ShowX(LPCWSTR lpszText, CPoint pt,HWND hWndParent)
	{
		if (!Init(hWndParent))
			return FALSE;

		//HideX();

		CSize	sz = CalcSize(lpszText);
		SetItemText(1,lpszText);
		::SetWindowPos(m_hWnd,NULL,pt.x,pt.y,sz.cx,sz.cy,SWP_NOACTIVATE|SWP_SHOWWINDOW);
// 		CRect	rc(pt, sz);
// 		MoveWindow(&rc,FALSE);
// 		ShowWindow(SW_SHOW);
		SetTimer(1,5000);
		m_bShow = TRUE;
		return TRUE;
	}	

	BOOL HideX()
	{
		if (m_bShow)
		{
			ShowWindow(SW_HIDE);
			KillTimer(1);
			m_bShow=FALSE;
		}
		return TRUE;
	}
protected:

	CSize CalcSize(LPCWSTR lpszText)
	{
		CDCHandle	dchandle;
		CRect		rcProbe;
		HDC dcDesk = ::GetWindowDC( ::GetDesktopWindow() );

		dchandle.Attach(dcDesk);

		HFONT hFntOld = dchandle.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		dchandle.DrawText(lpszText,-1,&rcProbe,DT_CALCRECT);
		dchandle.SelectFont(hFntOld);

		dchandle.Detach();
		::ReleaseDC( ::GetDesktopWindow(), dcDesk );

		return CSize(rcProbe.Width()+7,rcProbe.Height()+9);
	}

protected:
	BOOL Init(HWND hParWnd)
	{
		if (m_bInit)
			return TRUE;

		if ( !SetXml("<layer><body width=full height=full><dlg pos=\"0,0,-0,-0\" crbg=7f7f7f><dlg pos=\"1,1,-1,-1\" class=tipbg><text pos=\"3,2\" id=1 /></dlg></dlg></body></layer>") )
			return FALSE;

		Create(hParWnd);

		m_bInit = TRUE;
		return TRUE;
	}
	void OnTimer(UINT_PTR nIdEvent)
	{
		if (nIdEvent==1)
		{
			HideX();
		}
	}

protected:
	BOOL	m_bInit;
	BOOL	m_bShow;

protected:

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam)
	{
		return S_OK;
	}

	void OnSize(UINT nType, CSize size)
	{
//		HRGN	hRgn = ::CreateRoundRectRgn(0,0,size.cx+1,size.cy+1,2,2);
		HRGN	hRgn = ::CreateRectRgn(0,0,size.cx,size.cy);
		::SetWindowRgn(m_hWnd,hRgn,FALSE);
		SetMsgHandled(FALSE);
	}

	BEGIN_MSG_MAP_EX(CBkSafeTip)
		CHAIN_MSG_MAP(CBkDialogImpl<CBkSafeTip>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkSafeTip>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SIZE(OnSize)
	END_MSG_MAP()
};
