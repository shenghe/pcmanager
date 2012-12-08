// winoptDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "winopt.h"
#include "winoptDlg.h"
#include "Psapi.h"
#include "../../../src_kws/include//common/msg_logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Psapi.lib")

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CwinoptDlg 对话框




CwinoptDlg::CwinoptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CwinoptDlg::IDD, pParent), m_nHookMessage(0)
	, m_nUpdateWndTimer(-1), m_bLoggingWndInfo(FALSE)
	, m_ActiveWnd(_T(""))
	, m_ActiveProcess(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CwinoptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WIN_LIIST, m_winList);
	DDX_Text(pDX, IDC_ACTIVEWIND, m_ActiveWnd);
	DDX_Text(pDX, IDC_ACTIVE_PROCESS, m_ActiveProcess);
}

BEGIN_MESSAGE_MAP(CwinoptDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CLOSE_SEL_WIN, &CwinoptDlg::OnBnClickedCloseSelWin)
	ON_BN_CLICKED(IDC_LOG_WND_INFO, &CwinoptDlg::OnBnClickedLogWndInfo)
END_MESSAGE_MAP()


// CwinoptDlg 消息处理程序

BOOL CwinoptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_nHookMessage = RegisterWindowMessage(TEXT("SHELLHOOK"));
	if (!RegisterShellHookWindow(GetSafeHwnd()))
	{
		::MessageBox(NULL, TEXT("test"), TEXT("test"), MB_OK);
	}

	InitListData();
	EnumTopLevelWnd();
	UpdateListData();

	// 设置timer
	m_nUpdateWndTimer = SetTimer(WINDOW_UPDATE_TIMEER, 1000, NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CwinoptDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CwinoptDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CwinoptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CwinoptDlg::OnBnClickedCloseSelWin()
{
	// TODO: 在此添加控件通知处理程序代码

	POSITION pos = m_winList.GetFirstSelectedItemPosition();
	if (pos == NULL)
		TRACE0("No items were selected!\n");
	else
	{
		while (pos)
		{
			int nItem = m_winList.GetNextSelectedItem(pos);
			DWORD_PTR nData = m_winList.GetItemData(nItem);
			HWND hWnd = (HWND)nData;
			::SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
}

LRESULT CwinoptDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == m_nHookMessage)
	{
		switch (wParam)
		{
		case HSHELL_WINDOWACTIVATED:
			SetActiveWnd((HWND)lParam);
			break;
		}
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

BOOL CwinoptDlg::IsWindowMustList(HWND hWnd, INT* pID)
{
	if (!::IsWindowVisible(hWnd))
	{
		if (pID) *pID = 1;
		return FALSE;
	}

	RECT rct = {0};
	::GetWindowRect(hWnd, &rct);
	if ((rct.left == rct.right))
	{
		if (pID) *pID = 2;
		return FALSE;
	}

	DWORD nExStyle = ((DWORD)GetWindowLong(hWnd, GWL_EXSTYLE));
	DWORD nStyle = ((DWORD)GetWindowLong(hWnd, GWL_STYLE));

	if (nExStyle & WS_EX_TOOLWINDOW)
	{
		if (pID) *pID = 3;
		return FALSE;
	}

	if (nExStyle & WS_EX_APPWINDOW)
	{
		if (pID) *pID = 4;
		return TRUE;
	}
	/*
	if (nStyle & WS_CAPTION)
	{
		if (pID) *pID = 5;
		return TRUE;
	}*/
	if (nStyle & WS_SYSMENU)
	{
		if (pID) *pID = 6;
		return TRUE;
	}
	if (pID) *pID = 7;
	return FALSE;
}

BOOL CwinoptDlg::AddEnumWindow(HWND hWnd)
{
	LogWndInfo(hWnd);

	if (IsWindowMustList(hWnd))
	{
		if (m_TopLevelWinList.find(hWnd) == m_TopLevelWinList.end())
		{
			int nItem = AddWndInfoToList(hWnd);
			m_TopLevelWinList[hWnd] = nItem;
		}
	}
	return TRUE;
}

void CwinoptDlg::SetActiveWnd(HWND hWnd)
{
	TCHAR szWndTitle[MAX_PATH] = {0};
	::GetWindowText((HWND)hWnd, szWndTitle, MAX_PATH);
	m_ActiveWnd = szWndTitle;

	DWORD nProcessID = 0;
	GetWindowThreadProcessId(hWnd, &nProcessID);

	TCHAR szProcPath[MAX_PATH] = {0};
	HANDLE hHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, nProcessID);
	if (hHandle)
	{
		::GetModuleFileNameEx(hHandle, NULL, szProcPath, MAX_PATH);
		::CloseHandle(hHandle);
		hHandle = NULL;
	}
	m_ActiveProcess = szProcPath; 
	UpdateData(FALSE);
}

void CwinoptDlg::UpdateListData()
{
	/*
	vector<HWND>::iterator it = m_TopLevelWinList.begin();
	for(; it != m_TopLevelWinList.end(); it++)
	{
		HWND hWnd = *it;
		AddWndInfoToList(hWnd);
	}*/
}

int CwinoptDlg::AddWndInfoToList(HWND hWnd)
{
	TCHAR szWndTitle[MAX_PATH] = {0};
	::GetWindowText((HWND)hWnd, szWndTitle, MAX_PATH);

	RECT rct = {0};
	::GetWindowRect(hWnd, &rct);

	DWORD nStyle = ((DWORD)GetWindowLong(hWnd, GWL_STYLE));
	DWORD nExStyle = ((DWORD)GetWindowLong(hWnd, GWL_EXSTYLE));

	DWORD nProcessID = 0;
	GetWindowThreadProcessId(hWnd, &nProcessID);
	
	TCHAR szProcPath[MAX_PATH] = {0};
	HANDLE hHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, nProcessID);
	if (hHandle)
	{
		::GetModuleFileNameEx(hHandle, NULL, szProcPath, MAX_PATH);
		::CloseHandle(hHandle);
		hHandle = NULL;
	}

	int nItem = m_winList.InsertItem(m_winList.GetItemCount(), szWndTitle);
	m_winList.SetItemText(nItem, 1, szProcPath);
	m_winList.SetItemData(nItem, (DWORD_PTR)hWnd);
	return nItem;
}

void CwinoptDlg::InitListData()
{
	m_winList.InsertColumn(0, TEXT("窗口名"), HDF_LEFT, 200);
	m_winList.InsertColumn(1, TEXT("程序名"), HDF_LEFT, 400);
}

void CwinoptDlg::AddListHeader(CHeaderCtrl* pHeader, INT nPos, INT nWidth, LPTSTR strTxt, INT nFormat)
{
	HDITEMW hdItem =  {0};
	hdItem.mask = HDI_FORMAT | HDI_WIDTH | HDI_TEXT;
	hdItem.fmt = nFormat;
	hdItem.cxy = nWidth;
	hdItem.pszText = strTxt;
	pHeader->InsertItem(nPos, &hdItem);
}

void CwinoptDlg::EnumTopLevelWnd()
{
	::EnumWindows(&CwinoptDlg::EnumWindowsProc, (LPARAM)this);
}

BOOL CwinoptDlg::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	CwinoptDlg* pThis = (CwinoptDlg*)lParam;
	pThis->AddEnumWindow(hwnd);
	return TRUE;
}

