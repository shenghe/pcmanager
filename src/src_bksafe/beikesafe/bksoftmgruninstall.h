#pragma once

#include <atlcoll.h>
#include <wtlhelper/whwindow.h>
#include <bkres/bkres.h>
#include "beikesafemsgbox.h"
#include <softmgr/ISoftUninstall.h>
#include "SoftMgrUniExpandListView.h"
#include "BkSoftMgrPowerSweepDlg.h"
#include "beikesafe.h"

#define	WM_SOFTMG_BASE						(WM_USER + 0x400)
#define	WM_SOFTMGR_UNINSTALL_BEGIN			(WM_SOFTMG_BASE + 1)
#define	WM_SOFTMGR_UNINSTALL_ANALYSIS		(WM_SOFTMG_BASE + 2)
#define	WM_SOFTMGR_UNINSTALL_INVOKE			(WM_SOFTMG_BASE + 3)
#define	WM_SOFTMGR_UNINSTALL_CHECKREMAINT	(WM_SOFTMG_BASE + 4)
#define	WM_SOFTMGR_UNINSTALL_REMAINT		(WM_SOFTMG_BASE + 5)
#define	WM_SOFTMGR_UNINSTALL_END			(WM_SOFTMG_BASE + 10)
#define	WM_SOFTMGR_UNINSTALL_POWERSWEEP		(WM_SOFTMG_BASE + 20)
#define	WM_SOFTMGR_UNINSTALL_MULTI_IN_DIR	(WM_SOFTMG_BASE + 6)
#define WM_SOFTMGR_UNINSTALL_TOO_MUCH		(WM_SOFTMG_BASE + 7)



class CBeikeSafeSoftmgrUIHandler;
class CBeikeSoftMgrPowerSweepDlg;

class CBkSafeSoftMgrUnstDlg
	: public CBkDialogImpl<CBkSafeSoftMgrUnstDlg>, 
	public CWHRoundRectFrameHelper<CBkSafeSoftMgrUnstDlg>  // 窗口使用圆角边框
{
public:
	CBkSafeSoftMgrUnstDlg(CBeikeSafeSoftmgrUIHandler* pHandler, ksm::ISoftUninstall2* pUnistaller, LPCTSTR strCaption, LPCTSTR strkey ) 
		: CBkDialogImpl<CBkSafeSoftMgrUnstDlg>(IDR_BK_SOFTMGR_UNINSTALL), 
		m_bSweeping(false), 
		m_nAnimateID(0),
		m_nLoopValue(0), 
		m_nRemaintNum(0), 
		m_hSoftWnd(NULL), 
		m_bShowSoftWnd(false),
		m_pSoftUnstall(pUnistaller),
		m_strSoftKey(strkey),
		m_pPowerSweepdlg(NULL),
		m_bSoftUnistOK(FALSE),
		m_strCaption(strCaption),
		m_bMulSoftInDir(FALSE),
		m_pHandler(pHandler),
		m_pdataSweep(NULL),
		m_bTooMuchFile(FALSE),
		m_ullSize(0)
	{
	}

	~CBkSafeSoftMgrUnstDlg()
	{
		if ( m_pdataSweep )
		{
			m_pdataSweep->Uninitialize();
			m_pdataSweep = NULL;
		}


		_ClearData();
	}

protected:
	BOOL		m_bInit;
	BOOL		m_bShow;
	HWND		m_hSoftWnd;			// 卸载程序的窗口
	bool		m_bShowSoftWnd;		// 显示卸载程序的窗口
	int			m_nRemaintNum;		// 残留项数
	int			m_nLoopValue;		// 循环值
	int			m_nAnimateID;		// 指定要使用动画的控件的ID
	HANDLE		m_hSweepEvent;		// 同步事件
	bool		m_bSweeping;			// 用户是否选择强力清扫
	BOOL		m_bSweepAll;		
	BOOL		m_bCancel;
	BOOL		m_bMulSoftInDir;	// 是否多款软件装在一个目录
	BOOL		m_bTooMuchFile;		// 太多的残留文件
	ULONGLONG	m_ullSize;			// 残留的大小
	ksm::ISoftUninstall2*	m_pSoftUnstall;
	ksm::ISoftDataSweep2*	m_pdataSweep;
	CString		m_strSoftKey;
	CString		m_strCaption;
	CBeikeSoftMgrPowerSweepDlg*	m_pPowerSweepdlg;
	BOOL		m_bSoftUnistOK;		// 是否完全卸载了
	CString		m_strMainDir;
	CBeikeSafeSoftmgrUIHandler* m_pHandler;

	CSimpleArray<SOFT_RUB_INFO*>		m_arrSoftRubData;		//卸载软件的残留项目列表

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_SOFTMGR_UNIN_CLOSE, OnClose)
		BK_NOTIFY_ID_COMMAND(IDC_SOFTMGR_UNIN_FINISH, OnClose)
		BK_NOTIFY_ID_COMMAND(IDC_SOFTMGR_UNIN_SWEEP, OnSweep)
		BK_NOTIFY_ID_COMMAND(IDC_SOFTMGR_UNIN_OPEN_INST_DIR, OnOpenMainDir)
		BK_NOTIFY_ID_COMMAND(IDC_SOFTMGR_UNIN_OPEN_INST_DIR2, OnOpenMainDir)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBkSafeSoftMgrUnstDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)

		CHAIN_MSG_MAP(CBkDialogImpl<CBkSafeSoftMgrUnstDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkSafeSoftMgrUnstDlg>)
		
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_BEGIN, OnBegin)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_BEGIN, OnEnd)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_ANALYSIS, OnAnalysis)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_INVOKE, OnInvokeUninstall)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_CHECKREMAINT, OnCheckRemaint)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_REMAINT, OnGetRemaint)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_POWERSWEEP, OnPowerSweep)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_END, OnUnistallEnd)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_MULTI_IN_DIR, OnMultiSoftInDir)
		MESSAGE_HANDLER_EX(WM_SOFTMGR_UNINSTALL_TOO_MUCH, OnTooMuchFileInDir)

		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

