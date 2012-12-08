#include "stdafx.h"
#include "kscmaindlg.h"
#include "kscbase/kscres.h"
#include "kscbase/kscfilepath.h"
#include "kscbase/kscconf.h"
#include "kuiwin/kuimsgbox.h"
#include "kclearnavigate.h"
#include "../../publish/bksafe/bksafenavigatestring.h"
#include "dlgsetting.h"
#include "kscbase/kcslogging.h"

//////////////////////////////////////////////////////////////////////////

CKscMainDlg::CKscMainDlg() 
    : CKuiDialogImpl<CKscMainDlg>("IDR_DLG_CLEAR")
    , m_handlerOpt(this)
    , m_handlerOnekey(this)
    , m_handlerBigFile(this)
	, m_handlerTrashClean(this)
    , m_handlerSysSlim(this)
    , m_nDefaultTab(1)
	, m_bInitOpt(FALSE)
	, m_bInitOneKey(FALSE)
	, m_bInitTrashClean(FALSE)
	, m_bInitBigFind(FALSE)
    , m_bExamNeedScan(FALSE)
{
}

CKscMainDlg::~CKscMainDlg()
{
}

//////////////////////////////////////////////////////////////////////////

typedef void (__cdecl *SetCareWindow_t)(HWND hWnd);

//////////////////////////////////////////////////////////////////////////

BOOL CKscMainDlg::DisableTab(const CString& strTab)
{
    if (_T("bigfile") == strTab)
    {
        m_disabledTab[_T("bigfile")] = TRUE;
    }

    if (_T("regclr") == strTab)
    {
        m_disabledTab[_T("regclr")] = TRUE;
    }

    if (_T("trackclr") == strTab)
    {
        m_disabledTab[_T("trackclr")] = TRUE;
    }

    if (_T("trashclr") == strTab)
    {
        m_disabledTab[_T("trashclr")] = TRUE;
    }

    if (_T("onekey") == strTab)
    {
        m_disabledTab[_T("onekey")] = TRUE;
    }

    if (_T("systemslim") == strTab)
    {
        m_disabledTab[_T("systemslim")] = TRUE;
    }

    return TRUE;
}

BOOL CKscMainDlg::IsTabDisabled(const CString& strTab)
{
    return (m_disabledTab.find(strTab) != m_disabledTab.end());
}

BOOL CKscMainDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
	CString	str;
	KAppRes& appRes = KAppRes::Instance();
    CKcsLogging kcsLogging;
    // 清理过期的日志
    kcsLogging.ClearLogFileByDays();

    SetIcon(appRes.GetIcon("IDI_CLEAR_ICON_BIG"), TRUE);
    SetIcon(appRes.GetIcon("IDI_CLEAR_ICON_SMALL"), FALSE);

	RECT rcTest = { 1, 1, 20, 20 };
	m_wndToolVector.Create(GetRichWnd(), &rcTest, NULL, 0, 0, 3888);

	m_wndToolVector.InitTools();

    if (IsTabDisabled(_T("systemslim")))
    {
        DeleteTab(IDC_TAB_MAIN, 5);
    }
    else
    {
        m_handlerSysSlim.Init();
    }

    if (IsTabDisabled(_T("bigfile")))
    {
        DeleteTab(IDC_TAB_MAIN, 4);
    }
    else
    {
        m_handlerBigFile.Init();
    }

    if (IsTabDisabled(_T("regclr")))
    {
        DeleteTab(IDC_TAB_MAIN, 3);
    }
    else
    {

    }

    if (IsTabDisabled(_T("trackclr")))
    {
        DeleteTab(IDC_TAB_MAIN, 2);
    }
    else
    {
        m_handlerOpt.Init();
    }

    if (IsTabDisabled(_T("trashclr")))
    {
        DeleteTab(IDC_TAB_MAIN, 1);
    }
    else
    {
        m_handlerTrashClean.Init();
    }
    
    if (IsTabDisabled(_T("onekey")))
    {
        DeleteTab(IDC_TAB_MAIN, 0);
    }
    else
    {
        m_handlerOnekey.Init();
    }

    SetTabCurSel(IDC_TAB_MAIN, m_nDefaultTab);

	SetItemText(IDC_TXT_PRODUCT_VERSION, _Module.GetAppFileVersion());

	g_hWndKClear = m_hWnd;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL CKscMainDlg::OnTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew)
{
	switch ( nTabItemIDNew )
	{
	case 0:
		if( m_bInitOneKey == FALSE )
		{
			m_bInitOneKey = TRUE;
			m_handlerOnekey.InitDelay();

            if (FALSE == m_bInitOpt)
            {
                m_bInitOpt = TRUE;
                m_handlerOpt.InitDelay();
            }
		}
		break;
	case 1:
		//if( m_bInitTrashClean == FALSE )
		//{
		//	m_bInitTrashClean = TRUE;
		//	m_handlerTrashClean.InitDelay();
		//}
        if ( m_bExamNeedScan == TRUE )
        {
            if (m_handlerTrashClean.IsFinishInitilize())
            {
                m_handlerTrashClean.OnRescan();
            }
            else
            {
                m_handlerTrashClean.SetFinishExamNeedScan();
            }
            m_bExamNeedScan = FALSE;
        }
		break;
	case 2:
		if( m_bInitOpt == FALSE )
		{
			m_bInitOpt = TRUE;
			m_handlerOpt.InitDelay();
		}
		break;
	case 3:
		break;
	case 4:
		if( m_bInitBigFind == FALSE )
		{
			m_bInitBigFind = TRUE;
			m_handlerBigFile.InitDelay();
		}
		break;
	}

    if (5 == nTabItemIDNew)
    {
        m_handlerSysSlim.OnShow();
    }

	if (4 == nTabItemIDNew)
	{
		m_handlerBigFile.Show();
	}

	return m_wndToolVector.SwitchTool(nTabItemIDNew);
}

