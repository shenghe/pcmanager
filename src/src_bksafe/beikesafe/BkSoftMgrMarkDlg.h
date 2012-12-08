#pragma  once

class CBeikeSafeSoftmgrUIHandler;		// ¸¸´°¿Ú

class CBkMarkDlg
	: public CBkDialogImpl<CBkMarkDlg>
	, public CWHRoundRectFrameHelper<CBkMarkDlg>
{
public:
	CBkMarkDlg(CBeikeSafeSoftmgrUIHandler * pHandler, const CString &softId, const CString &softName)
		: CBkDialogImpl<CBkMarkDlg>(IDR_BK_MARK_DLG)
		, m_pHandler( pHandler )
		, m_softId(softId)
		, m_softName(softName)
	{
	}

	~CBkMarkDlg()
	{
	}

public:
	UINT_PTR DoModal(HWND hWndParent);

protected:
	CBeikeSafeSoftmgrUIHandler * m_pHandler;

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnBkBtnClose();

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBkMarkDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBkMarkDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkMarkDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

public:
	const CString &m_softId;
	const CString &m_softName;
	CWHHtmlContainerWindow			m_IEMark;
};

