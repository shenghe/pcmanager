#include "stdafx.h"
#include "beikesafemsgbox.h"
#include "bkres/bkres.h"

#define IDC_LBL_MSGBOX_TITLE                60004
#define IDC_ICON_MSGBOX_ICON                60005
#define IDC_LBL_MSGBOX_TEXT                 60006
#define IDC_CHK_MSGBOX_NONOTIFYLATER        60007
#define IDC_DIV_MSGBOX_BUTTONS              60008
#define IDC_DIV_MSGBOX_CONTENT              60009

#define IDS_MSGBOX_BUTTON_XML                   (300+17)
#define IDS_MSGBOX_OK                           (300+18)
#define IDS_MSGBOX_CANCEL                       (300+19)
#define IDS_MSGBOX_ABORT                        (300+20)
#define IDS_MSGBOX_RETRY                        (300+21)
#define IDS_MSGBOX_IGNORE                       (300+22)
#define IDS_MSGBOX_YES                          (300+23)
#define IDS_MSGBOX_NO                           (300+24)
#define IDS_MSGBOX_CONTINUE                     (300+25)
#define IDS_MSGBOX_BUTTON_DIV                   (300+26)

UINT_PTR CBkSafeMsgBox::Show(
    LPCWSTR lpszText, 
    LPCWSTR lpszCaption     /*= NULL*/, 
    UINT uType              /*= MB_OK*/, 
    BOOL *pbNoNotifyLater   /*= NULL*/, 
    HWND hWndParent         /*= ::GetActiveWindow()*/)
{
    CBkSafeMsgBox msgbox;

    return msgbox.ShowMsg(lpszText, lpszCaption, uType, pbNoNotifyLater, hWndParent);
}

void CBkSafeMsgBox::AddButton(LPCWSTR lpszText, UINT uCmdID, UINT uWidth /*= 0*/)
{
    if (0 == uWidth)
    {
        CString strText = lpszText;
        CWindowDC dc(::GetDesktopWindow());
        CRect rcText(0, 0, 1000, 1000);

        const BkStyle& btnStyle = BkStyle::GetStyle("normalbtn");

        HFONT hftOld = dc.SelectFont(btnStyle.m_ftText);

        dc.DrawText(strText, strText.GetLength(), rcText, btnStyle.m_nTextAlign | DT_CALCRECT);

        rcText.InflateRect(btnStyle.m_nMarginX, btnStyle.m_nMarginY);

        dc.SelectFont(hftOld);

        uWidth = max(80, rcText.Width());
    }

    m_lstButton.AddTail(__BKMBButton(lpszText, uCmdID, uWidth));
}


UINT_PTR CBkSafeMsgBox::ShowMsg(
								LPCWSTR lpszText, 
								LPCWSTR lpszCaption     /*= NULL*/, 
								LPCWSTR lpszCheckCaption,
								UINT uType              /*= MB_OK*/, 
								BOOL *pbNoNotifyLater   /*= NULL*/, 
								HWND hWndParent         /*= ::GetActiveWindow()*/)
{
	if (!Load(IDR_BK_MESSAGEBOXEX))
		return -1;

	m_isTextXml = FALSE;
	m_strText = lpszText;
	m_uType = uType;

	SetItemVisible(60016, FALSE);
	SetItemVisible(IDC_LBL_MSGBOX_TEXT, TRUE);
	SetItemText(IDC_LBL_MSGBOX_TEXT, lpszText);
	if (lpszCaption)
	{
		m_strCaption    = lpszCaption;
		SetWindowCaption(m_strCaption);

		SetItemText(IDC_LBL_MSGBOX_TITLE, lpszCaption);
	}

	if ( pbNoNotifyLater != NULL )
	{
		SetItemCheck(IDC_CHK_MSGBOX_NONOTIFYLATER, *pbNoNotifyLater);
		if ( lpszCheckCaption != NULL )
		{
			SetItemText(IDC_CHK_MSGBOX_NONOTIFYLATER, lpszCheckCaption);
		}
	}

	_InitLayout(pbNoNotifyLater);

	UINT_PTR nRet = DoModal(hWndParent);
	if (pbNoNotifyLater)
	{
		*pbNoNotifyLater = m_bNoNotifyLater;
	}

	return nRet;
}

UINT_PTR CBkSafeMsgBox::ShowMsg(
    LPCWSTR lpszText, 
    LPCWSTR lpszCaption     /*= NULL*/, 
    UINT uType              /*= MB_OK*/, 
    BOOL *pbNoNotifyLater   /*= NULL*/, 
    HWND hWndParent         /*= ::GetActiveWindow()*/)
{
    return ShowMsg(lpszText, lpszCaption, NULL, uType, pbNoNotifyLater, hWndParent);
}