public:

	// 是否软件已经完全卸载掉
	BOOL IsSoftUnistComplete()
	{
		return m_bSoftUnistOK;
	}

	// 卸载，软件卸载事件
	void SoftDataSweepEvent(ksm::UninstEvent event, ksm::ISoftDataSweep *pSweep)
	{
		if ( !::IsWindow(m_hWnd) )
			return;

		if ( m_pPowerSweepdlg != NULL )
			m_pPowerSweepdlg->SoftDataSweepEvent(event, pSweep);

		switch(event)
		{
		case ksm::UE_Sweep_Begin:
			break;

		case ksm::UE_Invoke_Uninst://通知卸载窗第一步分析完毕
			SendMessage(WM_SOFTMGR_UNINSTALL_ANALYSIS,NULL,NULL);
			SendMessage(WM_SOFTMGR_UNINSTALL_INVOKE,(WPARAM)TRUE,NULL);
			break;

		//通知卸载窗第二步标准卸载完毕
		case ksm::UE_Invoke_Failed:
			SendMessage(WM_SOFTMGR_UNINSTALL_INVOKE,(WPARAM)FALSE,(LPARAM)0);
			SendMessage(WM_SOFTMGR_UNINSTALL_CHECKREMAINT,(WPARAM)TRUE,NULL);
			break;
		case ksm::UE_Invoke_Succeeded://通知卸载窗第二步标准卸载完毕
			SendMessage(WM_SOFTMGR_UNINSTALL_INVOKE,(WPARAM)FALSE,(LPARAM)1 );
			SendMessage(WM_SOFTMGR_UNINSTALL_CHECKREMAINT,(WPARAM)TRUE,NULL);
			break;
		case ksm::UE_Invoke_Canceled:
			SendMessage(WM_SOFTMGR_UNINSTALL_INVOKE,(WPARAM)FALSE,(LPARAM)2 );
			SendMessage(WM_SOFTMGR_UNINSTALL_CHECKREMAINT,(WPARAM)TRUE,NULL);
			break;

		case ksm::UE_Enum_End://情况B, 枚举残留完毕，//通知卸载窗第三步残留分析完毕
			{
				_ClearData();

				ksm::ISoftRubbishEnum* pRub = pSweep->DataUninstEnum();
				ksm::SoftRubbish sr;
				BOOL bRetCode = pRub->EnumFirst(&sr);
				while (bRetCode)
				{
					SOFT_RUB_INFO *pRubInfo = new SOFT_RUB_INFO;
					pRubInfo->nType = sr._type;
					pRubInfo->strData = sr._pData;

					m_arrSoftRubData.Add(pRubInfo);

					bRetCode = pRub->EnumNext(&sr);
				}

				m_nRemaintNum = m_arrSoftRubData.GetSize();
				SendMessage(WM_SOFTMGR_UNINSTALL_CHECKREMAINT,(WPARAM)FALSE,NULL);			
			}
			break;
		case ksm::UE_Enum_Too_Much:
			SendMessage(WM_SOFTMGR_UNINSTALL_TOO_MUCH,NULL,NULL);
		case ksm::UE_Enum_Multi:
			SendMessage(WM_SOFTMGR_UNINSTALL_MULTI_IN_DIR,NULL,NULL);
			break;
		case ksm::UE_Sweeping:
			break;
		case ksm::UE_Sweep_End:
			PostMessage(WM_SOFTMGR_UNINSTALL_END,0,0);
			break;
		}
	}


	VOID _ClearData()
	{
		for (int i=0; i < m_arrSoftRubData.GetSize(); i++)
		{
			SOFT_RUB_INFO* pRubInfo = (SOFT_RUB_INFO*)m_arrSoftRubData[i];
			if (pRubInfo)
			{
				delete pRubInfo;
				pRubInfo = NULL;
			}
		}
		m_arrSoftRubData.RemoveAll();
		m_nRemaintNum = 0;
	}

private:
	void	OnSetFocus(HWND wndOld);
	void	OnKillFocus(HWND wndFocus);
	LRESULT	OnBegin(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnEnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnAnalysis(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnInvokeUninstall(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnCheckRemaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnGetRemaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnPowerSweep(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnUnistallEnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnMultiSoftInDir(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT	OnTooMuchFileInDir(UINT uMsg, WPARAM wParam, LPARAM lParam);		// 残留文件过多

protected:
	void OnTimer(UINT_PTR nIdEvent);
	void OnClose();
	void OnSweep();
	void OnOpenMainDir();

protected:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
};
