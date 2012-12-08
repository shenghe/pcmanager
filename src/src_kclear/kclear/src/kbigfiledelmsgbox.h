#ifndef _KBIGFILEDELMSGBOX_H_
#define _KBIGFILEDELMSGBOX_H_

class CKBigFileMsgBox
	:public CKuiDialogImpl<CKBigFileMsgBox>
	,public CWHRoundRectFrameHelper<CKBigFileMsgBox>
{
public:
	CKBigFileMsgBox(const CString strFileName)
		:CKuiDialogImpl<CKBigFileMsgBox>("IDR_DLG_DEL_MSGBOX")
		, m_bType(FALSE)
		,m_strFileName(strFileName)
	{
	}
	~CKBigFileMsgBox(){}
	void OnDelFile()
	{
		EndDialog(1);
	}

	BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
    {
		SetItemCheck(3, TRUE);
		std::string strTemp;
		strTemp = UnicodeToUtf8(m_strFileName.GetBuffer());
		if (strTemp.size() > 24)
		{
			CString strTempLeft;
			CString strTempRight;
			strTempLeft = m_strFileName.Left(6);
			strTempRight = m_strFileName.Right(4);
			m_strFileName = strTempLeft;
			m_strFileName += L"...";
			m_strFileName += strTempRight;
		}
		CString str;
		str = L"您确认删除“";
		str += m_strFileName;
		str +=L"”这个文件吗？";
		SetItemText(5, str);
		return TRUE;
	}
	void OnCancel()
	{
		EndDialog(0);
	}

	void OnDelToRecy()
	{
		m_bType = FALSE;
	}

	void OnDelDirect()
	{
		m_bType = TRUE;
	}

	BOOL IsDelFileDirect()
	{
		return m_bType;
	}

	KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		KUI_NOTIFY_ID_COMMAND(2, OnCancel)
		KUI_NOTIFY_ID_COMMAND(1, OnDelFile)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnCancel)
		KUI_NOTIFY_ID_COMMAND(3, OnDelToRecy)
		KUI_NOTIFY_ID_COMMAND(4, OnDelDirect)
	KUI_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CKBigFileMsgBox)
		MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CKuiDialogImpl<CKBigFileMsgBox>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CKBigFileMsgBox>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
protected:
	BOOL m_bType;
	CString m_strFileName;
};
#endif