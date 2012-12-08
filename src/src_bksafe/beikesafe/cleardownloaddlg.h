
#pragma once

#include <uplive/updproxy.h>
#include "LockHelper.h"

#define KCLEAR_INSTALLER_DOWNLOAD_URL			_T("http://dl.ijinshan.com/safe/clear.exe")
#define KCLEAR_INSTALLER_TEMP_FILE_NAME			_T("\\ksafe_setup_clear.exe")

#define KCLEAR_DIR_NAME		_T("KClear")
#define KCLEAR_EXE_NAME		_T("KClear.exe")

#define KCLEAR_STATUS_KEY_NAME       L"SOFTWARE\\KSafe\\KClear"
#define KCLEAR_STATUS_VALUE_NAME     L"Install"

#define MSG_KCLEAR_DOWN_BEGIN		(WM_APP+1)
#define MSG_KCLEAR_DOWN_PROC		(WM_APP+2)
#define MSG_KCLEAR_DOWN_END			(WM_APP+3)
#define MSG_KCLEAR_THREAD_END		(WM_APP+4)

#define RUN_TYPE_KCLEAR_HISTORY		1
#define RUN_TYPE_KCLEAR_RABBISH		2
#define RUN_TYPE_KCLEAR_REG			3


class CClearModHelper
{
public:
	CClearModHelper()
	{
		CAppPath::Instance().GetLeidianTempPath(m_strDownTmpPath,TRUE);
		m_strDownTmpPath += KCLEAR_INSTALLER_TEMP_FILE_NAME;

		CAppPath::Instance().GetLeidianAppPath(m_strExePath);
		m_strExePath.Append(_T("\\") KCLEAR_DIR_NAME _T("\\") KCLEAR_EXE_NAME );
	}

	virtual ~CClearModHelper(){}
	
	static CClearModHelper* GetPtr()
	{
		static CClearModHelper x;
		return &x;
	}

	BOOL IsClearModInstall()
	{
		return FALSE;
	}

	LPCTSTR GetExePath()
	{
		return m_strExePath;
	}

	LPCTSTR GetDownTmpPath()
	{
		return m_strDownTmpPath;
	}

	HWND FindTuziWindow()
	{
		HWND hWnd = ::FindWindow(_T("{C9A5B730-1E5F-49A2-AAD5-025BBD1F43F0}"), NULL);
		return hWnd;
	}

	void BringTuziToTop(HWND hWnd)
	{
		if ( !::IsWindow(hWnd) )
			return;

		if ( ::IsIconic(hWnd) )
		{
			::ShowWindow(hWnd,SW_RESTORE);
		}

		::BringWindowToTop(hWnd);
	}

	BOOL RunClearType(int nType, HWND hWndMy=NULL)
	{
		static LPCTSTR lpParam[] = 
		{
			_T("/ClearRubbish"),_T("/Clearhengji"), _T("/ClearReg")
		};
		
		HWND	hWndTuzi = FindTuziWindow();
		CString	strParam;
		
		strParam.Format(_T("\"%s\""), m_strExePath );
		if (nType <= sizeof(lpParam)/sizeof(lpParam[0]) )
		{
			if ( nType!=3 || hWndTuzi!=NULL )
			{
				strParam.Append(_T(" "));
				strParam.Append(lpParam[nType-1]);
			}
		}

		STARTUPINFO si = {sizeof(STARTUPINFO)};
		PROCESS_INFORMATION pi = {0};
		int nProgress = 1;
		DWORD dwRet = 0;

		BOOL bRet = ::CreateProcess(NULL, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		if (!bRet)
			return FALSE;

		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);

		if (hWndTuzi!=NULL)
			BringTuziToTop(hWndTuzi);


		HWND hWnd = hWndTuzi; 
		
		if ( hWnd == NULL )
		{
			for (int k = 0; k < 30; k++)
			{				
				hWnd = FindTuziWindow();
				if (hWnd!=NULL)
					break;

				Sleep(100);
				}
			if (hWnd && nType==3)
				::PostMessage( hWnd, (WM_USER+102), NULL, NULL);
		}
		
		if (hWnd!=NULL)
		{
			WIN32_FIND_DATA wfd = {0};
			HANDLE hFile = FindFirstFile(m_strExePath,&wfd);
			if (hFile!=INVALID_HANDLE_VALUE )
			{
				::FindClose(hFile);
				if ( wfd.nFileSizeLow == 2572152 )
				{
					for (int i=0; i < 20; i++)
					{
						HWND hpanel = ::GetWindow(hWnd,GW_CHILD);
						HWND hpanel1 = ::GetWindow(hpanel,GW_HWNDNEXT);
						HWND hpanel2 = ::GetWindow(hpanel1,GW_HWNDNEXT);
						HWND hpanel3 = ::GetWindow(hpanel2,GW_HWNDNEXT);
						HWND hbutton = ::GetWindow(hpanel3,GW_CHILD);

						if (hbutton)
						{
							TCHAR cname[MAX_PATH] = _T("");
							GetClassName( hbutton, cname, sizeof(cname) );
							if( _tcsicmp(cname,_T("TRzBmpButton")) == 0 )
							{
								::ShowWindow(hbutton,SW_HIDE);
								break;
							}
						}
						Sleep(100);
					}
				}	
			}
		}
		
		if (hWndMy!=NULL && bRet)
		{
			::PostMessage(hWndMy,WM_LBUTTONUP,NULL,NULL);
		}
		
		return TRUE;
	}

