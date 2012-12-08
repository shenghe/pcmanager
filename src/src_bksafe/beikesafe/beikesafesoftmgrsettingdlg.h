#pragma  once

#include "TypeListBox.h"

// tab
enum {
	SettingPageDownload = 0,
	SettingPageInstall
};

class CBeikeSafeMainDlg;		// 父窗口

class CBeikeSafeSoftMgrSettingDlg
	: public CBkDialogImpl<CBeikeSafeSoftMgrSettingDlg>
	, public CWHRoundRectFrameHelper<CBeikeSafeSoftMgrSettingDlg>
	, public IClickCallback
{
public:
	CBeikeSafeSoftMgrSettingDlg()
		: CBkDialogImpl<CBeikeSafeSoftMgrSettingDlg>(IDR_BK_SOFTMGR_SETTING_DLG)
		, m_nPage(SettingPageDownload)
		, m_bBkSafeSettingChanged(FALSE)
	{
	}

	~CBeikeSafeSoftMgrSettingDlg();

	void	OnClick( int nListId, CTypeListItemData * pData);

	UINT_PTR DoModal(int nPage, HWND hWndParent);
	BOOL BkSafeSoftMgrSettingChanged();
	
	CEdit			m_ctlEditStorePath;		// 保存安装文件的目录
	CTypeListBox	m_list;	

protected:

	int m_nPage;

	BOOL m_bBkSafeSettingChanged;

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnBkBtnOK();
	void OnBkBtnClose();
 
	

protected:

	void	LoadSettings();
	void	SaveSettings();
	
	void	InitList();

	void	GetCurDiskFreeSpace( LPCTSTR szCurrentPath, CString & strFreeSpace );;

	void	OnOpenStoreDir();
	void	OnSelDir();
	void	OnBkSafeSoftMgrSettingChanged();
	void	OnRestoreDefaultDir();

public:

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDOK, OnBkBtnOK)
		BK_NOTIFY_ID_COMMAND(IDCANCEL, OnBkBtnClose)

		// button
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_OPEN_DIR, OnOpenStoreDir)
		BK_NOTIFY_ID_COMMAND( IDC_IMGBTN_SELECT_DIR, OnSelDir )
		BK_NOTIFY_ID_COMMAND( IDC_LINKTEXT_DEFAULT_DIR, OnRestoreDefaultDir )

		// radio
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_SHOWHINT, OnBkSafeSoftMgrSettingChanged )
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_NO_HINT, OnBkSafeSoftMgrSettingChanged )

		BK_NOTIFY_ID_COMMAND( IDC_CHECK_AUTO_INSTALL, OnBkSafeSoftMgrSettingChanged )
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_NO_AUTO_INSTALL, OnBkSafeSoftMgrSettingChanged )

		BK_NOTIFY_ID_COMMAND( IDC_CHECK_DEL_WEEK, OnBkSafeSoftMgrSettingChanged )
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_DEL_RIGHTNOW, OnBkSafeSoftMgrSettingChanged )
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_DEL_NEVER, OnBkSafeSoftMgrSettingChanged )
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_DEL_TO_RECY, OnBkSafeSoftMgrSettingChanged )
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_DEL_REAL, OnBkSafeSoftMgrSettingChanged )

		BK_NOTIFY_ID_COMMAND( IDC_CHECK_DAY_POINT_OUT, OnBkSafeSoftMgrSettingChanged )
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_WEEK_POINT_OUT, OnBkSafeSoftMgrSettingChanged )
		BK_NOTIFY_ID_COMMAND( IDC_CHECK_NEVER_POINT_OUT, OnBkSafeSoftMgrSettingChanged )
	BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBeikeSafeSoftMgrSettingDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeSoftMgrSettingDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeSoftMgrSettingDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
};

class CBeikeSafeSoftMgrSettingNavigator
	: public CBkNavigator
{
public:
	CBeikeSafeSoftMgrSettingNavigator(CBeikeSafeMainDlg *pDialog)
		: m_pDlg(pDialog)
	{

	}

	CBkNavigator* OnNavigate(CString &strChildName);
	UINT_PTR DoModal(int nPage = SettingPageDownload, HWND hWndParent = ::GetActiveWindow());

protected:

	CBeikeSafeMainDlg *m_pDlg;
};