UINT_PTR CBkSafeMsgBox::ShowPanelMsg( LPCWSTR lpszXml, LPCRECT lpRect
									 , LPCWSTR lpszCaption /*= NULL*/, UINT uType /*= MB_OK*/, BOOL *pbNoNotifyLater /*= NULL*/, HWND hWndParent /*= NULL */ )
{
	ATLASSERT(lpRect);
	if (!lpRect || !Load(IDR_BK_MESSAGEBOXEX))
		return -1;
	
	m_isTextXml = TRUE;
	m_rcXml = *lpRect;
	m_uType = uType;
	SetItemVisible(60016, TRUE);
	SetItemVisible(IDC_LBL_MSGBOX_TEXT, FALSE);
	SetPanelXml(60016, CT2CA(lpszXml, CP_UTF8));
	if (lpszCaption)
	{
		m_strCaption    = lpszCaption;
		SetWindowCaption(m_strCaption);
		SetItemText(IDC_LBL_MSGBOX_TITLE, lpszCaption);
	}
	_InitLayout(pbNoNotifyLater);

	UINT_PTR nRet = DoModal(hWndParent);
	if (pbNoNotifyLater)
		*pbNoNotifyLater = m_bNoNotifyLater;

	return nRet;
}

void CBkSafeMsgBox::_InitLayout( BOOL * pbNoNotifyLater )
{
	CString strButtonXmlFormat, strButtons, strButtonDiv;

	_MakeButtons();

	strButtonXmlFormat = BkString::Get(IDS_MSGBOX_BUTTON_XML);

#   define BUTTON_SPACING 20
	UINT uWidth = 0, uLeft = 0;
	POSITION pos = m_lstButton.GetHeadPosition();


	while (pos)
	{
		const __BKMBButton &btn = m_lstButton.GetNext(pos);

		uWidth += btn.uWidth;
		if (pos)
			uWidth += BUTTON_SPACING;
	}

	pos = m_lstButton.GetHeadPosition();

	while (pos)
	{
		const __BKMBButton &btn = m_lstButton.GetNext(pos);

		strButtons.AppendFormat(strButtonXmlFormat, btn.uCmdID, uLeft, uLeft + btn.uWidth, btn.strText);

		uLeft += btn.uWidth + BUTTON_SPACING;
	}

	strButtonDiv.Format(BkString::Get(IDS_MSGBOX_BUTTON_DIV), uWidth / 2, uWidth / 2, strButtons);
	SetPanelXml(IDC_DIV_MSGBOX_BUTTONS, CT2A(strButtonDiv, CP_UTF8));

	DWORD dwIconID = 0;

	if (0 != (MB_ICONMASK & m_uType))
	{
		switch (MB_ICONMASK & m_uType)
		{
		case MB_ICONHAND:
			dwIconID = (DWORD)(DWORD_PTR)IDI_HAND;
			break;
		case MB_ICONQUESTION:
			dwIconID = (DWORD)(DWORD_PTR)IDI_QUESTION;
			break;
		case MB_ICONEXCLAMATION:
			dwIconID = (DWORD)(DWORD_PTR)IDI_EXCLAMATION;
			break;
		case MB_ICONASTERISK:
			dwIconID = (DWORD)(DWORD_PTR)IDI_ASTERISK;
			break;
		default:
            SetItemDWordAttribute(IDC_ICON_MSGBOX_ICON, "src", dwIconID);
            SetItemDWordAttribute(IDC_ICON_MSGBOX_ICON, "oem", 0);
		}

		if (0 != dwIconID)
			SetItemDWordAttribute(IDC_ICON_MSGBOX_ICON, "src", dwIconID);
	}

	if (0 == dwIconID)
		SetItemVisible(IDC_ICON_MSGBOX_ICON, FALSE);

	if (NULL == pbNoNotifyLater)
		SetItemVisible(IDC_CHK_MSGBOX_NONOTIFYLATER, FALSE);
	else
	{
		m_bShowNoNotifyLater = TRUE;
	}
}