void CKscMainDlg::OnSize(UINT nType, CSize size)
{
	UpdateToolSize();
}

//////////////////////////////////////////////////////////////////////////

BOOL CKscMainDlg::UpdateToolSize()
{
	BOOL retval = FALSE;
	CRect rcTool;

	if (!m_wndToolVector.m_hWnd)
		goto clean0;

	GetClientRect(rcTool);
	//rcTool.left += 7;
	rcTool.top += 28;
	//rcTool.right -= 7;
	rcTool.bottom -= 0;
	m_wndToolVector.SetWindowPos(HWND_TOP, &rcTool, 0);

	retval = TRUE;

clean0:
	return retval;
}

//////////////////////////////////////////////////////////////////////////

BOOL CKscMainDlg::OnNavigate(const CString& strNavigate)
{
    BOOL retval = FALSE;
    int nTabIndex = -1;

    if (!strNavigate.CompareNoCase(KCLEARNS_ONEKEY) ||
        !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_ONEKEY))
    {
        nTabIndex = 0;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_TRASHCLEANER) ||
             !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_RUBBISH))
    {
        nTabIndex = 1;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_TRACKCLEANER) ||
             !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_HENJI))
    {
        nTabIndex = 2;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_REGCLEANER) ||
             !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_REG))
    {
        nTabIndex = 3;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_BIGCLEANER) ||
             !strNavigate.CompareNoCase(BKSFNS_SYSOPT_BIG_FILE))
    {
        nTabIndex = 4;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_SYSTEMSLIM) ||
             !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_SHOUSHEN))
    {
        nTabIndex = 5;
    }
    else if (!strNavigate.CompareNoCase(L"clrrubscan"))
    {
        nTabIndex = 1;
        m_bExamNeedScan = TRUE;
    }

    if (-1 == nTabIndex)
        goto clean0;

    retval = SetTabCurSel(IDC_TAB_MAIN, nTabIndex);

clean0:
    return retval;
}

BOOL CKscMainDlg::PreNavigate(const CString& strNavigate)
{
    BOOL retval = FALSE;
    int nTabIndex = -1;

    if (!strNavigate.CompareNoCase(KCLEARNS_ONEKEY) ||
        !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_ONEKEY))
    {
        nTabIndex = 0;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_TRASHCLEANER) ||
        !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_RUBBISH))
    {
        nTabIndex = 1;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_TRACKCLEANER) ||
        !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_HENJI))
    {
        nTabIndex = 2;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_REGCLEANER) ||
        !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_REG))
    {
        nTabIndex = 3;
    }
    else if (!strNavigate.CompareNoCase(KCLEARNS_BIGCLEANER) ||
        !strNavigate.CompareNoCase(BKSFNS_SYSOPT_CLR_SHOUSHEN))
    {
        nTabIndex = 4;
    }

    if (-1 == nTabIndex)
        goto clean0;

    m_nDefaultTab = nTabIndex;

clean0:
    return retval;
}

void CKscMainDlg::OnDestroy()
{
    m_handlerOnekey.UnInit();
    m_handlerBigFile.UnInit();
    m_handlerOpt.UnInit();
    m_handlerTrashClean.UnInit();
    m_handlerSysSlim.UnInit();
}

void CKscMainDlg::OnSetting()
{
    KClearSettingDlg dlgSetting;
    if (dlgSetting.DoModal(GetParent().m_hWnd) == IDOK)
	{
		::SendMessage(m_hWnd, WM_CLEAR_SETTIING, NULL, NULL);
	}
}

//////////////////////////////////////////////////////////////////////////