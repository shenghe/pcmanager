#pragma once


#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include <plugin/interface.h>
#include <common/utility.h>
#include "beikecolordef.h"
#include "beikesafemsgbox.h"
#include <miniutil/bkprocprivilege.h>
#include "common/KwsSettingOld.h"

class CIeLockDlg
	: public CBkDialogImpl<CIeLockDlg>
	, public CWHRoundRectFrameHelper<CIeLockDlg>
{
public:
	CIeLockDlg(BOOL b, int n)
		: CBkDialogImpl<CIeLockDlg>(IDR_BK_IELOCK_DLG)
	{
		m_pComBoBox = NULL;
		m_bReboot = b;
		m_nCnt = n;
	}
	~CIeLockDlg(void)
	{

	}
public:
	int _ReBoot()
	{
		CBkProcPrivilege privilege;

		if (!privilege.EnableShutdown())
			return -1;

		if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 
			SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
			SHTDN_REASON_MINOR_HOTFIX |
			SHTDN_REASON_FLAG_PLANNED)) 
			return 0;

		return -1;
	}
	void OnBkClose()
	{
		EndDialog(0);
	}
	void OnReboot()
	{
		//重新启动
		if (GetItemCheck(IDC_IEFIX_TEXT_LNK_SELECTALL))//选中了
		{
			ATL::CString strWebName;
			ATL::CString strWebUrl;
			CSafeMonitorTrayShell trayShell;

			if( m_pComBoBox )
			{
				strWebUrl = m_pComBoBox->GetInputWebUrl();
				strWebName = m_pComBoBox->GetInputWebName();
				ATL::CString strTrimed = strWebUrl.Trim();

				if ( strTrimed.GetLength() == 0 )
				{
					strWebUrl = TEXT("about:blank");
					strWebName = CSettingHlp::Instance().UrlToName((LPCTSTR)strWebUrl);
				}
			}

			kws_old_ipc::KwsSetting setting;

			trayShell.SetDefaultHomepage(strWebUrl);
			setting.SetStartPage( strWebUrl );
			setting.SetLockSPEnable( TRUE );
			strWebName = CSettingHlp::Instance().UrlToName((LPCTSTR)strWebUrl);

			CSettingHlp::Instance().RegSetStartPage(strWebUrl, HKEY_CURRENT_USER);
			CSettingHlp::Instance().RegSetStartPage(strWebUrl, HKEY_LOCAL_MACHINE);

			if( m_pComBoBox )
			{
				strWebName = CSettingHlp::Instance().UrlToName( strWebUrl );
				strWebUrl = CSettingHlp::Instance().NameToUrl( strWebName );
				m_pComBoBox->AddString( strWebUrl, strWebName );
			}
		}		
		_ReBoot();
		EndDialog(1);
	}
	void OnRebootLater()
	{
		//稍后启动		
		if (GetItemCheck(IDC_IEFIX_TEXT_LNK_SELECTALL))//选中了
		{
			ATL::CString strWebName;
			ATL::CString strWebUrl;
			CSafeMonitorTrayShell trayShell;

			if( m_pComBoBox )
			{
				strWebUrl = m_pComBoBox->GetInputWebUrl();
				strWebName = m_pComBoBox->GetInputWebName();
				ATL::CString strTrimed = strWebUrl.Trim();

				if ( strTrimed.GetLength() == 0 )
				{
					strWebUrl = TEXT("about:blank");
					strWebName = CSettingHlp::Instance().UrlToName((LPCTSTR)strWebUrl);
				}
			}

			kws_old_ipc::KwsSetting setting;

			trayShell.SetDefaultHomepage(strWebUrl);
			setting.SetStartPage( strWebUrl );
			setting.SetLockSPEnable( TRUE );
			strWebName = CSettingHlp::Instance().UrlToName((LPCTSTR)strWebUrl);

			CSettingHlp::Instance().RegSetStartPage(strWebUrl, HKEY_CURRENT_USER);
			CSettingHlp::Instance().RegSetStartPage(strWebUrl, HKEY_LOCAL_MACHINE);

			if( m_pComBoBox )
			{
				strWebName = CSettingHlp::Instance().UrlToName( strWebUrl );
				strWebUrl = CSettingHlp::Instance().NameToUrl( strWebName );
				m_pComBoBox->AddString( strWebUrl, strWebName );
			}
		}
		
		EndDialog(0);
	}


	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{	
		if( !m_pComBoBox )
		{
			m_pComBoBox = new KComboBox;
			CSettingHlp::Instance().CreateComBoBox(GetViewHWND() , m_pComBoBox, IDC_KWS_COMBOBOX_CTRL );
		}
		ATL::CString strX;
		strX.Format(_T("已经成功修复 %d 项异常"), m_nCnt);
		this->SetItemVisible(IDC_KWS_COMBOBOX_CTRL, TRUE);
		this->SetItemCheck(IDC_IEFIX_TEXT_LNK_SELECTALL,TRUE);
		this->SetItemText(IDC_TXT_TEXT_DANER_NUM, strX);

		if (m_bReboot)
		{
			this->SetItemVisible(IDC_TXT_BTM_SURE, FALSE);
			this->SetItemVisible(IDC_TXT_TEXT_LOCK, FALSE);
			this->SetItemVisible(IDC_TXT_BTM_REBOOT, TRUE);
			this->SetItemVisible(IDC_TXT_BTM_REBOOT_LATER, TRUE);
			this->SetItemVisible(IDC_TXT_TEXT_REBOOT, TRUE);
		}
		else
		{			
			this->SetItemVisible(IDC_TXT_BTM_REBOOT, FALSE);
			this->SetItemVisible(IDC_TXT_BTM_REBOOT_LATER, FALSE);
			this->SetItemVisible(IDC_TXT_TEXT_REBOOT, FALSE);
			this->SetItemVisible(IDC_TXT_TEXT_LOCK, TRUE);
			this->SetItemVisible(IDC_TXT_BTM_SURE, TRUE);
		}
		return TRUE;
	}

protected:
	KComboBox*			m_pComBoBox;
	BOOL				m_bReboot;//是否需要重启
	int					m_nCnt;//异常的数目
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_TXT_BTM_REBOOT_LATER, OnRebootLater)//稍后启动
		BK_NOTIFY_ID_COMMAND(IDC_TXT_BTM_CLOSE, OnBkClose)//关闭按钮
		BK_NOTIFY_ID_COMMAND(IDC_TXT_BTM_REBOOT, OnReboot)//重新启动
		BK_NOTIFY_ID_COMMAND(IDC_TXT_BTM_SURE, OnRebootLater)//确定		
		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CIeLockDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CIeLockDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CIeLockDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		END_MSG_MAP()
};