void CBkSafeMsgBox::_MakeButtons()
{
    if ((MB_BK_CUSTOM_BUTTON & m_uType) == MB_BK_CUSTOM_BUTTON)
        return;

    m_lstButton.RemoveAll();

    switch (MB_TYPEMASK & m_uType)
    {
    case MB_OK:
        AddButton(BkString::Get(IDS_MSGBOX_OK), IDOK);
        break;
    case MB_OKCANCEL:
        AddButton(BkString::Get(IDS_MSGBOX_OK), IDOK);
        AddButton(BkString::Get(IDS_MSGBOX_CANCEL), IDCANCEL);
        break;
    case MB_ABORTRETRYIGNORE:
        AddButton(BkString::Get(IDS_MSGBOX_ABORT), IDABORT);
        AddButton(BkString::Get(IDS_MSGBOX_RETRY), IDRETRY);
        AddButton(BkString::Get(IDS_MSGBOX_IGNORE), IDIGNORE);
        break;
    case MB_YESNOCANCEL:
        AddButton(BkString::Get(IDS_MSGBOX_YES), IDYES);
        AddButton(BkString::Get(IDS_MSGBOX_NO), IDNO);
        AddButton(BkString::Get(IDS_MSGBOX_CANCEL), IDCANCEL);
        break;
    case MB_YESNO:
        AddButton(BkString::Get(IDS_MSGBOX_YES), IDYES);
        AddButton(BkString::Get(IDS_MSGBOX_NO), IDNO);
        break;
    case MB_RETRYCANCEL:
        AddButton(BkString::Get(IDS_MSGBOX_RETRY), IDRETRY);
        AddButton(BkString::Get(IDS_MSGBOX_CANCEL), IDCANCEL);
        break;
#   if(WINVER >= 0x0500)
    case MB_CANCELTRYCONTINUE:
        AddButton(BkString::Get(IDS_MSGBOX_CANCEL), IDCANCEL);
        AddButton(BkString::Get(IDS_MSGBOX_RETRY), IDRETRY);
        AddButton(BkString::Get(IDS_MSGBOX_CONTINUE), IDCONTINUE);
        break;
#   endif /* WINVER >= 0x0500 */
    }
}

void CBkSafeMsgBox::OnBkCommand(UINT uItemID, LPCSTR szItemClass)
{
    if (strcmp(CBkButton::GetClassName(), szItemClass) != 0 && strcmp(CBkImageBtnWnd::GetClassName(), szItemClass) != 0)
    {
        return;
    }

	m_bNoNotifyLater = GetItemCheck(IDC_CHK_MSGBOX_NONOTIFYLATER);
//	::ShowWindow(m_hWnd, SW_HIDE);
//	::SetTimer(m_hWnd, 1989, 1000, NULL); // 触发消息循环

    EndDialog(uItemID);
}

LRESULT CBkSafeMsgBox::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    CWindowDC dc(m_hWnd);
    CRect rcClient, rcCaption, rcText;

    if (m_strCaption.IsEmpty())
        GetWindowText(m_strCaption);

    HMONITOR hMonitor = ::MonitorFromWindow(GetParent(), MONITOR_DEFAULTTONULL/* MONITOR_DEFAULTTONEAREST*/);

    if (hMonitor)
    {
        MONITORINFO mi = {sizeof(MONITORINFO)};
        ::GetMonitorInfo(hMonitor, &mi);

        rcClient = mi.rcWork;
    }
    else
        GetClientRect(rcClient);


	const BkStyle& titleStyle = BkStyle::GetStyle("dlgtitle");
	HFONT hftOld = dc.SelectFont(titleStyle.m_ftText);

	rcCaption = rcClient;
	rcCaption.MoveToXY(0, 0);
	dc.DrawText(m_strCaption, m_strCaption.GetLength(), rcCaption, DT_CALCRECT);
	rcCaption.right += 100;
	
	if(m_isTextXml)
	{
		rcText = m_rcXml;
	}
	else
	{
		rcText = rcClient;
		rcText.MoveToXY(0, 0);

		const BkStyle& textStyle = BkStyle::GetStyle("msgtext");

		dc.SelectFont(textStyle.m_ftText);

		dc.DrawText(m_strText, m_strText.GetLength(), rcText, textStyle.m_nTextAlign | DT_CALCRECT);
	}
	rcText.right += 100;
	rcText.bottom += 120;

    if (m_bShowNoNotifyLater)
	{
        rcText.bottom += 20;
	}

	if ( m_nFuckMode == 1 )
	{
		SetItemVisible(10001, FALSE);
		SetItemVisible(10002, TRUE);
	}
	
    dc.SelectFont(hftOld);

	rcClient.right = rcClient.left + max(max(rcCaption.right, rcText.right), 320);
    rcClient.bottom = rcClient.top + max(rcText.bottom, 160);

    MoveWindow(rcClient, FALSE);

	

    return TRUE;
}

void CBkSafeMsgBox::OnTimer(UINT_PTR nIDEvent)
{
	if ( 1989 == nIDEvent )
	{
		PostMessage(WM_NULL);
	}
}



