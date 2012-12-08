#pragma once
//设置下载保护的浏览器
#include "kws/ctrls/rulelistctrl.h"

class CSetBrowerPage 
	: public CBkDialogImpl<CSetBrowerPage>
	,public CWHRoundRectFrameHelper<CSetBrowerPage>
{
public:
	CSetBrowerPage()
		: CBkDialogImpl<CSetBrowerPage>(IDR_BK_SAFEKWS_BROWS_SETTING_DLG)
	{
	}

	~CSetBrowerPage(void)
	{
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{		
		return 0;
	}
	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{		
		InitCtrls();
		return TRUE;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}
	void OnBkBtnClose()
	{
		EndDialog(0);
		return;
	}
	void InitCtrls()
	{
		m_rtFile = CRect(13, 72, 13 + 463, 360);
		m_browers.SetRowHeight( 24 );
		m_browers.Create( 	GetViewHWND(), m_rtFile, 
							NULL, 
							WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_OWNERDRAWFIXED, 
							0 ,
							IDC_LIST_PROBROWS_DLG);

		m_browers.SetNotifyWnd(m_hWnd);
		m_browers.HeaderSubclassWindow();
		m_browers.InsertColumn(0, L"浏览器程序名称",	LVCFMT_LEFT, 395);
		m_browers.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | m_browers.GetExtendedListViewStyle());
		m_rtFile.InflateRect(1, 1);
		LoadBrowerList();
		return;
	}
	void LoadBrowerList()
	{
		KwsSetting  setting;
		std::vector<wstring>::iterator it;

		m_process.clear();

		/* Load browers file name by setting */
		setting.GetShieldProcess(m_process);

		for (it = m_process.begin(); it != m_process.end(); ++it)
		{
			int i = m_browers.GetItemCount();
			m_browers.InsertItem(i, CW2T((*it).c_str()));
		}
		EnableItem( IDC_LINKTEXT_PROBROWS_DEL,   m_browers.GetSelectedIndex() != -1 );
		return;
	}

	LRESULT OnListViewItemChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	{
		if (m_browers.IsWindow())
		{
			EnableItem( IDC_LINKTEXT_PROBROWS_DEL,   m_browers.GetSelectedIndex() != -1 );
		}
		
		return TRUE;
	}
	
	BOOL IsExistBrower(std::wstring& name)
	{
		std::vector<wstring>::iterator it;

		for (it=m_process.begin(); it!=m_process.end(); ++it)
		{
			if (name == (*it))
				return TRUE;
		}
		return FALSE;
	}
	void OnBkBtnAdd()//添加浏览器
	{
		WCHAR szFilter[] = L"All Files (*.exe)\0*.exe\0\0";
		CFileDialog dlg(TRUE, NULL, NULL, 
			OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, m_hWnd);
		if (dlg.DoModal() == IDOK)
		{
			std::wstring procname = dlg.m_ofn.lpstrFile;

			if (!IsExistBrower(procname))
			{
				//加个判断
				//if (CBkSafeMsgBox2::ShowMultLine( L"您是否确定要添加这个程序？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING) == IDOK)	
				//{
					int i = m_browers.GetItemCount();
					m_browers.InsertItem(i, CW2T(procname.c_str()));
					m_process.push_back(procname);
					DoSettingApply();
					EnableItem( IDC_LINKTEXT_PROBROWS_DEL,   m_browers.GetSelectedIndex() != -1 );
				//}				
			}
			else
			{
				CBkSafeMsgBox2::ShowMultLine( L"您选中的程序已经存在，请选择其他的程序！", L"金山卫士",  MB_OK|MB_ICONWARNING );
			}
		}
		return;
	}

	void UpdateBrowerList()
	{
		std::vector<wstring>::iterator it;
		/* Refresh the list */
		m_browers.DeleteAllItems();
		for (it = m_process.begin(); it != m_process.end(); ++it)
		{
			int i = m_browers.GetItemCount();
			m_browers.InsertItem(i, CW2T((*it).c_str()));
		}
		EnableItem( IDC_LINKTEXT_PROBROWS_DEL,   m_browers.GetSelectedIndex() != -1 );
		return;
	}
	void OnBkBtnDel()//删除浏览器
	{
		int idx = m_browers.GetSelectedIndex();
		if (idx >= 0)
		{
			if (CBkSafeMsgBox2::ShowMultLine( L"您是否确定删除选中的程序？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING) == IDOK)	
			{
				TCHAR szProcPath[MAX_PATH] = {0};
				m_browers.GetItemText(idx, 0, szProcPath, sizeof(szProcPath)/sizeof(TCHAR));

				for (size_t i = 0; i < m_process.size(); ++i)
				{
					if (wcscmp(m_process[i].c_str(), CT2W(szProcPath)) == 0)
					{
						m_process.erase(m_process.begin() + i);
						break;
					}
				}

				m_browers.DeleteItem(idx);
				UpdateBrowerList();
				DoSettingApply();//应用
			}			
		}
		return;
	}

	BOOL DoSettingApply()
	{
		/* Do save current setting */

		KwsSetting  setting;

		if (setting.SetShieldProcess(m_process))
		{
			CSafeMonitorTrayShell::KwsUserProtectionProcessUpdated();
			return TRUE;
		}

		return FALSE;
	}

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN);	
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE,				 OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_PROBROWS_ADD,		 OnBkBtnAdd)
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_PROBROWS_DEL,		 OnBkBtnDel)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CSetBrowerPage)	
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MESSAGE_HANDLER(WM_CREATE,				OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,				OnDestroy)	
		MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_HANDLER( IDC_LIST_PROBROWS_DLG, LVN_ITEMCHANGED,	OnListViewItemChange)
		CHAIN_MSG_MAP(CBkDialogImpl<CSetBrowerPage>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CSetBrowerPage>)
	END_MSG_MAP()
private:
	CRect				m_rtFile;
	KRuleListCtrl		m_browers;
	BOOL                m_fSettingChange;
	std::vector<std::wstring> m_process;   /* all protected process */
};