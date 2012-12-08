#pragma once

#include "ignoredlist.h"
#include <iefix/cinifile.h>
#include <communits/CmdLineParser.h>
using namespace IniFileOperate;

#define CONFIG_FILEPATH			L"\\Cfg\\bksafe.ini"
#define CONFIG_SEC_NAME			L"runopt"

class CMsgBoxDisableByX
	: public CBkDialogImpl<CMsgBoxDisableByX>
	, public CWHRoundRectFrameHelper<CMsgBoxDisableByX>
{
public:
	CMsgBoxDisableByX(CBeikeSafeMainDlg* refDlg)
		: CBkDialogImpl<CMsgBoxDisableByX>(IDR_BK_RUNOPT_DISABLE_BY3XX)
	{
		m_pMainDlg = refDlg;
	}

	~CMsgBoxDisableByX(void)
	{
	}

	BOOL OnInitDialog(HWND,LPARAM)
	{
		CAppPath::Instance().GetLeidianAppPath(m_strIniPath);
		m_strIniPath.Append(CONFIG_FILEPATH);
		return TRUE;
	}

	BOOL IsDisableAutoRunBy360()
	{
		BOOL bResult = FALSE;
		TCHAR szFilePath[MAX_PATH + 1];
		DWORD cbData;
		DWORD dwType;
		LRESULT lResult;

		cbData = MAX_PATH * sizeof (TCHAR);
		lResult = SHGetValue(HKEY_LOCAL_MACHINE, 
			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\360safe.exe"),
			_T("Path"), 
			&dwType, 
			szFilePath, 
			&cbData
			);
		if ( lResult == ERROR_SUCCESS && dwType == REG_SZ )
		{
			szFilePath[MAX_PATH] = 0;
			if ( MyGetPathType(szFilePath) == 2 )
			{
				_tcsncat(szFilePath, _T("\\SoftMgr\\somextrainfo.ini"), MAX_PATH);
				if ( MyGetPathType(szFilePath) == 1 )
				{
					const DWORD nBuffLen = 1024 * 4;
					auto_ptr<TCHAR> buf(new TCHAR[nBuffLen + 1]);

					GetPrivateProfileString(_T("AllRuns"), _T("value"), _T(""), buf.get(), nBuffLen, szFilePath);

					LPTSTR lpStr1 = StrStrI(buf.get(), _T("KSafeTray"));
					if ( lpStr1 != NULL )
					{
						TCHAR szSectName[MAX_PATH + 1];

						LPTSTR lpStr2 = StrStrI(lpStr1, _T("###"));
						if ( lpStr2 != NULL )
						{
							int nLen = lpStr2 - lpStr1;
							nLen = min(MAX_PATH, nLen);

							_tcsncpy(szSectName, lpStr1, nLen);
							szSectName[nLen] = 0;
						}
						else
						{
							_tcsncpy(szSectName, lpStr1, MAX_PATH);
							szSectName[MAX_PATH] = 0;
						}

						LPTSTR lpStr3 = StrStrI(szSectName, _T("$$$"));
						if ( lpStr3 != NULL )
						{
							LPTSTR lpStr4 = lpStr3 + 3;
							while ( *lpStr4 != 0 )
							{
								*lpStr3++ = *lpStr4++;
							}
							*lpStr3 = 0;
						}

						GetPrivateProfileString(szSectName, _T("value"), _T(""), buf.get(), nBuffLen, szFilePath); 
						if ( buf.get()[0] != 0 )
						{
							return TRUE;
						}
					}
				}
			}
		}

		cbData = MAX_PATH * sizeof (TCHAR);
		lResult = SHGetValue(HKEY_LOCAL_MACHINE, 
			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\360Disabled"),
			_T("KSafeTray"),
			&dwType,
			szFilePath,
			&cbData
			);
		if ( lResult == ERROR_SUCCESS && dwType == REG_SZ )
		{
			szFilePath[MAX_PATH] = 0;
			CCmdLineParser cmdLine(szFilePath);
			CString strFilePath = cmdLine.GetCmd();
			if ( MyGetPathType(strFilePath) == 1 )
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	void OnBkBtnOK()
	{
		EndDialog(IDOK);
	}
	
	void OnBkBtnCancel()
	{
		if (m_pMainDlg)
			::PostMessage(m_pMainDlg->m_hWnd, MSG_SYSOPT_RESTORE_DELAYRUN, 0, 0);
		EndDialog(IDCANCEL);
	}

	void OnBkLnkShowMoreInfo()
	{
		::ShellExecute(NULL,_T("open"),_T("http://bbs.ijinshan.com/thread-58507-1-1.html"),NULL,NULL,SW_SHOW);
	}

	void OnBkBtnOpen3xx()
	{
		CRegKey regKey;
		LRESULT lResult;

		lResult = regKey.Open(HKEY_LOCAL_MACHINE, 
			_T("SOFTWARE\\360Safe\\360krnlsvc\\softmgrs"),
			KEY_READ
			);
		if ( lResult == ERROR_SUCCESS )
		{
			TCHAR szFilePath[MAX_PATH];
			DWORD cbLen = MAX_PATH;

			lResult = regKey.QueryStringValue(_T("ImagePath"), szFilePath, &cbLen);
			if ( lResult == ERROR_SUCCESS )
			{
				::PathRemoveFileSpec(szFilePath);
				::PathAppend(szFilePath, TEXT("AdvUtils.exe"));
				if (::PathFileExists(szFilePath))
				{
					// ´ò¿ª360
					::ShellExecute(NULL, _T("Open"), szFilePath, NULL, NULL, SW_SHOWNORMAL);
				}
			}

			regKey.Close();
		}
		EndDialog(IDOK);
	}

private:
	CString m_strIniPath;
	CBeikeSafeMainDlg* m_pMainDlg;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnOK)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_MOREINFO_CTRL, OnBkLnkShowMoreInfo)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_OPEN_3XX_CTRL, OnBkBtnOpen3xx)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_CLOSE_CTRL ,OnBkBtnCancel)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CMsgBoxDisableByX)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CMsgBoxDisableByX>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CMsgBoxDisableByX>)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()
};