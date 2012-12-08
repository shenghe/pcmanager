#pragma once

#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include <comproxy/bkscanfile.h>
#include "uplive/updproxy.h"


#define UP_LIB_URL_PATH			TEXT("http://up.ijinshan.com/safe/instup.pack")
#define UP_LIB_INI_PATH			TEXT("instup.ini")


#define MSG_UPDATE_SCRIPT_PROCESS						(WM_APP + 21)
#define MSG_UPDATE_PRE_SCAN_FINISH						(WM_APP + 22)
#define MSG_ON_UPDATING									(WM_APP + 23)
#define MSG_NET_FAILED									(WM_APP + 24)
#define TICKCOUNT_REFRESH	(50)
#define TIME_DELAY_ALL		(10*1000)

class CSetupDlg
	: public CBkDialogImpl<CSetupDlg>
	, public CWHRoundRectFrameHelper<CSetupDlg>
	, public IUpdatePolicyCallback
	, public IStdUpdateCallback

{
public:
	CSetupDlg()
		: CBkDialogImpl<CSetupDlg>(IDR_BK_SETUP_DLG)
	{
		m_bCancel	= FALSE;
		m_nCount	= 0;
		m_bInitOk	= FALSE;
		m_bSendPreScanFinish = FALSE;
		m_bFinish	= FALSE;
		m_bMsg		= FALSE;
		m_bNoUpdate	= FALSE;
		m_bUpFileFailed = FALSE;
	}
	~CSetupDlg(void)
	{

	}

	void MessageConfigUpdate( LPCWSTR lpwszConfigPath, HRESULT hr ){};
	void VirusLibUpdated( ULONGLONG ullVersion, BOOL bSuccessful = FALSE ){};
	void NotifyLibUpdate( LPCWSTR lpwszFileName ){};

	void STDMETHODCALLTYPE UpdateType( DWORD dwUpdType ){};
	BOOL STDMETHODCALLTYPE OnProgress( DWORD   dwStatus, DWORD   dwPercentage)
	{
		::PostMessage( m_hWnd, MSG_UPDATE_SCRIPT_PROCESS, (WPARAM)dwPercentage, NULL );
		if( enumUpdStatusDownloadFiles <= dwStatus )
		{
			if( m_bSendPreScanFinish == FALSE )
			{
				::PostMessage( m_hWnd, MSG_UPDATE_PRE_SCAN_FINISH, NULL, NULL );
				m_bSendPreScanFinish = TRUE;
			}
		}

		if ( enumUpdStatusFinished == dwStatus )
		{
			::PostMessage(m_hWnd,MSG_SCAN_END,NULL,NULL);
		}

		if( enumUpdNothing == dwStatus )
		{
			::PostMessage(m_hWnd, MSG_ON_UPDATING, NULL, NULL );
		}

		return TRUE;
	};
	void STDMETHODCALLTYPE OnError(	DWORD   dwStatus, HRESULT hErrCode, LPCWSTR lpInformation )
	{
		if( lpInformation )
		{
			if( _tcslen( lpInformation ) > 0 )
			{
				::PostMessage( m_hWnd, MSG_NET_FAILED, NULL, NULL );
			}
		}
	};

	void STDMETHODCALLTYPE OnFilePreUpdated(
		LPCWSTR lpszModuleName, 
		LPCWSTR lpszFileName,
		int     nType,
		LPCWSTR lpszVersion = NULL,
		LPCWSTR lpszExtend = NULL
		){};

	BOOL STDMETHODCALLTYPE OnBeginUpdateFiles( DWORD dwUpdType )
	{
		return TRUE;
	};
	void STDMETHODCALLTYPE OnFileReplace(
		LPCWSTR lpszModuleName, 
		LPCWSTR lpszFileName,
		int     nType,
		int     nOperation,
		DWORD   dwErrorCode
		){};

	void STDMETHODCALLTYPE OnFileUpdated(
		LPCWSTR lpszModuleName, 
		LPCWSTR lpszFileName,
		int     nType
		){};

	void STDMETHODCALLTYPE OnPreDownloadUpdated(
		LPCWSTR lpszDownloadUrl,
		LPCWSTR lpszFileName,
		LPCWSTR lpszTargetFileName,
		LPCWSTR lpszTargetPath,
		LPCWSTR lpszCRC,
		LPCWSTR lpszHint,
		LPCWSTR lpszVer,
		DWORD   dwSize,
		BOOL    b3rd,
		int     nType
		){};

public:
	DWORD dwTime;
	void ExitScan()
	{
		if( m_bInitOk )
		{
			m_UpliveProxy.Stop();
			m_UpliveProxy.WaitExit();
			m_UpliveProxy.Uninitialize();
		}
	}

	void OnBkClose()
	{
		if( m_bFinish )
		{
			EndDialog(IDOK );
			return;
		}
		else
		{
			CBkSafeMsgBox2 msg;
			msg.AddButton( BkString::Get(IDS_VIRSCAN_2724), IDCANCEL );
			msg.AddButton( BkString::Get(IDS_VIRSCAN_2725), IDOK );
			m_bMsg = TRUE;
			UINT uId = msg.ShowMutlLineMsg( BkString::Get(IDS_VIRSCAN_2723), NULL, MB_BK_CUSTOM_BUTTON|MB_ICONINFORMATION);
			m_bMsg = FALSE;

			if( uId == IDOK )
			{
				ExitScan();
				m_bInitOk = FALSE;
				m_bCancel=TRUE;
				EndDialog( IDCANCEL );
			}
			else if( m_bFinish == TRUE )
			{
				SetTimer( 2, 2000, NULL );
			}
		}
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		ShowDiv( IDC_LBL_PRE_DL );
		_beginthread(_UpdateScriptLib,0,this);
		return TRUE;
	}

	void OnTimer(DWORD_PTR nIdEvent)
	{
		if(nIdEvent == 2)
		{
			KillTimer( 2 );
			EndDialog(IDOK);
		}
	}

	int DoUpdateScriptLib()
	{
		HRESULT hr = E_FAIL;
		do 
		{  
			hr = m_UpliveProxy.Initialize();
			if ( FAILED(hr) )
			{
				ATLASSERT(SUCCEEDED(hr));
				break;
			};

			m_UpliveProxy.SetCallback( (IStdUpdateCallback*)this, (IUpdatePolicyCallback*)this );
			hr = m_UpliveProxy.ForceUpdateSpecial( UP_LIB_URL_PATH, UP_LIB_INI_PATH, 0);
			hr = S_OK;

		} while (false);

		if ( FAILED(hr) )
		{
			m_UpliveProxy.Uninitialize();
		}
		else
		{
			m_bInitOk = TRUE;
		}	
		return 0;
	}

	static void _UpdateScriptLib(LPVOID pvParam)
	{		
		CSetupDlg* pThis = static_cast<CSetupDlg*>(pvParam);
		if(NULL == pThis)
			return;

		pThis->DoUpdateScriptLib();

		return;
	}

	LRESULT OnDestory( UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		if( m_bInitOk )
		{
			m_UpliveProxy.Stop();
			m_UpliveProxy.WaitExit();
			m_UpliveProxy.Uninitialize();
		}

		return TRUE;
	}

	LRESULT	OnScanFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if( m_bNoUpdate == FALSE && m_bUpFileFailed == FALSE )
		{
			SetItemAttribute(IDC_PROGRESS_SETUP,"value","100");
			ShowDiv( IDC_LBL_DL_FINISH );

			m_bFinish = TRUE;

			SetItemText( IDC_BTN_SETUP_CANCEL, BkString::Get(IDS_MSGBOX_OK) );

			if( m_bMsg == FALSE )
				SetTimer(2,2000);
		}
		
		return S_OK;
	}
	
	LRESULT OnUpdating( UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		SetItemAttribute(IDC_PROGRESS_SETUP,"value","100");
		ShowDiv( IDC_LBL_NEW_LIB );
		SetItemText( IDC_BTN_SETUP_CANCEL, BkString::Get(IDS_MSGBOX_OK) );
		SetTimer(2,2000);
		m_bNoUpdate = TRUE;
		m_bFinish = TRUE;
		return TRUE;
	}

	LRESULT OnNetFailed( UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		ShowDiv( IDC_LBL_NET_FAILED );
		m_bFinish = TRUE;
		m_bUpFileFailed = TRUE;

		SetItemText( IDC_BTN_SETUP_CANCEL, BkString::Get(IDS_MSGBOX_OK) );
		SetTimer(2,3000);
		return TRUE;
	}

	LRESULT OnPreScanFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ShowDiv( IDC_LBL_DOWNLOADING );
		return TRUE;
	}

	LRESULT OnUpdateScriptProcess ( UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		CString		str;
		str.Format(_T("%d"), (DWORD)wParam );
		SetItemStringAttribute(IDC_PROGRESS_SETUP,"value",str);
		str += L"\t";
		OutputDebugString( str );
		return TRUE;
	}

	void ShowDiv( UINT uId )
	{
		SetItemVisible( IDC_LBL_PRE_DL, uId == IDC_LBL_PRE_DL );
		SetItemVisible( IDC_LBL_DOWNLOADING, uId == IDC_LBL_DOWNLOADING );
		SetItemVisible( IDC_LBL_NEW_LIB, uId == IDC_LBL_NEW_LIB );
		SetItemVisible( IDC_LBL_DL_FINISH, uId == IDC_LBL_DL_FINISH );
		SetItemVisible( IDC_LBL_NET_FAILED, uId == IDC_LBL_NET_FAILED );
	}

