// winoptDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include <vector>
#include <map>
using namespace std;

#define WINDOW_UPDATE_TIMEER		1

// CwinoptDlg 对话框
class CwinoptDlg : public CDialog
{
// 构造
public:
	CwinoptDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WINOPT_DIALOG };

	BOOL	AddEnumWindow(HWND hWnd);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void		UpdateListData();
	void		InitListData();
	void		AddListHeader(CHeaderCtrl* pHeader, INT nPos, INT nWidth, LPTSTR strTxt, INT nFormat);
	void		EnumTopLevelWnd();
	int			AddWndInfoToList(HWND hWnd);
	void		OnDestroyWnd(HWND hWnd);
	void		UpdateWndInfo();
	void		RemoveUnVisibleFromList();
	void		LogWndInfo(HWND hWnd);
	BOOL		IsWindowMustList(HWND hWnd, INT* pID = NULL);
	void		ResetList();
	void		SetActiveWnd(HWND hWnd);
	void		DeleteWndFromList(HWND hWnd);

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

// 实现
protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nID);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_winList;
	afx_msg void OnBnClickedCloseSelWin();

private:
	HICON				m_hIcon;
	DWORD				m_nHookMessage;
	map<HWND, int>		m_TopLevelWinList;
	DWORD				m_nUpdateWndTimer;
	BOOL				m_bLoggingWndInfo;

public:
	afx_msg void OnBnClickedLogWndInfo();
	CString m_ActiveWnd;
	CString m_ActiveProcess;
};
