#ifndef _KUPDATEVERSIONDLG_H_
#define _KUPDATEVERSIONDLG_H_


#include "bkupctl/bkupddownloader.h"  /* For IBkDownloadCallback interface */
#include <vector>
#include "common/bksafelog.h"


class CManualUpdater : public IBkDownloadCallback
{
public:
	CManualUpdater();
	~CManualUpdater();

	enum { CHECKVER, DOWNLAOD };

	void Attach(HWND hWnd);
	void Start(UINT option);
	void Stop();
	void Execute();

	int GetNumberOfDetail();
	const CString& GetDetailItem(int index);
	const CString& GetVersion();
	
	/* For IBkDownloadCallback interface method */
	virtual BOOL DownloadProcess(DWORD dwTotalSize, DWORD dwReadSize);

protected:
	HRESULT CheckNewVersion(BOOL *pfFoundNew);
	HRESULT ParseCfg(LPCTSTR lpCfgName);
	DWORD CalcFileCRC(LPCTSTR lpFileName);
	BOOL VerifyFileSign(LPCTSTR lpFileName);
	HRESULT DownloadInstallPack();
	static unsigned __stdcall WorkThread(void* param);
	unsigned DoWork();

private:
	CString m_strNewVersion;
	CString m_strUrl;
	CString m_strExeName;
	CString m_strCrc;
	CString m_strCmd;
	CString m_strParamete;
	
	HWND    m_hWnd;
	UINT    m_uOption;
	HANDLE  m_hStopEvent;
	HANDLE  m_hThread;
	BOOL    m_fCompress;
	DWORD   m_dwFileSize;
	BKSafeLog::CBKSafeLog		m_BKSafeLog;

	std::vector<CString> m_details;
};


class CUpdateVersionDlg : public CBkDialogImpl<CUpdateVersionDlg>, 
					      public CWHRoundRectFrameHelper<CUpdateVersionDlg>
{
public:
	CUpdateVersionDlg();
	~CUpdateVersionDlg();

	BEGIN_MSG_MAP_EX(CUpdateVersionDlg)
		MSG_WM_INITDIALOG(OnInitDialog)

		CHAIN_MSG_MAP(CBkDialogImpl<CUpdateVersionDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CUpdateVersionDlg>)

		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)

		MESSAGE_HANDLER_EX(WM_UPDATE_PROGRESS,         OnProgress)
		MESSAGE_HANDLER_EX(WM_UPDATE_CHECK_VERSION,    OnCheckVersion)
		MESSAGE_HANDLER_EX(WM_UPDATE_DOWNLOAD_FINISH,  OnDownlaodFinish)

	END_MSG_MAP()

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE,        OnBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_MIN,          OnBtnMini)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_UPDATE_OK,        OnBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_UPDATE_START,     OnBtnStart)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_UPDATE_CANCEL,    OnBtnCancel)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_UPDATE_OFFLINE,   OnBtnLink)

	BK_NOTIFY_MAP_END()


	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	LRESULT OnProgress(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCheckVersion(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDownlaodFinish(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnBtnClose();
	void OnBtnCancel();
	void OnBtnStart();
	void OnBtnMini();
	void OnBtnLink();

protected:
	void ShowUpdateDetail();
	void ShowOfflineInstallHint(BOOL fShow = TRUE);
	void ShowWarnning(LPCTSTR lpMsg);

private:
	CManualUpdater  m_updater;
	CRichEditCtrl   m_edtDetail;
};





#endif