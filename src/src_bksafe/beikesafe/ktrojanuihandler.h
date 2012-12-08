#pragma once

//////////////////////////////////////////////////////////////////////////

#include "trojan/iTrojan.h"//放到公共地方
#include "bksafeuihandlerbase.h"
//////////////////////////////////////////////////////////////////////////

class CBeikeSafeMainDlg;
class CKTrojanUIHandler 
	: public CBkNavigator
	, public CBkSafeUIHandlerBase
{
public:
	CKTrojanUIHandler(CBeikeSafeMainDlg* refDialog)
		: m_dlg(refDialog)
		, m_piKTrajon(NULL)
		, m_hKTrajon(NULL)
		, m_bMove(FALSE)
		, m_bInit(FALSE)
	{
	}

	virtual ~CKTrojanUIHandler()
	{
	}
 	
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_REALWND_RESIZED(IDC_CTL_KTROJAN, OnKTrojanReSize)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CKTrojanUIHandler)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		 MSG_WM_SIZE(OnSize)
	END_MSG_MAP()

public:
	void OnKTrojanReSize(CRect rcWnd);
	void Init();
	void UnInit();
	void Show(BOOL bShow);
	virtual CBkNavigator* OnNavigate(CString &strChildName);
	virtual STATE_CLOSE_CHECK	CloseCheck();
	HWND GetTrojanHwnd();
	virtual VOID			CloseSuccess(BOOL bSucClose);
protected:
	BOOL InitKTrojan(BOOL bPreLoad = FALSE);
	void UnInitKTrojan();
	void OnSize(UINT nType, CSize size);
private:
	CBeikeSafeMainDlg*  m_dlg;
	HMODULE             m_hKTrajon;
	IKTrojan*           m_piKTrajon;
	CWindow             m_wndKTrojan;//
	CRect				m_rtClient;
	BOOL				m_bMove;
	BOOL				m_bInit;
};

//////////////////////////////////////////////////////////////////////////