void CwinoptDlg::OnTimer(UINT_PTR nID)
{
	if (nID == m_nUpdateWndTimer)
	{
		RemoveUnVisibleFromList();
		EnumTopLevelWnd();
		if (m_TopLevelWinList.size() != m_winList.GetItemCount())
		{
			ResetList();
		}
	}
}

void CwinoptDlg::ResetList()
{
	while (m_winList.GetItemCount() != 0)
		m_winList.DeleteItem(0);
	map<HWND, int>::iterator it = m_TopLevelWinList.begin();
	for (; it != m_TopLevelWinList.end(); it++)
		m_TopLevelWinList[it->first] = AddWndInfoToList(it->first);
}

void CwinoptDlg::RemoveUnVisibleFromList()
{
	map<HWND, int>::iterator it = m_TopLevelWinList.begin();
	for (; it != m_TopLevelWinList.end(); )
	{
		HWND hWnd = it->first;
		int nID = it->second;
		if (!::IsWindow(hWnd) || !IsWindowMustList(hWnd))
		{
			map<HWND, int>::iterator it_cur = it++;
			m_TopLevelWinList.erase(it_cur);
			DeleteWndFromList(hWnd);
		}
		else
		{
			it++;
		}
	}
}
void CwinoptDlg::DeleteWndFromList(HWND hWnd)
{
	for (int i = 0; i < m_winList.GetItemCount(); i++)
	{
		if ((HWND)m_winList.GetItemData(i) == hWnd)
		{
			m_winList.DeleteItem(i);
			break;
		}
	}
}
void CwinoptDlg::UpdateWndInfo()
{
	// todo: 1:重新枚举当前所有的窗口
	//	2: 对不在当前窗口列表中的窗口进行：
	//	3： 从库中匹配，如果是需要屏蔽的，直接屏蔽
	//	4：对不需要屏蔽的，获取窗口特定的设置
	//  5：将窗口添加到列表中
	//  6：删除当前已经不存在的窗口，如果存在，更新是否可见信息，窗口位置，父窗口等
	//	  7：更新可见信息时，根据窗口的设置，决定是否设置其active属性
	//  8：对需要显示在列表中的窗口，添加到显示列表中
	//  9：对不需要再显示在列表中的，从列表中删除
	//  10: 对需要更新信息的窗口，更新显示信息

	// 对于需要屏蔽的窗口，加入的屏蔽列表中，每次更新，将不在使用的窗口
	// 从屏蔽列表中删除
}

