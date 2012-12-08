#pragma once

//////////////////////////////////////////////////////////////////////////

#include "../../import/kclear_include/kclear/ikclear.h"
#include "bksafeuihandlerbase.h"
//////////////////////////////////////////////////////////////////////////

class CBeikeSafeMainDlg;
class CKClearUIHandler 
    : public CBkNavigator
	, public CBkSafeUIHandlerBase
{
public:
    CKClearUIHandler(CBeikeSafeMainDlg* refDialog)
        : m_dlg(refDialog)
        , m_piKClear(NULL)
        , m_hKClear(NULL)
		, m_bMove(FALSE)
		, m_bInit(FALSE)
    {
    }

    virtual ~CKClearUIHandler()
    {
    }

    void PreLoad(); // т╓ох╪сть

    void Init();
    void UnInit();
    void Show();
    void Hide();

    virtual CBkNavigator* OnNavigate(CString &strChildName);

    void OnKClearReSize(CRect rcWnd);

	virtual STATE_CLOSE_CHECK	CloseCheck(){return CLOSE_CHECK_CONTINUE;}					
	virtual VOID				CloseSuccess(BOOL bSucClose);


    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_REALWND_RESIZED(IDC_CTL_KCLEAR, OnKClearReSize)
    BK_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CKClearUIHandler)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
    END_MSG_MAP()

protected:
    BOOL InitKClear(BOOL bPreLoad = FALSE);
    void UnInitKClear();
    static UINT WINAPI PreLoadThread(LPVOID pParam);

private:
    CBeikeSafeMainDlg*  m_dlg;
    HMODULE             m_hKClear;
    IKClear*            m_piKClear;
    CWindow             m_wndKClear;
	CRect				m_rtClient;
	BOOL				m_bMove;
	BOOL				m_bInit;
};

//////////////////////////////////////////////////////////////////////////
