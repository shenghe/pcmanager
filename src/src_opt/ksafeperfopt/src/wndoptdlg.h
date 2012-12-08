

#pragma once


#include <wtlhelper/whwindow.h>
#include <common/utility.h>
#include "perfmon/kperfmonmem.h"
#include "bklistbox.h"
#include "bkres/bkres.h"
#include "kprocesslib/interface.h"
#include "appwndlistitemdata.h"
#include "common/bksafelog.h"
#include "perfmonutility.h"
#include "iefix/cinifile.h"
#include "kpfw/netflowformat.h"
#include <time.h>
#include "atltime.h"
#include "kmaindlg.h"

#define MSG_START_FIX_ITEM	WM_APP+0x402
#define MSG_END_FIX_ITEM	WM_APP+0x403
#define MSG_FINISH_FIX		WM_APP+0x404
#define TIMER_ID_CHECK_WND			1001

enum{
	ITEM_TYPE_TITLE	= 0,
	ITEM_TYPE_WND	= 1,
	ITEM_TYPE_PRO	= 2,
	ITEM_TYPE_MEM	= 3,
};

enum{
	CHK_ST_UNCHECK	= 0,
	CHK_ST_CHECK	= 1,
	CHK_ST_FIXED	= 2,
	CHK_ST_FIXFALED = 3,
};

enum{
	WND_ST_NORMAL		= 0,
	WND_ST_NORESPONSE	= 1,
	WND_ST_CLOSING		= 2,
	WND_ST_NOUSE		= 3,
};

enum{
	FIX_ST_UNFIX		= 0,
	FIX_ST_UNFREE		= 1,
	FIX_ST_FIXING		= 2,
	FIX_ST_FIXED		= 3,
	FIX_ST_FREED		= 4,
	FIX_ST_FIXFAILED	= 5,
};

BOOL IsDownLoadTitle( CString& strTitle );
BOOL IsKSafeWindow(KAppWndListItemData);

class WndOptItem
{
public:
	WndOptItem()
	{
		m_nType		= 0;
		m_hWnd		= NULL;
		m_hIcon		= NULL;
		m_dwPId		= 0;
		m_nCheckState	= 0;
		m_nWndState		= 0;
		m_nFixState		= 0;
		m_nIndex		= 0;
	}

	int			m_nType;			//0 title, 1 wnd , 2 process 3 memory
	HWND		m_hWnd; 
	HICON		m_hIcon;			
	CString		m_strName;
	CString		m_strFileName;
	DWORD		m_dwPId;
	int			m_nCheckState;		//0 uncheck, 1 check, 2 fixed, 3 fixfailed
	int			m_nWndState;		//0 normal , 1 noresponse, 2 fixing, 3 nouse
	int			m_nFixState;		//0 unfix, 1 unfree, 2 fixing, 3 fixed, 4 freed, 5 fixfaild
	int			m_nIndex;			//wnd list index
};


class CBKSafeWndOptdlg
	: public CBkDialogImpl<CBKSafeWndOptdlg>
	, public CWHRoundRectFrameHelper<CBKSafeWndOptdlg>

{
public:
	CBKSafeWndOptdlg(KMainDlg* pMainDlg)
		:m_nPidCnt(0),
		m_nWndCnt(0),
		m_hFixThread(NULL),
		m_bStopFix(FALSE),
		m_nFixAllCnt(0),
		m_nFixCurIndex(0),
		m_nPrcess(0),
		m_uOptSize(0),
		m_nFixingIndex(0),
		m_nWndGif(0),
		m_pMainDlg(pMainDlg)
	{

	};

	~CBKSafeWndOptdlg()
	{

	};

protected:
		BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
			BK_NOTIFY_ID_COMMAND(102, OnBkClose)
			BK_NOTIFY_ID_COMMAND(IDC_BTN_START_WND_OPT, OnBtnStartWndOpt)
			BK_NOTIFY_ID_COMMAND(IDC_LNK_STOP_WND_OPT,	OnBtnStopWndOpt)
			BK_NOTIFY_ID_COMMAND(IDC_WND_OPT_CHECKALL,	OnCheckAll)
		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBKSafeWndOptdlg)
			MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
			CHAIN_MSG_MAP(CBkDialogImpl<CBKSafeWndOptdlg>)
			CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKSafeWndOptdlg>)
			NOTIFY_HANDLER_EX(IDC_WND_OPT_LIST, BKLBM_GET_DISPINFO, OnBkLvmGetDispInfo)
			NOTIFY_HANDLER_EX(IDC_WND_OPT_LIST, BKLBM_ITEMCLICK,	OnListBoxLClickCtrl)
			MSG_WM_INITDIALOG(OnInitDialog)
			MESSAGE_HANDLER( WM_TIMER, OnTimer )
			MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
			MESSAGE_HANDLER( MSG_START_FIX_ITEM,	OnStartFixItem )
			MESSAGE_HANDLER( MSG_END_FIX_ITEM,		OnEndFixItem )
			MESSAGE_HANDLER( MSG_FINISH_FIX,		OnFinishFix )
			REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

	LRESULT	OnStartFixItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle );
	LRESULT	OnEndFixItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle );
	LRESULT	OnFinishFix(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle );

	LRESULT	OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle );
	LRESULT	OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle );
	BOOL	OnInitDialog(HWND wParam, LPARAM lParam);
	void	OnBkClose();
	void	OnCheckAll();
	void	OnBtnStartWndOpt();
	void	OnBtnStopWndOpt();
	void	OnOk();
	void	UpdateCheckAllState();
	void	InitCtrl();
	BOOL	IsAnyCheck();
	BOOL	IsAllCheck();
	LRESULT OnBkLvmGetDispInfo( LPNMHDR pnmh );
	LRESULT	OnListBoxLClickCtrl(LPNMHDR pnmh);
	void	UpdateWndOptInfo( vector<KAppWndListItemData>& vecListItem );

	static DWORD WINAPI FixWndProc(__in  LPVOID lpParameter);

protected:
	void AddMemoryItem( vector<KAppWndListItemData>& vecListItem );
	void AddWndItem( vector<KAppWndListItemData>& vecListItem );
	void AddProcessItem( vector<KAppWndListItemData>& vecListItem );

	int GetWndCnt( vector<KAppWndListItemData>& vecListItem );
	int GetPidCnt( vector<KAppWndListItemData>& vecListItem );
	int GetAllItemCnt();
	int GetRestUnFixCnt();

	void UpdateInfoText();

	HICON GetWndIcon(KAppWndListItemData& ItemData);
	void DoFixWnd();
	void GetFixArray();
	BOOL CloseOneWnd( WndOptItem& item );
	ULONGLONG FreeMemory( WndOptItem& item );
	BOOL _KillProcess2(DWORD dwPID);
	void _DoCloseWnd( HWND hWnd );
	void FreeMemFinished(ULONGLONG uSize, int nUnFixCnt);

	
private:
	CBkListBox				m_wndOptList;
	CAtlArray<WndOptItem>	m_aryListItem;
	CAtlArray<WndOptItem>	m_aryFixList;

	int						m_nPidCnt;
	int						m_nWndCnt;
	int						m_nFixingIndex;

	int						m_nFixAllCnt;
	int						m_nFixCurIndex;
	double					m_nPrcess;

	HANDLE					m_hFixThread;
	BOOL					m_bStopFix;
	ULONGLONG				m_uOptSize;
	int						m_nWndGif;

	KMainDlg*				m_pMainDlg;
//	CSysPerfOpt	m_sysPerf;
};