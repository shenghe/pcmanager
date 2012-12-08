#include "stdafx.h"
#include "kuimsgbox.h"
#include "kuires.h"

UINT_PTR CKuiMsgBox::Show(
    LPCWSTR lpszText, 
    LPCWSTR lpszCaption     /*= NULL*/, 
    UINT uType              /*= MB_OK*/, 
    BOOL *pbNoNotifyLater   /*= NULL*/, 
    HWND hWndParent         /*= ::GetActiveWindow()*/)
{
    CKuiMsgBox msgbox;

    return msgbox.ShowMsg(lpszText, lpszCaption, uType, pbNoNotifyLater, hWndParent);
}

void CKuiMsgBox::AddButton(LPCWSTR lpszText, UINT uCmdID, UINT uWidth /*= 0*/)
{
    if (0 == uWidth)
    {
        CString strText = lpszText;
        CWindowDC dc(::GetDesktopWindow());
        CRect rcText(0, 0, 1000, 1000);

        const KuiStyle& btnStyle = KuiStyle::GetStyle("normalbtn");

        HFONT hftOld = dc.SelectFont(btnStyle.m_ftText);

        dc.DrawText(strText, strText.GetLength(), rcText, btnStyle.m_nTextAlign | DT_CALCRECT);

        rcText.InflateRect(btnStyle.m_nMarginX, btnStyle.m_nMarginY);

        dc.SelectFont(hftOld);

        uWidth = max(80, rcText.Width());
    }

    m_lstButton.AddTail(__BKMBButton(lpszText, uCmdID, uWidth));
}


void CKuiMsgBox::AddButtonEx(LPCWSTR lpszText, UINT uCmdID, LPCWSTR lpszPos)
{
    if (lpszText && lpszPos)
        m_lstButtonEx.AddTail(__BKMBButtonEx(lpszText, uCmdID, lpszPos));
}

void CKuiMsgBox::SetNoNotifyText(LPCWSTR lpszText)
{
    m_strNoNotifyText = lpszText;
}

UINT_PTR CKuiMsgBox::ShowMsg(
    LPCWSTR lpszText, 
    LPCWSTR lpszCaption     /*= NULL*/, 
    UINT uType              /*= MB_OK*/, 
    BOOL *pbNoNotifyLater   /*= NULL*/, 
    HWND hWndParent         /*= ::GetActiveWindow()*/)
{
    if (!Load("IDR_DLG_MESSAGEBOX"))
        return -1;

    if (m_strNoNotifyText.GetLength())
        SetItemText(60007, m_strNoNotifyText);

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
}

