
#pragma once

#include <uplive/updproxy.h>
#include <strsafe.h>
#include <RpcDce.h>
#include "communits/VerifyFileFunc.h"
#include "common/bksafelog.h"


#define KCLEAR_INSTALLER_DOWNLOAD_URL			_T("http://dl.ijinshan.com/safe/clear.pack")
#define KCLEAR_INSTALLER_TEMP_FILE_NAME			_T("\\ksafe_setup_clear.exe")

#define KCLEAR_DIR_NAME		_T("KClear")
#define KCLEAR_EXE_NAME		_T("KClear.exe")

#define KCLEAR_STATUS_KEY_NAME       L"SOFTWARE\\KSafe\\KClear"
#define KCLEAR_STATUS_VALUE_NAME     L"Install"
#define KSafe_Tool_NOT_RUN			 L"nosetup"

#define MSG_SYSTOOLS_DOWN_BEGIN		(WM_APP+1)
#define MSG_SYSTOOLS_DOWN_PROC		(WM_APP+2)
#define MSG_SYSTOOLS_DOWN_END		(WM_APP+3)
#define MSG_SYSTOOLS_THREAD_END		(WM_APP+4)

class CToolsDownloadDlg
	: public CBkDialogImpl<CToolsDownloadDlg>
	, public CWHRoundRectFrameHelper<CToolsDownloadDlg>
	, public IUpdateDownloadFileCallback
{
public:
	CToolsDownloadDlg(LPCTSTR lpszDownLoadUrl, LPCTSTR lpszSetupFile, LPCTSTR lpszParam, CString strTitle = L"", BOOL bWait = TRUE )
		: CBkDialogImpl<CToolsDownloadDlg>(IDR_BK_SYSTOOLS_DOWNLOAD)
	{
		m_bCancel = FALSE;
		m_strDownTmpPath = lpszSetupFile;
		if ( PathFileExists(m_strDownTmpPath) )
		{//如果需要下载的文件存在，并且被占用，那么先重命名再重启删除
			BOOL bRet = DeleteFile(m_strDownTmpPath);
			if ( bRet == FALSE )
			{
				if( PathFileExists(m_strDownTmpPath + L"_tmp") )
					DeleteFile( m_strDownTmpPath + L"_tmp" );

				MoveFile(m_strDownTmpPath, m_strDownTmpPath + L"_tmp");
				MoveFileEx(m_strDownTmpPath + L"_tmp", NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
			
		}

		m_strDownLoadUrl = lpszDownLoadUrl;
		m_strParam		 = lpszParam;
		m_strDownloadTitle	= strTitle;
		m_bWait				= bWait;

		m_BKSafelog.SetLogForModule(BKSafeLog::enumBKSafeRunoptLog);
	}

	~CToolsDownloadDlg(void)
	{
	}

public:
	void OnBkBtnClose()
	{
		if ( IsItemVisible(51) )
		{
			EndDialog(IDCANCEL);
		}
		EnableItem(3, FALSE);
		m_bCancel = TRUE;

	}
	void OnBkBtnSureClose()
	{
		EndDialog(IDCANCEL);
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		SetItemVisible(4,FALSE);
		m_bShowMsg = FALSE;

		if (FALSE == m_strDownloadTitle.IsEmpty())
		{
			CString strTitle;
			strTitle.Format(BkString::Get(IDS_EXAM_1441), m_strDownloadTitle);
			SetItemText(1, strTitle);
		}

		HANDLE hThread = ::CreateThread(NULL, 0, _InstallThreadProc, this, 0, NULL);
		::CloseHandle(hThread);
		hThread = NULL;

		return TRUE;
	}

	virtual void BeginDownload()
	{
		SendMessage(MSG_SYSTOOLS_DOWN_BEGIN,NULL,NULL);
	}

	virtual void Retry( DWORD dwTimes ){}

	virtual BOOL DownloadProgress(
		DWORD dwTotalSize, 
		DWORD dwReadSize 
		)
	{
		if (!m_bCancel)
			PostMessage(MSG_SYSTOOLS_DOWN_PROC,(WPARAM)dwTotalSize,(LPARAM)dwReadSize);

		return !m_bCancel;
	}

	virtual void FinishDownload( HRESULT hError )
	{
		PostMessage(MSG_SYSTOOLS_DOWN_END, (WPARAM)hError,NULL);	
	}

	static DWORD WINAPI _InstallThreadProc(LPVOID pvParam)
	{
		if (NULL == pvParam)
			return -1;

		CToolsDownloadDlg *pThis = (CToolsDownloadDlg*)pvParam;
		pThis->_DownloadInstallPackage( pThis->GetDownTmpPath());
		return 0;
	}

	CString GetDownTmpPath()
	{
		return m_strDownTmpPath;
	}

	void _AppendUrlTail(CString& strUrl)
	{
		UUID uuid;
		WCHAR* pUuidStr = NULL;

		UuidCreate(&uuid);
		UuidToStringW(&uuid, (RPC_WSTR *)&pUuidStr);
		strUrl += "?rnd=";
		if (pUuidStr)
		{
			strUrl += pUuidStr;
		}
		else
		{
			CString strRnd;
			strRnd.Format(_T("%d"), GetTickCount());
			strUrl += strRnd;
		}

		if (pUuidStr)
		{
			RpcStringFreeW((RPC_WSTR *)&pUuidStr);
			pUuidStr = NULL;
		}
	}

	BOOL _DownloadInstallPackage(LPCTSTR lpszPath)
	{
		CUpdUtilityProxy downloader;
		CString strDownloadUrl = m_strDownLoadUrl;

		HRESULT hRet = downloader.Initialize();
		if (FAILED(hRet))
		{
			m_BKSafelog.WriteLog(L"Download: init error");
			goto Exit0;
		}

		_AppendUrlTail(strDownloadUrl);

		hRet = downloader.DownloadFile(strDownloadUrl, lpszPath, FALSE, NULL, this);
		if (FAILED(hRet))
		{
			m_BKSafelog.WriteLog(L"Download: DownloadFile Error.url = %s, path = %s", strDownloadUrl, lpszPath);
			goto Exit0;
		}

		if( 0 != CVerifyFileFunc::GetPtr()->CheckKingSoftFileSigner(lpszPath))
		{
			m_BKSafelog.WriteLog(L"Download: Check kingsoft singer error");
			goto Exit0;
		}

		if ( !m_bCancel && 
			_InstallPackage(lpszPath) )
		{
			hRet = S_OK;
		}
		else
		{
			if (TRUE == m_bCancel)
				hRet = -1;
		}
		m_BKSafelog.WriteLog(L"Download: install result = %d, bCancel = %d", hRet, m_bCancel);
Exit0:
		downloader.Uninitialize();
		PostMessage(MSG_SYSTOOLS_THREAD_END,(WPARAM)hRet,NULL);
		return SUCCEEDED(hRet);
	}

protected:
	DWORD m_nBeginCount;

	BOOL _InstallPackage(LPCTSTR lpszPath)
	{

		if (0 == m_strParam.CompareNoCase(KSafe_Tool_NOT_RUN))
			return TRUE;

		STARTUPINFO si = {sizeof(STARTUPINFO)};
		PROCESS_INFORMATION pi;
		int nProgress = 1;
		DWORD dwRet = 0;

 		CString strCmdLine;

		strCmdLine.Format(L"%s %s", lpszPath, m_strParam);
		
		m_BKSafelog.WriteLog(L"InstallPack: cmd = %s", strCmdLine);;
		BOOL bRet = ::CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		if (!bRet)
		{
			m_BKSafelog.WriteLog(L"Install Pack error. errorcode = %d", GetLastError());
			return FALSE;
		}

		::CloseHandle(pi.hThread);
		if( m_bWait == TRUE )
		{
			::WaitForSingleObject(pi.hProcess,INFINITE);
			::GetExitCodeProcess(pi.hProcess, &dwRet);
		}

		::CloseHandle(pi.hProcess);

		return (0 == dwRet);
	}

public:
	LRESULT OnDownBegin(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_nBeginCount = GetTickCount();
		return S_OK;
	}

	LRESULT OnDownProc(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_bShowMsg)
		{
			SetItemVisible(4,TRUE);
			m_bShowMsg = TRUE;
		}

		DWORD	nTotal = (DWORD)wParam;
		DWORD	nSize  = (DWORD)lParam;
		DWORD	nRate  = 0;

		if (nTotal!=0)
			nRate = (DWORD)(((float)nSize*100)/nTotal);

		TCHAR	szBuffer[100] ={0};
		_itot(nRate,szBuffer,10);
		SetItemStringAttribute(2,"value",szBuffer);

		CString	strText;
		strText.Format( _T("已下载：%s / %s"),
			GetSizeMB(nSize),
			GetSizeMB(nTotal)
			);

		SetItemText(4,strText);
		return S_OK;
	}

	CString GetSizeMB(DWORD nByte)
	{
		float	f = (float)nByte/(1024*1024);
		CString	str;
		str.Format(_T("%.2fMB"), f);
		return str;
	}

	LRESULT OnThreadEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_bCancel)
		{
			//如果用户取消下载了，那就判断下载下来的文件是不是有效，如果有无效就直接删除掉
			if (TRUE == ::PathFileExists(m_strDownTmpPath) &&
				0	 != CVerifyFileFunc::GetPtr()->CheckKingSoftFileSigner(m_strDownTmpPath))
			{
				if (FALSE == ::DeleteFile(m_strDownTmpPath))
					::MoveFileEx(m_strDownTmpPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}

			EndDialog(IDCANCEL);
			return S_OK;
		}
		if (SUCCEEDED(wParam))
		{
			EndDialog(IDOK);
			return S_OK;
		}
		else
		{
			SetItemVisible(50,FALSE);
			SetItemAttribute(52, "href", CW2A(m_strDownLoadUrl));
			SetItemVisible(51,TRUE);

			if (FALSE == m_strDownloadTitle.IsEmpty())
			{
				CString strTitle;
				strTitle.Format(BkString::Get(IDS_EXAM_1442), m_strDownloadTitle);
				SetItemText(53, strTitle);

				strTitle.Format(BkString::Get(IDS_EXAM_1443), m_strDownloadTitle);
				SetItemText(52, strTitle);
			}
		}
		return S_OK;
	}


	LRESULT OnDownEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return S_OK;
	}

protected:
	BOOL						m_bShowMsg;
	BOOL						m_bCancel;
	BOOL						m_bWait;
	CString						m_strDownTmpPath;
	CString						m_strDownLoadUrl;
	CString						m_strParam;
	BKSafeLog::CBKSafeLog		m_BKSafelog;
	CString						m_strDownloadTitle;
	


public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(60001, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(3,OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(101,OnBkBtnSureClose)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CToolsDownloadDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CToolsDownloadDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CToolsDownloadDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(MSG_SYSTOOLS_DOWN_BEGIN,OnDownBegin)
		MESSAGE_HANDLER(MSG_SYSTOOLS_DOWN_PROC,OnDownProc)
		MESSAGE_HANDLER(MSG_SYSTOOLS_DOWN_END,OnDownEnd)
		MESSAGE_HANDLER(MSG_SYSTOOLS_THREAD_END,OnThreadEnd)
	END_MSG_MAP()
};
