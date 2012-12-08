#pragma once

#include <wtlhelper/whwindow.h>
#include "common/bwsuserdatadb.h"
#include "beikesafemsgbox.h"			

class CBeikeSafeAddUrlDlg
	: public CBkDialogImpl<CBeikeSafeAddUrlDlg>
	, public CWHRoundRectFrameHelper<CBeikeSafeAddUrlDlg>
{
public:
	CBeikeSafeAddUrlDlg( LPVOID pvParam )
		: CBkDialogImpl<CBeikeSafeAddUrlDlg>(IDR_BK_ADD_URL_DLG)
	{
		m_pBwsUserDataDB = (CBwsUserDataDB *)pvParam;
		m_strIllegalChars = _T("\x20\t\r\n");
	}

protected:

	CString m_strIllegalChars;

	CAtlArray<CString> m_arrayUrl;

	UINT ShowMsg(LPCTSTR pszCaption)
	{
		CBkSafeMsgBox dlg;
		return dlg.ShowMsg(pszCaption, NULL, MB_OK|MB_ICONINFORMATION, NULL, GetViewHWND());
	}

	BOOL UrlFilter( CString& strUrl )
	{
		BOOL bRet = FALSE, bIllegal = FALSE;
		bIllegal = ( -1 == strUrl.FindOneOf( m_strIllegalChars ) ) ? FALSE:TRUE;

		bRet = !bIllegal;
		return bRet;
	}

	void UrlPreoperate( CString& strUrl )
	{
		CString strHeader = _T("http://"), strTmp;
		int nHeaderLength = strHeader.GetLength(), nUrlLength = strUrl.GetLength();
		BOOL bAddHeader = TRUE, bAddTailer = TRUE;
		
		if ( nUrlLength >= nHeaderLength )
		{
			strTmp = strUrl.Left( nHeaderLength );
			if ( 0 == strTmp.CompareNoCase( strHeader ) )
			{
				bAddHeader = FALSE;
			}
		}

		if ( (nUrlLength - 1) == strUrl.ReverseFind( _T('*') ) )
		{
			bAddTailer = FALSE;
		}

		if ( bAddHeader )
		{
			strUrl = strHeader + strUrl;
		}

		if ( bAddTailer )
		{
			strUrl += _T('*');
		}
	}

	void OnBkBtnOk()
	{
		CAtlArray<BWS_USER_DATA_URL> arrayUrls;
		BWS_USER_DATA_URL bwsUserDataUrl;
		CAtlArray<CString> arrayLines;

		CString strRead;

		m_EditInfo.GetWindowText( strRead );
		int nLength = strRead.GetLength();
		int nAddCount = 0, nErrorCount = 0;
		while ( nLength )
		{
			int nPosNextLine = -1;
			CString strUrl;

			nPosNextLine = strRead.Find( _T('\n') );
			if ( -1 == nPosNextLine )
			{// 最后一行或者已经结尾
				strUrl = strRead;
				strRead.Empty();
			}
			else
			{
				strUrl = strRead.Left( nPosNextLine );
				strRead = strRead.Right( nLength - nPosNextLine - 1 );
			}

			strUrl = strUrl.Trim( m_strIllegalChars );

			if ( strUrl.GetLength() )
			{
				arrayLines.Add( strUrl );
			}

			nLength = strRead.GetLength();
		}

		size_t stLineCount = arrayLines.GetCount();
		for ( size_t i = 0 ; i < stLineCount ; i ++ )
		{
			bwsUserDataUrl.m_dwOperation = BK_BWSUSERDATA_OPERATION_BLOCK;
			bwsUserDataUrl.m_strUrl      = arrayLines[i];
			if ( UrlFilter( bwsUserDataUrl.m_strUrl ) )
			{
				UrlPreoperate( bwsUserDataUrl.m_strUrl );
				arrayUrls.Add( bwsUserDataUrl );
			}
			else
			{
				nErrorCount ++;
			}
		}

		if ( nErrorCount && 
			 1 == stLineCount )
		{
			ShowMsg( BkString::Get(IDS_IEFIX_6133) );
		}

		if ( arrayUrls.GetCount() )
		{
			m_pBwsUserDataDB->AddUrlRecordArray(arrayUrls);
		}

		EndDialog(IDOK);
	}

	void OnBkBtnCancel()
	{
		EndDialog(IDCANCEL);
	}

	BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
	{
		m_EditInfo.Create( GetViewHWND(), NULL, NULL, WS_VISIBLE|WS_CHILD|ES_LEFT|ES_MULTILINE|ES_WANTRETURN|WS_VSCROLL|ES_AUTOVSCROLL, 
			0, IDC_DLG_ADD_URL_REALWND, NULL);

		PARAFORMAT2 pf2;
		pf2.cbSize = sizeof(PARAFORMAT2);
		pf2.dwMask = PFM_LINESPACING;
		pf2.dyLineSpacing     = 330;
		pf2.bLineSpacingRule  = 4;

		m_EditInfo.SetParaFormat(pf2);
		m_EditInfo.SetFont(BkFontPool::GetFont(FALSE, FALSE, FALSE, 0));
		m_EditInfo.SetBackgroundColor( RGB(255, 255, 255) );

		CString ShowText;
		m_EditInfo.SetWindowText( _T("") );

		return TRUE;
	}

public:

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE,          OnBkBtnCancel)
		BK_NOTIFY_ID_COMMAND(IDC_DLG_ADD_URL_BTN_OK,     OnBkBtnOk)
		BK_NOTIFY_ID_COMMAND(IDC_DLG_ADD_URL_BTN_CANCEL, OnBkBtnCancel)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeAddUrlDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeAddUrlDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeAddUrlDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	CRichEditCtrl   m_EditInfo;
	CBwsUserDataDB *m_pBwsUserDataDB;
};