void CwinoptDlg::OnDestroy()
{
	KillTimer(m_nUpdateWndTimer);
	m_nUpdateWndTimer = -1;
}

void CwinoptDlg::LogWndInfo(HWND hWnd)
{
	if (m_bLoggingWndInfo)
	{
		if (!::IsWindowVisible(hWnd))
			return ;
		
		kws_log(TEXT("window: %08x"), hWnd);
		kws_log(TEXT("visible: %d"), ::IsWindowVisible(hWnd));
		INT nID = 0;
		kws_log(TEXT("should show: %d"), IsWindowMustList(hWnd, &nID));
		kws_log(TEXT("should show rule: %d"), nID);

		TCHAR szWndTitle[MAX_PATH] = {0};
		::GetWindowText((HWND)hWnd, szWndTitle, MAX_PATH);
		kws_log(TEXT("window text: %s"), szWndTitle);
		
		DWORD nStyle = ((DWORD)GetWindowLong(hWnd, GWL_STYLE));
		kws_log(TEXT("window style: %08x"), nStyle);

		DWORD nExStyle = ((DWORD)GetWindowLong(hWnd, GWL_EXSTYLE));
		kws_log(TEXT("window ext style: %08x"), nExStyle);

		RECT rct = {0};
		::GetWindowRect(hWnd, &rct);
		kws_log(TEXT("window rct: %d, %d, %d, %d"), rct.left, rct.top, rct.right, rct.bottom);

		DWORD nProcessID = 0;
		GetWindowThreadProcessId(hWnd, &nProcessID);

		TCHAR szProcPath[MAX_PATH] = {0};
		HANDLE hHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, nProcessID);
		if (hHandle)
		{
			::GetModuleFileNameEx(hHandle, NULL, szProcPath, MAX_PATH);
			::CloseHandle(hHandle);
			hHandle = NULL;
		}
		kws_log(TEXT("window process: %s"), szProcPath);

		kws_log(TEXT("-----------------------------"));
	}
}	

void CwinoptDlg::OnBnClickedLogWndInfo()
{
	// TODO: 在此添加控件通知处理程序代码
	kws_log_forcelog();
	m_bLoggingWndInfo = TRUE;
	EnumTopLevelWnd();
	m_bLoggingWndInfo = FALSE;
}
