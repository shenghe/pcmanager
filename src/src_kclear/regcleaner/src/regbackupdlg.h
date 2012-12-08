#ifndef _REGBACKUPDLG_H_
#define _REGBACKUPDLG_H_

#include <wtlhelper/whwindow.h>
#include "kuires.h"
#include "resource.h"
#pragma warning(disable : 4244 4311 4312 4267)
//#include "kbitmapbutton.h"
//#include "atlbuttenstatc.h"
//#include "bitmapstatic.h"
#include "listctrlex.h"
//#include "skinbutton.h"
#include "hlhovercr.h"
#pragma warning(default : 4244 4311 4312 4267)
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <set>
#include "kbackupreg.h"
using namespace std;


typedef CListCtrlDataEx<wstring> CListCtrlExt;

class CRegBackupDlg 
	: public CKuiDialogImpl<CRegBackupDlg>
	, public CWHRoundRectFrameHelper<CRegBackupDlg>
{
public:
	CRegBackupDlg(void) :
	CKuiDialogImpl<CRegBackupDlg>("IDR_DLG_REG_BACKUP")
	{
		m_nWidth = 0;
		m_nHeight = 0;
		m_strSelctFolder = L"";
		m_bParentKsafe = FALSE;
	}

	~CRegBackupDlg(void)
	{
	}

public:
	KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
	KUI_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnClose)
	KUI_NOTIFY_ID_COMMAND(IDOK, OnClose)
	KUI_NOTIFY_ID_COMMAND(2, OnRestore)
	KUI_NOTIFY_ID_COMMAND(3, OnDelete)
	KUI_NOTIFY_ID_COMMAND(4, OnDeleteAll)
	KUI_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CRegBackupDlg)
	MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
	CHAIN_MSG_MAP(CKuiDialogImpl<CRegBackupDlg>)
	CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CRegBackupDlg>)
	MSG_WM_INITDIALOG(OnInitDialog)
	MSG_WM_LBUTTONDOWN(OnLButtonDown)
	NOTIFY_HANDLER(4420, LCN_LEFTCLICK, OnListSelect)
	REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	void OnClose();
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnPaint(HDC hDc);
	void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnCtlColorStatic(HDC hDC, HWND hWnd);

	BOOL CreateBackpFile();

//	BOOL InitControlBtn();

	BOOL InitBackupList();

	BOOL RestoreRegFile(wstring strFolder);

	DWORD KscCreateProcessUser(wchar_t* lpApplicationName, wchar_t* lpCommandLine);

	BOOL KscFindBackupFile(wstring strFile);

	BOOL KscFindBackupFolder();

	BOOL _DoInsertItem();

	LRESULT	OnDelete(/*WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled*/);
	LRESULT	OnDeleteAll(/*WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled*/);
	LRESULT	OnRestore(/*WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled*/);
	
	LRESULT	OnClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
	LRESULT OnListSelect(int idCtrl, LPNMHDR pnmh, BOOL &bHandled);

	void	SetCtrlStatus();

	wstring GetSysDateString();

	void OnLButtonDown(UINT nFlags, CPoint point);

	BOOL GenBackupFileName();

	BOOL RegNameInvert(wstring strIn, wstring& strOut);

	BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

	int UpperMyPrivilege();

	BOOL KCreateLongDir(
		LPCTSTR lpPathName, 
		LPSECURITY_ATTRIBUTES lpSecurityAttributes
		);

	BOOL GetFileInfo(wstring srtFileFolder, int& nFileCount, int& nFileSize);

	BOOL DeleteBackupFolder(wstring strFolder);
	void GetDialogSize(); 

//	LRESULT OnLButtonDownList(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	BOOL Init();
	BOOL Unit();
 
 	bool InTitleZone(CPoint& pt) 
 	{
 		if (pt.y <= 24)
 			return true;
 		else
 			return false;
 	}

private:
	wstring					m_strBackupPath;
	CListCtrlExt			m_listBackup;

	vector<wstring>			m_vsBackup;
	MSSGS					m_vsBackupFile;
	MSSGS					m_vsBackupTemp;

	wstring					m_strTempFolder;
	wstring					m_strBackupFileName;
	wstring					m_strSelctFolder;

//	set<wstring>m_sBackupKey;

	int m_nWidth;
	int m_nHeight;
	BOOL m_bParentKsafe;
};

#endif
