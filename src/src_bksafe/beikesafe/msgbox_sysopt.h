#pragma once

#define MSG_BOX_RUNOPT_TYPE1	1	//检测到卫士启动项不在时的提示
#define MSG_BOX_RUNOPT_TYPE2	2	//延迟360时的提示
#define MSG_BOX_RUNOPT_TYPE3	3	//延迟安全类软件时的提示
#define MSG_BOX_RUNOPT_TYPE4	4	//延迟自保护类软件时的提示
#define MSG_BOX_RUNOPT_TYPE5	5	//禁止360时的提示
#define MSG_BOX_RUNOPT_TYPE6	6	//禁止QQ电脑管家时的提示
#define MSG_BOX_RUNOPT_TYPE7	7	//禁止360杀毒时的提示
#define MSG_BOX_RUNOPT_TYPE8	8	//延迟自身启动项时的提示
#define MSG_BOX_RUNOPT_TYPE9	9	//延迟启动失效时再去进行延迟操作的提示

#define ID_RESTORE_KSAFETRAY	10

class CMsgBoxSysopt
	: public CBkDialogImpl<CMsgBoxSysopt>
	, public CWHRoundRectFrameHelper<CMsgBoxSysopt>
{
public:
	CMsgBoxSysopt(int nType = 1, CBeikeSafeMainDlg* refDlg = NULL)
		: CBkDialogImpl<CMsgBoxSysopt>(IDR_BK_RUNOPT_MSGBOX)
	{
		m_nShowType = nType;
		m_pMainDlg = refDlg;
	}

	~CMsgBoxSysopt(void)
	{
	}

	void OnBkBtnOK()
	{
		EndDialog(IDOK);
	}

	void OnBkBtnCancel()
	{
		if (m_nShowType == MSG_BOX_RUNOPT_TYPE1 
			&& m_pMainDlg)
			::PostMessage(m_pMainDlg->m_hWnd, MSG_SYSOPT_RESTORE_DELAYRUN, 0, 0);
		EndDialog(IDCANCEL);
	}

	BOOL OnInitDialog(HWND, LPARAM)
	{
		SetItemVisible(IDC_BTN_RESTORE_MSG_RUNOPT, (m_nShowType == MSG_BOX_RUNOPT_TYPE1 || m_nShowType == MSG_BOX_RUNOPT_TYPE9));
		SetItemVisible(IDC_BTN_FEEDBACK_MSG_RUNOPT, !(m_nShowType == MSG_BOX_RUNOPT_TYPE1 || m_nShowType == MSG_BOX_RUNOPT_TYPE9));
		SetItemVisible(IDC_DIV_MSG_RUNOPT_1, m_nShowType == MSG_BOX_RUNOPT_TYPE1);
		SetItemVisible(IDC_DIV_MSG_RUNOPT_2, m_nShowType == MSG_BOX_RUNOPT_TYPE2);
		SetItemVisible(IDC_DIV_MSG_RUNOPT_3, m_nShowType == MSG_BOX_RUNOPT_TYPE3);
		SetItemVisible(IDC_DIV_MSG_RUNOPT_4, m_nShowType == MSG_BOX_RUNOPT_TYPE4);
		SetItemVisible(IDC_DIV_MSG_RUNOPT_5, m_nShowType == MSG_BOX_RUNOPT_TYPE5);
		SetItemVisible(IDC_DIV_MSG_RUNOPT_6, m_nShowType == MSG_BOX_RUNOPT_TYPE6);
		SetItemVisible(IDC_DIV_MSG_RUNOPT_7, m_nShowType == MSG_BOX_RUNOPT_TYPE7);
		SetItemVisible(IDC_DIV_MSG_RUNOPT_8, m_nShowType == MSG_BOX_RUNOPT_TYPE8);
		SetItemVisible(IDC_DIV_MSG_RUNOPT_9, m_nShowType == MSG_BOX_RUNOPT_TYPE9);
		return TRUE;
	}

	void OnBkFeedBackClick()
	{
		::ShellExecute(NULL,_T("open"),_T("http://bbs.ijinshan.com/forum-18-1.html"),NULL,NULL,SW_SHOW);
		EndDialog(IDOK);
	}

	void OnBkijinshanBBS()
	{
		::ShellExecute(NULL,_T("open"),_T("http://bbs.ijinshan.com"),NULL,NULL,SW_SHOW);
	}

	void SetTrayAutorun(BOOL bOn)
	{
		//
		CRegKey reg;
		LONG lResult;

		lResult = reg.Create(HKEY_LOCAL_MACHINE, 
			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 
			NULL, 
			NULL, 
			KEY_WRITE
			);
		if ( lResult == ERROR_SUCCESS )
		{
			if ( bOn )
			{
				CString strCmdLine;

				CAppPath::Instance().GetLeidianAppPath(strCmdLine);
				strCmdLine.Insert(0, _T('\"'));
				strCmdLine.Append(_T("\\KSafeTray.exe\" -autorun"));

				reg.SetStringValue(_T("KSafeTray"), strCmdLine);
			}
			else
			{
				reg.DeleteValue(_T("KSafeTray"));
			}
		}
	}

	BOOL IsTrayAutorun()
	{
		BOOL bResult = FALSE;
		CRegKey reg;
		LONG lResult;

		lResult = reg.Open(HKEY_LOCAL_MACHINE, 
			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 
			KEY_READ
			);
		if ( lResult == ERROR_SUCCESS )
		{
			CString strCmdLine;
			DWORD dwCmdLineLen = MAX_PATH + 100;

			lResult = reg.QueryStringValue(_T("KSafeTray"), strCmdLine.GetBuffer(dwCmdLineLen), &dwCmdLineLen);
			strCmdLine.ReleaseBuffer();

			if ( lResult == ERROR_SUCCESS )
			{
				CString strRightCmdLine;

				CAppPath::Instance().GetLeidianAppPath(strRightCmdLine);
				strRightCmdLine.Insert(0, _T('\"'));
				strRightCmdLine.Append(_T("\\KSafeTray.exe\" -autorun"));

				if ( strRightCmdLine.CompareNoCase(strCmdLine) == 0 )
				{
					bResult = TRUE;
				}
				else
				{
					CRegKey regWrite;

					lResult = regWrite.Create(HKEY_LOCAL_MACHINE, 
						_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 
						NULL, 
						NULL, 
						KEY_WRITE
						);
					if ( lResult == ERROR_SUCCESS )
					{
						regWrite.DeleteValue(_T("KSafeTray"));
					}
				}
			}
		}

		return bResult;
	}
	//从注册表禁用目录中删除自身启动项
	void DelTrayFromDisDir()
	{
		CRegKey regWrite;

		LONG lResult = regWrite.Create(HKEY_LOCAL_MACHINE, 
			_T("Software\\KSafe\\run\\Disable\\hklm"), 
			NULL, 
			NULL, 
			KEY_WRITE
			);
		if ( lResult == ERROR_SUCCESS )
		{
			regWrite.DeleteValue(_T("KSafeTray"));
		}
	}

	void OnEnableAutoRun()
	{
		if (IsTrayAutorun() == FALSE)
		{
			CRestoreRunner* pLogRunner = new CRestoreRunner;
			pLogRunner->DeleteItemFromLog(_T("KSafeTray"), KSRUN_TYPE_STARTUP, 590);
			delete pLogRunner;
			SetTrayAutorun(TRUE);
			DelTrayFromDisDir();
			CSafeMonitorTrayShell x;
			x.SetAllMonitorRun(TRUE);
			x._SetAllMonitorRun(TRUE);
			if (m_pMainDlg)
			{
				::PostMessage(m_pMainDlg->m_hWnd, MSG_SYSOPT_REFRESH_ITEMLIST, 0, 0);
			}
		}
		EndDialog(ID_RESTORE_KSAFETRAY);
	}

private:
	CString m_strIniPath;
	DWORD	m_nShowType;
	CBeikeSafeMainDlg* m_pMainDlg;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnOK)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_CLOSE_MSG_RUNOPT, OnBkBtnCancel)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_FEEDBACK_MSG_RUNOPT, OnBkFeedBackClick)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_IJINSHAN_BBS, OnBkijinshanBBS)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_RESTORE_MSG_RUNOPT, OnEnableAutoRun)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CMsgBoxSysopt)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CMsgBoxSysopt>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CMsgBoxSysopt>)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()
};