//////////////////////////////////////////////////////////////////////////
UINT_PTR CBkSafeMsgBox2::ShowPanelMsg( LPCWSTR lpszXml, LPCRECT lpRect
									  , LPCWSTR lpszCaption /*= NULL*/, UINT uType /*= MB_OK*/, BOOL *pbNoNotifyLater /*= NULL*/, HWND hWndParent /*= NULL */ )
{
	ATLASSERT(lpRect);
	if (!lpRect || !Load(IDR_BK_MESSAGEBOX2))
		return -1;

	m_isTextXml = TRUE;
	m_rcXml = *lpRect;
	m_uType = uType;
	SetItemVisible(60016, TRUE);
	SetItemVisible(IDC_LBL_MSGBOX_TEXT, FALSE);
	SetPanelXml(60016, CT2CA(lpszXml, CP_UTF8));
	if (lpszCaption)
	{
		m_strCaption    = lpszCaption;
		SetWindowCaption(m_strCaption);
		SetItemText(IDC_LBL_MSGBOX_TITLE, lpszCaption);
	}
	_InitLayout(pbNoNotifyLater);

	UINT_PTR nRet = DoModal(hWndParent);
	if (pbNoNotifyLater)
		*pbNoNotifyLater = m_bNoNotifyLater;

	return nRet;
}

UINT_PTR CBkSafeMsgBox2::ShowMsg(
								 LPCWSTR lpszText, 
								 LPCWSTR lpszCaption     /*= NULL*/, 
								 UINT uType              /*= MB_OK*/, 
								 BOOL *pbNoNotifyLater   /*= NULL*/, 
								 HWND hWndParent         /*= ::GetActiveWindow()*/)
{
	/*
	if (!Load(IDR_BK_MESSAGEBOX2))
	return -1;

	m_isTextXml = FALSE;
	m_strText = lpszText;
	m_uType = uType;
	SetItemVisible(60016, FALSE);
	SetItemVisible(IDC_LBL_MSGBOX_TEXT, TRUE);
	SetItemText(IDC_LBL_MSGBOX_TEXT, lpszText);
	if (lpszCaption)
	{
	m_strCaption    = lpszCaption;
	SetWindowCaption(m_strCaption);

	SetItemText(IDC_LBL_MSGBOX_TITLE, lpszCaption);
	}
	_InitLayout(pbNoNotifyLater);

	UINT_PTR nRet = DoModal(hWndParent);
	if (pbNoNotifyLater)
	*pbNoNotifyLater = m_bNoNotifyLater;
	return nRet;
	*/

	return ShowMutlLineMsg(lpszText, lpszCaption, uType, pbNoNotifyLater, hWndParent);
}

UINT_PTR CBkSafeMsgBox2::ShowMutlLineMsg(LPCWSTR pszMsg, 
										 LPCWSTR lpszCaption /* = NULL */, 
										 UINT uType /* = NULL */, 
										 BOOL *pbNoNotifyLater /* = NULL */, 
										 HWND hWndParent /* = ::GetActiveWindow */)
{
	CString strMsg = pszMsg, strXml(L""), strXmlPanel(L"");
	CAtlArray<CString> arrMsgLine;
	if ( 0 == ParseMsgLine(strMsg, arrMsgLine))
		return -1;

	int nWidth = 0, 
		nLineCount = arrMsgLine.GetCount(), 
		nHeight =  nLineCount* 18,
		nTop = 0;
	CRect rcText;

	for (int i = 0; i < nLineCount; i++)
	{
		if (1 == nLineCount)
			nTop = 10;
		else
			nTop = i*18;
		strXml.Format(L"<text pos=\"0,%d\">%s</text>", nTop, arrMsgLine[i]);
		strXmlPanel += strXml;
		rcText = GetTextRect(arrMsgLine[i]);
		if (nWidth < rcText.Width())
			nWidth = rcText.Width();
	}
	CRect rc(0, 0, nWidth+10, nHeight);

	return ShowPanelMsg(strXmlPanel, &rc, NULL, uType, pbNoNotifyLater, hWndParent);
}

int CBkSafeMsgBox2::ParseMsgLine(CString strMsg, CAtlArray<CString>& arrMsg)
{
	int nRet = -1;
	if (TRUE == strMsg.IsEmpty())
		return nRet;
	arrMsg.RemoveAll();

	strMsg.Replace(_T("\\n"), _T("\n"));

	int nIndex = -1;
	strMsg += L"\n";
	CString strTmp(L""), msg = strMsg;;
	nIndex = msg.Find(L"\n");

	while (-1 != nIndex)
	{
		strTmp = msg.Left(nIndex);
		if (strTmp.GetLength())
			arrMsg.Add(strTmp);
		msg = msg.Mid(nIndex+1);

		nIndex = msg.Find(L"\n");
	} 

	return arrMsg.GetCount();
}

CRect CBkSafeMsgBox2::GetTextRect(CString strTxt)
{
	//CRect rc(0,0,0,0);
	CWindowDC dc(::GetDesktopWindow());
	CRect rcText(0, 0, 1000, 1000);

	const BkStyle& textStyle = BkStyle::GetStyle("msgtext");

	dc.SelectFont(textStyle.m_ftText);

	dc.DrawText(strTxt, strTxt.GetLength(), rcText, textStyle.m_nTextAlign | DT_CALCRECT);

	return rcText;
}
