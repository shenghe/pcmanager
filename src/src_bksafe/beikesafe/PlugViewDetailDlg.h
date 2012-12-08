#pragma once

#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include "beikesafemsgbox.h"

class CPlugViewDetailDlg
	: public CBkDialogImpl<CPlugViewDetailDlg>
	, public CWHRoundRectFrameHelper<CPlugViewDetailDlg>
{
public:
	CPlugViewDetailDlg(LPCTSTR strName, CSimpleArray<CString>* pList)
		: CBkDialogImpl<CPlugViewDetailDlg>(IDR_BK_PLUG_DETAIL_DLG)
	{
		m_nFlag = 1;
		m_strBuffer = TEXT("");
		m_listDetail = pList;
		m_strName	 = strName;
	}
	//构造函数重载  这个函数 是木马扫描之用的
	CPlugViewDetailDlg(LPCTSTR strName, CString strUrl, CString strFile, CString strPos, CString strLnk)
		: CBkDialogImpl<CPlugViewDetailDlg>(IDR_BK_PLUG_DETAIL_DLG)
	{
		//构造
		m_nFlag = 2;
		m_strBuffer = TEXT("");
		m_strName	 = strName;
		if (strUrl.GetLength() > 0)
		{
			m_strBuffer += _T("网址：");
			m_strBuffer += _T("\r\n");
			m_strBuffer += strUrl;
		}
		if (strFile.GetLength() > 0)
		{
			m_strBuffer += _T("文件：");
			m_strBuffer += _T("\r\n");
			m_strBuffer += strFile;
		}
		if (strPos.GetLength() > 0)
		{
			m_strBuffer += _T("注册表信息：");
			m_strBuffer += _T("\r\n");
			m_strBuffer += strPos;
		}

		if( strLnk.GetLength() > 0)
		{
			m_strBuffer += _T("快捷方式位置：");
			m_strBuffer += _T("\r\n");
			m_strBuffer += strLnk;
		}
	}

	CPlugViewDetailDlg(
		LPCTSTR strDlgCpt, 
		const CString& strItemName,
		const CString& strDescript,
		const CString& strUrl, 
		const CString& strFile, 
		const CString& strPos,
		const CString& strLnk )
		: CBkDialogImpl<CPlugViewDetailDlg>(IDR_BK_PLUG_DETAIL_DLG)
	{
		m_nFlag = 3;
		m_strBuffer = TEXT("");
		m_strName	 = strItemName;
		if (!strDescript.IsEmpty())
		{
			m_strBuffer += strDescript;
			m_strBuffer += _T("\r\n\r\n");	
		}
		if (!strUrl.IsEmpty())
		{
			m_strBuffer += _T("网址：");
			m_strBuffer += _T("\r\n");
			m_strBuffer += strUrl;
		}
		if (!strFile.IsEmpty())
		{
			m_strBuffer += _T("文件：");
			m_strBuffer += _T("\r\n");
			m_strBuffer += strFile;
		}
		if (!strPos.IsEmpty())
		{
			m_strBuffer += _T("注册表信息：");
			m_strBuffer += _T("\r\n");
			m_strBuffer += strPos;
		}

		if( strLnk.GetLength() > 0)
		{
			m_strBuffer += _T("快捷方式位置：");
			m_strBuffer += _T("\r\n");
			m_strBuffer += strLnk;
		}
	}
	~CPlugViewDetailDlg(void)
	{

	}
protected:
	CRichEditCtrl			m_editView;
	CSimpleArray<CString>*	m_listDetail;
	CFont					m_fntText;
	CString					m_strBuffer;
	CString					m_strName;
	int						m_nFlag;

public:

	void CopyDataToClipboard(LPCTSTR lpData )
	{
		HGLOBAL hglbCopy; 
		LPTSTR  lptstrCopy; 

		if( OpenClipboard() )
		{
			EmptyClipboard();

			hglbCopy = GlobalAlloc( GMEM_MOVEABLE, (_tcslen(lpData)+1)*sizeof(TCHAR)); 
			if( hglbCopy ) 
			{ 
				lptstrCopy = (LPTSTR) GlobalLock(hglbCopy); 
				_tcscpy( lptstrCopy, lpData );
				GlobalUnlock(hglbCopy); 
#ifdef _UNICODE
				SetClipboardData(CF_UNICODETEXT, hglbCopy); 
#else
				SetClipboardData(CF_TEXT, hglbCopy); 	
#endif
			} 
			CloseClipboard(); 
		}
	}

	void OnBkBtnClose()
	{
		EndDialog(0);
	}
	void OnBkBtnCopy()
	{
		CopyDataToClipboard(m_strBuffer);

        CString strText;
        strText.Format(BkString::Get(IDS_PLUGIN_2048));
		CBkSafeMsgBox::Show( strText, NULL, MB_ICONINFORMATION|MB_OK);
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		m_editView.Create( GetViewHWND(), NULL, NULL, 
			WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|ES_AUTOHSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_TABSTOP|ES_READONLY, 
			0, IDC_PLUG_DETAIL_DLG_EDIT);

		if (m_nFlag == 1)
		{
			for ( INT nIndex = 0; nIndex < m_listDetail->GetSize(); nIndex++)
			{
				m_strBuffer += (*m_listDetail)[nIndex];
				m_strBuffer += _T("\r\n");
			}
		}
		
		m_editView.SetWindowText(m_strBuffer);

		m_fntText.CreateFont(13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY ,DEFAULT_PITCH, _T("宋体"));
		m_editView.SetFont(m_fntText);

		CString		strX;

		if (m_nFlag==3)
		{
			strX = m_strName;
			SetItemStringAttribute(IDC_TXT_PLUG_DETAIL_ICON, "sub", L"1");
		}
		else
			strX.Format(BkString::Get(IDS_PLUGIN_2049),m_strName);

		SetItemText(IDC_TXT_PLUG_DETAIL_NAME,strX);
		SetItemStringAttribute(IDC_TXT_PLUG_DETAIL_NAME,"tip",strX);

		return TRUE;
	}

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_PLUG_DETAIL_DLG_EXIT, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_PLUG_DATAIL_DLG_COPY, OnBkBtnCopy)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_PLUG_DATAIL_DLG_XCLOSE, OnBkBtnClose)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CPlugViewDetailDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CPlugViewDetailDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CPlugViewDetailDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()
};