protected:
	BOOL				m_bCancel;
	DWORD				m_nCount;
	CUpliveProxy2		m_UpliveProxy;
	BOOL				m_bInitOk; 
	BOOL				m_bSendPreScanFinish;
	BOOL				m_bFinish;
	BOOL				m_bMsg;
	BOOL				m_bNoUpdate;
	BOOL				m_bUpFileFailed;

	void OnSize(UINT nType, CSize size)
	{
		HRGN	hRgn = ::CreateRectRgn(0,0,size.cx,size.cy);
		::SetWindowRgn(m_hWnd,hRgn,FALSE);
		SetMsgHandled(FALSE);
	}

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SETUP_CANCEL, OnBkClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SETUP_CLOSE, OnBkClose)
	BK_NOTIFY_MAP_END()


	BEGIN_MSG_MAP_EX(CSetupDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CSetupDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CSetupDlg>)
 		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER_EX(MSG_UPDATE_SCRIPT_PROCESS,	OnUpdateScriptProcess )
		MESSAGE_HANDLER_EX(MSG_SCAN_END,				OnScanFinish)
		MESSAGE_HANDLER_EX(WM_DESTROY,					OnDestory)
		MESSAGE_HANDLER_EX(MSG_UPDATE_PRE_SCAN_FINISH,	OnPreScanFinish)
		MESSAGE_HANDLER_EX(MSG_ON_UPDATING,				OnUpdating)
		MESSAGE_HANDLER_EX(MSG_NET_FAILED,				OnNetFailed)

//		MSG_WM_SIZE(OnSize)
	END_MSG_MAP()
};
