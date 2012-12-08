// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "resource.h"
#include "AboutDlg.h"
#include <libDownload/libDownload.h>

#include <vector>

#define UWM_DOWNLOADCALLBACK	WM_USER+0x1003


#include <map>
typedef std::map<CString, CString> CStringMap;
class CFileMd5List
{
public:
	BOOL Load(LPCTSTR szFilename);

	BOOL IsFileMd5Matched(LPCTSTR szFilePath, BOOL &bMatched);

protected:
	std::map<CString, CString> m_md5s; 
};


class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
		, public IHttpAsyncObserver
{
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()
	
	BEGIN_MSG_MAP_EX(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_BUTTON_START, BN_CLICKED, OnBnClickedButtonStart)
		COMMAND_HANDLER(IDC_BUTTON_STOP, BN_CLICKED, OnBnClickedButtonStop)
		MESSAGE_HANDLER(UWM_DOWNLOADCALLBACK, OnDownloadEvent)
		COMMAND_HANDLER(IDC_BUTTON_IMPORT, BN_CLICKED, OnBnClickedButtonImport)
		COMMAND_HANDLER(IDC_BUTTON_CLEAR, BN_CLICKED, OnBnClickedButtonClear)
		COMMAND_HANDLER(IDC_BUTTON_JUMPDOWNLOAD, BN_CLICKED, OnBnClickedButtonJumpdownload)
		COMMAND_HANDLER(IDC_BUTTON_ADDURL, BN_CLICKED, OnBnClickedButtonAddurl)
		COMMAND_HANDLER(IDC_BUTTON_DELURL, BN_CLICKED, OnBnClickedButtonDelurl)
		COMMAND_HANDLER(IDC_BUTTON_SEL_DOWNPATH, BN_CLICKED, OnBnClickedButtonSelDownpath)
		COMMAND_HANDLER(IDC_CHECK_TESTDOWNMODE, BN_CLICKED, OnBnClickedCheckTestdownmode)
		MSG_WM_TIMER(OnTimer)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}
	
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		//CloseDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		//CloseDialog(wID);
		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CloseDialog(IDCANCEL);
		return 0;
	}

	void CloseDialog(int nVal);
	LRESULT OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDownloadEvent(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);


public:
	virtual BOOL OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam ) ;
	
protected:
	void ImportUrlList( LPCTSTR szFilename );
	void AddUrl( LPCTSTR szUrl );
	void _EnableFuncButtons( BOOL enableStart );
	BOOL _DownloadNext(INT nCurrent);
	void _OnDownloadDone();
	
protected:
	typedef IDownload* IDownloadPtr;
	typedef std::vector<IDownloadPtr> IDownloadPtrs;
	IDownloadPtrs m_arrDownloads, m_arrDownloadsRunning, m_arrDownloadsQueuing;

protected:
	CListViewCtrl m_lstUrls;	
	CComboBox m_ctlCocurrent, m_ctlCoTask;
	CButton m_btnSelPath;
	int m_nCorrurent, m_nCoTask;
	CString m_strPath;

	CString m_strIniFile;
	CString m_strDatFile;
	
	CButton m_btnCheckTestMode;


	CString m_strMd5File;
	CFileMd5List m_filemd5s;
	INT m_nRestartTickCount;

public:
	LRESULT OnBnClickedButtonJumpdownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonAddurl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonDelurl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonSelDownpath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCheckTestdownmode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnTimer(UINT_PTR nIDEvent);
};