	BOOL IsInstalled()
	{
		DWORD	nType = REG_DWORD;
		DWORD	nValue = 0;
		DWORD	nSize = sizeof(DWORD);
		::SHGetValue(HKEY_LOCAL_MACHINE,KCLEAR_STATUS_KEY_NAME,
			KCLEAR_STATUS_VALUE_NAME,&nType,&nValue,&nSize);

		if (nValue==1 && PathFileExists(m_strExePath) )
		{
			return TRUE;
		}
		return FALSE;
	}

	void SetInstallMark()
	{
		if ( PathFileExists(m_strExePath) )
		{
			DWORD	nInstalled = 1;
			::SHSetValue(HKEY_LOCAL_MACHINE,KCLEAR_STATUS_KEY_NAME,
				KCLEAR_STATUS_VALUE_NAME,REG_DWORD,&nInstalled,sizeof(DWORD));
		}
	}
	
protected:
	CString	m_strExePath;
	CString	m_strDownTmpPath;
};

class CCleardownLoadDlg
	: public CBkDialogImpl<CCleardownLoadDlg>
	, public CWHRoundRectFrameHelper<CCleardownLoadDlg>
	, public IUpdateDownloadFileCallback
{
public:
	CCleardownLoadDlg()
		: CBkDialogImpl<CCleardownLoadDlg>(IDR_BK_CLEAR_DOWNLOAD)
	{
		m_bCancel = FALSE;
	}

	~CCleardownLoadDlg(void)
	{
	}

public:
	void OnBkBtnClose()
	{
		if ( IsItemVisible(51) )
		{
			EndDialog(IDCANCEL);
		}
		EnableItem(3,FALSE);
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

		HANDLE hThread = ::CreateThread(NULL, 0, _InstallThreadProc, this, 0, NULL);
		::CloseHandle(hThread);
		hThread = NULL;

		return TRUE;
	}

	virtual void BeginDownload()
	{
		SendMessage(MSG_KCLEAR_DOWN_BEGIN,NULL,NULL);
	}

	virtual void Retry( DWORD dwTimes ){}

	virtual BOOL DownloadProgress(
		DWORD dwTotalSize, 
		DWORD dwReadSize 
		)
	{
		if (!m_bCancel)
			PostMessage(MSG_KCLEAR_DOWN_PROC,(WPARAM)dwTotalSize,(LPARAM)dwReadSize);

		return !m_bCancel;
	}

	virtual void FinishDownload( HRESULT hError )
	{
		PostMessage(MSG_KCLEAR_DOWN_END,(WPARAM)hError,NULL);	
	}

	static DWORD WINAPI _InstallThreadProc(LPVOID pvParam)
	{
		if (NULL == pvParam)
			return -1;

		CCleardownLoadDlg *pThis = (CCleardownLoadDlg*)pvParam;
		pThis->_DownloadInstallPackage( CClearModHelper::GetPtr()->GetDownTmpPath() );
		return 0;
	}

	BOOL _DownloadInstallPackage(LPCTSTR lpszPath)
	{
		CUpdUtilityProxy downloader;

		HRESULT hRet = downloader.Initialize();
		if (FAILED(hRet))
			goto Exit0;

		hRet = downloader.DownloadFile(KCLEAR_INSTALLER_DOWNLOAD_URL, lpszPath, FALSE, NULL, this);
		if (FAILED(hRet))
			goto Exit0;

		if ( !m_bCancel && _InstallPackage(lpszPath) )
			hRet = S_OK;
		else
			hRet = -1;
Exit0:
		downloader.Uninitialize();
		PostMessage(MSG_KCLEAR_THREAD_END,(WPARAM)hRet,NULL);
		return SUCCEEDED(hRet);
	}

protected:
	DWORD m_nBeginCount;

	BOOL _InstallPackage(LPCTSTR lpszPath)
	{
		STARTUPINFO si = {sizeof(STARTUPINFO)};
		PROCESS_INFORMATION pi;
		int nProgress = 1;
		DWORD dwRet = 0;

		CString strCmdLine;
		strCmdLine += L"\"" ;
		strCmdLine += lpszPath;
		strCmdLine += L"\" /S /D2=\"";
		strCmdLine += _Module.GetAppDirPath();
		strCmdLine += L"\"";

		BOOL bRet = ::CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		if (!bRet)
			return FALSE;

		::CloseHandle(pi.hThread);
		::WaitForSingleObject(pi.hProcess,INFINITE);
		::GetExitCodeProcess(pi.hProcess, &dwRet);
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
		strText.Format( _T("ÒÑ¼ÓÔØ£º%s / %s"),
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
			EndDialog(IDCANCEL);
			return S_OK;
		}
		if (SUCCEEDED(wParam))
		{
			CClearModHelper::GetPtr()->SetInstallMark();
			EndDialog(IDOK);
			return S_OK;
		}
		else
		{
			SetItemVisible(50,FALSE);
			SetItemVisible(51,TRUE);
		}
		return S_OK;
	}


	LRESULT OnDownEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return S_OK;
	}

protected:
	BOOL	m_bShowMsg;
	BOOL	m_bCancel;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(3, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(100,OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(101,OnBkBtnSureClose)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CCleardownLoadDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CCleardownLoadDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CCleardownLoadDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(MSG_KCLEAR_DOWN_BEGIN,OnDownBegin)
		MESSAGE_HANDLER(MSG_KCLEAR_DOWN_PROC,OnDownProc)
		MESSAGE_HANDLER(MSG_KCLEAR_DOWN_END,OnDownEnd)
		MESSAGE_HANDLER(MSG_KCLEAR_THREAD_END,OnThreadEnd)
	END_MSG_MAP()
};
