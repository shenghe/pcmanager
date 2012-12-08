#pragma  once

class CBeikeSafeSoftmgrUIHandler;		// ¸¸´°¿Ú

class CBkNewInfoDlg
	: public CBkDialogImpl<CBkNewInfoDlg>
	, public CWHRoundRectFrameHelper<CBkNewInfoDlg>
{
public:
	CBkNewInfoDlg(CBeikeSafeSoftmgrUIHandler * pHandler, const CString &softId, const CString &softName)
		: CBkDialogImpl<CBkNewInfoDlg>(IDR_BK_NEW_INFO_DLG)
		, m_pHandler( pHandler )
		, m_softId(softId)
		, m_softName(softName)
	{
	}

	~CBkNewInfoDlg()
	{
	}

public:
	UINT_PTR DoModal(HWND hWndParent);

protected:
	CBeikeSafeSoftmgrUIHandler * m_pHandler;

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnBkBtnClose();
	void OnBkBtnOK();
	void OnBkBtnCancel();

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDOK, OnBkBtnOK)
		BK_NOTIFY_ID_COMMAND(IDCANCEL, OnBkBtnCancel)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBkNewInfoDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBkNewInfoDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkNewInfoDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

public:
	const CString &m_softId;
	const CString &m_softName;
	CWHHtmlContainerWindow			m_IE;
};

