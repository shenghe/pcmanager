#pragma  once

#include "TypeListBox.h"
#include "softmgrdownlistview.h"
#include "softmgrdownloadlistview.h"
#include "SoftMgrRubbishListView.h"


#define		DOWNMGR_GETDATA_TIMER		1

class CBeikeSafeSoftmgrUIHandler;		// 父窗口

class CBkDownloadMgrDlg
	: public CBkDialogImpl<CBkDownloadMgrDlg>
	, public CWHRoundRectFrameHelper<CBkDownloadMgrDlg>
	, public IClickCallback
	, public ISoftMgrDownCallback
	, public ISoftMgrDownloadCallback
	, public ISoftMgrRubbishCallback
{
public:
	CBkDownloadMgrDlg(CBeikeSafeSoftmgrUIHandler * pHandler)
		: CBkDialogImpl<CBkDownloadMgrDlg>(IDR_BK_DOWNLOAD_MGR_DLG)
		, m_nPage(0)
		, m_pHandler( pHandler )
	{
		m_nDownloaded = 0;
		m_nDownloading = 0;

	}

	~CBkDownloadMgrDlg()
	{

	}

public:
	void OnClick( int nListId, CTypeListItemData * pData);
	void OnDownSetUp(DWORD dwIndex);
	void OnLbuttonUpNotify(DWORD dwIndex);
	void OnMgrDownLoad(DWORD dwIndex);	//下载
	void OnMgrContinue(DWORD dwIndex);	//继续
	void OnMgrPause(DWORD dwindex);	//暂停
	void OnMgrCancel(DWORD dwIndex);	//取消
	void OnMgrFreebackFromList(DWORD dwIndex);
	void OnMgrTautilogyFromList(DWORD dwIndex);

	void OnLbuttonUpNotifyEx(DWORD dwIndex);
	void OnResume(CString strId);


	void CheckDelBtn();

	UINT_PTR DoModal(int nPage, HWND hWndParent);

public:
	CTypeListBox				m_list;	
	CSoftMgrDownloadListView	m_IngList;		// 正在下载
	CSoftMgrDownListView		m_EdList;		// 已下载
	CSoftMgrRubbishListView		m_RibList;		//垃圾箱

protected:

	int m_nPage;
	CBeikeSafeSoftmgrUIHandler * m_pHandler;


	int		m_nDownloaded;			// 已下载数目
	int		m_nDownloading;			// 正在下载数目



	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnBkBtnClose();


protected:

	void	InitList();
	void	InitTypeList();
	void	InitIngList();
	void	InitEdList();
	void	InitRibList();

	void    InsertDataToList();

	void	OnTimer(UINT_PTR nIDEvent);
	void	GetDataFromHandler();

	void	GetCurDiskFreeSpace( LPCTSTR szCurrentPath, CString & strFreeSpace );
	void	OnOpenStoreDir();
	void	OnDelFiles();
	void	OnSelAll();
	void	OnDelDownloadTask();
	void	OnSelAllListItem();

	void	OnListEdReSize(CRect rcWnd);
	void	OnListIngReSize(CRect rcWnd);
	void	OnListRibReSize(CRect rcWnd);

	CString CalcStrFormatByteSize(LONGLONG qdw);
public:

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)

		// button
		BK_NOTIFY_ID_COMMAND(IDC_DOWNMGR_ING_BTN_DIR, OnOpenStoreDir)
		BK_NOTIFY_ID_COMMAND(IDC_DOWNMGR_ING_LNK_DIR, OnOpenStoreDir)
		BK_NOTIFY_ID_COMMAND(IDC_DOWNMGR_ED_BTN_DIR, OnOpenStoreDir)
		BK_NOTIFY_ID_COMMAND(IDC_DOWNMGR_ED_LNK_DIR, OnOpenStoreDir)
		BK_NOTIFY_ID_COMMAND(IDC_DOWNMGR_ED_BTN_DEL, OnDelFiles)
		BK_NOTIFY_ID_COMMAND(IDC_DOWNMGR_RUBBISH_BTN_DEL, OnDelDownloadTask)
		BK_NOTIFY_ID_COMMAND( IDC_DOWNMGR_ED_CHECK, OnSelAll )
		BK_NOTIFY_ID_COMMAND( IDC_DOWNMGR_RUBBISH_CHECK, OnSelAllListItem )

		BK_NOTIFY_REALWND_RESIZED(IDC_DOWNMGR_ED_RIGHT_LIST, OnListEdReSize)
		BK_NOTIFY_REALWND_RESIZED(IDC_DOWNMGR_ING_RIGHT_LIST, OnListIngReSize)
		BK_NOTIFY_REALWND_RESIZED(IDC_DOWNMGR_RUBBISH_RIGHT_LIST, OnListRibReSize)

		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBkDownloadMgrDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBkDownloadMgrDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkDownloadMgrDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
};