UINT_PTR CKuiMsgBox::ShowPanelMsg( LPCWSTR lpszXml, LPCRECT lpRect
									 , LPCWSTR lpszCaption /*= NULL*/, UINT uType /*= MB_OK*/, BOOL *pbNoNotifyLater /*= NULL*/, HWND hWndParent /*= NULL */ )
{
	ATLASSERT(lpRect);
	if (!lpRect || !Load("IDR_DLG_MESSAGEBOX"))
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

void CKuiMsgBox::_InitLayout( BOOL * pbNoNotifyLater )
{
	CString strButtonXmlFormat, strButtons, strButtonDiv;
    CString strButtonEx = "<dlg pos=\"0,0,-0,-0\">";
    KAppRes& appRes = KAppRes::Instance();
    BOOL fUseButtonEx = FALSE;

	_MakeButtons();

	strButtonXmlFormat = appRes.GetString("IDS_MSGBOX_BUTTON_XML");

    POSITION pos = m_lstButtonEx.GetHeadPosition();
    while (pos)
    {
        const __BKMBButtonEx &btn = m_lstButtonEx.GetNext(pos);
        CString strBtn;
        strBtn.Format(
            _T("<button id=%d pos=\"%s\" class=normalbtn>%s</button>"),
            btn.uCmdID,
            btn.strPos,
            btn.strText
            );
        strButtonEx += strBtn;
        fUseButtonEx = TRUE;
    }

#   define BUTTON_SPACING 20
	UINT uWidth = 0, uLeft = 0;
	pos = m_lstButton.GetHeadPosition();


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

    if (!fUseButtonEx)
    {
        strButtonDiv.Format(appRes.GetString("IDS_MSGBOX_BUTTON_DIV"), uWidth / 2, uWidth / 2, strButtons);
        SetPanelXml(IDC_DIV_MSGBOX_BUTTONS, CT2A(strButtonDiv, CP_UTF8));
        SetItemVisible(IDC_DIV_MSGBOX_BUTTONS+1, FALSE);
    }
    else
    {
        strButtonEx += "</dlg>";
        SetPanelXml(IDC_DIV_MSGBOX_BUTTONS+1, CT2A(strButtonEx, CP_UTF8));
        SetItemVisible(IDC_DIV_MSGBOX_BUTTONS, FALSE);
    }

	CString strIconID;

	if (0 != (MB_ICONMASK & m_uType))
	{
		switch (MB_ICONMASK & m_uType)
		{
		case MB_ICONHAND:
			strIconID = _T("IDI_HAND");
			break;
		case MB_ICONQUESTION:
			strIconID = _T("IDI_QUESTION");
			break;
		case MB_ICONEXCLAMATION:
			strIconID = _T("IDI_EXCLAMATION");
			break;
		case MB_ICONASTERISK:
			strIconID = _T("IDI_ASTERISK");
			break;
		default:
            SetItemStringAttribute(IDC_ICON_MSGBOX_ICON, "src", strIconID);
            SetItemDWordAttribute(IDC_ICON_MSGBOX_ICON, "oem", 0);
		}

		if (strIconID.GetLength())
			SetItemStringAttribute(IDC_ICON_MSGBOX_ICON, "src", strIconID);
	}

	if (0 == strIconID.GetLength())
		SetItemVisible(IDC_ICON_MSGBOX_ICON, FALSE);

	if (NULL == pbNoNotifyLater)
		SetItemVisible(IDC_CHK_MSGBOX_NONOTIFYLATER, FALSE);
	else
		m_bShowNoNotifyLater = TRUE;
}

void CKuiMsgBox::_MakeButtons()
{
    KAppRes& appRes = KAppRes::Instance();

    if ((MB_BK_CUSTOM_BUTTON & m_uType) == MB_BK_CUSTOM_BUTTON)
        return;

    m_lstButton.RemoveAll();

    switch (MB_TYPEMASK & m_uType)
    {
    case MB_OK:
        AddButton(appRes.GetString("IDS_MSGBOX_OK"), IDOK);
        break;
    case MB_OKCANCEL:
        AddButton(appRes.GetString("IDS_MSGBOX_OK"), IDOK);
        AddButton(appRes.GetString("IDS_MSGBOX_CANCEL"), IDCANCEL);
        break;
    case MB_ABORTRETRYIGNORE:
        AddButton(appRes.GetString("IDS_MSGBOX_ABORT"), IDABORT);
        AddButton(appRes.GetString("IDS_MSGBOX_RETRY"), IDRETRY);
        AddButton(appRes.GetString("IDS_MSGBOX_IGNORE"), IDIGNORE);
        break;
    case MB_YESNOCANCEL:
        AddButton(appRes.GetString("IDS_MSGBOX_YES"), IDYES);
        AddButton(appRes.GetString("IDS_MSGBOX_NO"), IDNO);
        AddButton(appRes.GetString("IDS_MSGBOX_CANCEL"), IDCANCEL);
        break;
    case MB_YESNO:
        AddButton(appRes.GetString("IDS_MSGBOX_YES"), IDYES);
        AddButton(appRes.GetString("IDS_MSGBOX_NO"), IDNO);
        break;
    case MB_RETRYCANCEL:
        AddButton(appRes.GetString("IDS_MSGBOX_RETRY"), IDRETRY);
        AddButton(appRes.GetString("IDS_MSGBOX_CANCEL"), IDCANCEL);
        break;
#   if(WINVER >= 0x0500)
    case MB_CANCELTRYCONTINUE:
        AddButton(appRes.GetString("IDS_MSGBOX_CANCEL"), IDCANCEL);
        AddButton(appRes.GetString("IDS_MSGBOX_RETRY"), IDRETRY);
        AddButton(appRes.GetString("IDS_MSGBOX_CONTINUE"), IDCONTINUE);
        break;
#   endif /* WINVER >= 0x0500 */
    }
}

void CKuiMsgBox::OnBkCommand(UINT uItemID, LPCSTR szItemClass)
{
    if (strcmp(CKuiButton::GetClassName(), szItemClass) != 0 && strcmp(CKuiImageBtnWnd::GetClassName(), szItemClass) != 0)
    {
        if (uItemID == IDC_CHK_MSGBOX_NONOTIFYLATER)
            m_bNoNotifyLater = GetItemCheck(IDC_CHK_MSGBOX_NONOTIFYLATER);
        return;
    }

    EndDialog(uItemID);
}

LRESULT CKuiMsgBox::OnInitDialog(HWND hWnd, LPARAM lParam)
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


	const KuiStyle& titleStyle = KuiStyle::GetStyle("dlgtitle");
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

		const KuiStyle& textStyle = KuiStyle::GetStyle("msgtext");

		dc.SelectFont(textStyle.m_ftText);

		dc.DrawText(m_strText, m_strText.GetLength(), rcText, textStyle.m_nTextAlign | DT_CALCRECT);
	}
	rcText.right += 100;
	rcText.bottom += 120;

    if (m_bShowNoNotifyLater)
        rcText.bottom += 20;
	
    dc.SelectFont(hftOld);

	rcClient.right = rcClient.left + max(max(rcCaption.right, rcText.right), 320);
    rcClient.bottom = rcClient.top + max(rcText.bottom, 160);

    MoveWindow(rcClient, FALSE);

    return TRUE;
}
