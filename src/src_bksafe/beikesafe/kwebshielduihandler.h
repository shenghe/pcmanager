#pragma once

#include "kws/ikwebshieldui.h"
#include "bksafeuihandlerbase.h"

class CBeikeSafeMainDlg;

class CKWebShieldUIHandler 
    : public CBkNavigator
	, public CBkSafeUIHandlerBase
{
public:
    CKWebShieldUIHandler(CBeikeSafeMainDlg* refDialog)
        : m_dlg(refDialog)
        , m_piWebShieldUI(NULL)
        , m_hModule(NULL)
		, m_bMove(FALSE)
		, m_bInit(FALSE)
    {
    }

    virtual ~CKWebShieldUIHandler()
    {

    }

	int  InitCtrl(void);
	void InitCtrlDelay(void);
	void ReloadEngData(void);
	void ModifyMonitorState(int nType);

	LRESULT OnKwsShowSetting(void);
	LRESULT	OnKwsOpenDLSPage(void);

	void Show(BOOL bShow);

    virtual CBkNavigator* OnNavigate(CString &strChildName);

    void OnWebShieldReSize(CRect rcWnd);

	virtual STATE_CLOSE_CHECK	CloseCheck(){return CLOSE_CHECK_CONTINUE;}					


    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_REALWND_RESIZED(IDC_CTL_WEBSHIELD, OnWebShieldReSize)
    BK_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CKWebShieldUIHandler)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_WM_SIZE(OnSize)
    END_MSG_MAP()

protected:
    BOOL InitWebShieldUI(BOOL bPreLoad = FALSE);
    void UnInitWebShieldUI();
	void OnSize(UINT nType, CSize size);

private:
    CBeikeSafeMainDlg*  m_dlg;
    HMODULE             m_hModule;
    IKWebShieldUI*      m_piWebShieldUI;
    CWindow             m_wndWebShieldUI;
	CRect				m_rtClient;
	BOOL				m_bMove;
	BOOL				m_bInit;
};
