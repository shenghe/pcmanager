#include "stdafx.h"
#include <winbase.h>
#include "regbackupdlg.h"
#include <shlwapi.h>
// #include <time.h>
#include "backupdef.h"
#include "kuimsgbox.h"
#pragma comment(lib, "shlwapi.lib")

////#define BACKUP_FILENAME L"kingsoft\\ksc\\regbackup"
//#define BACKUP_FILENAME L"regbackup"

BOOL CRegBackupDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	HWND	hwndParent = GetParent();
	if ( ::IsWindow(hwndParent) )
	{
		TCHAR	szClass[40] = {};

		::GetClassName(hwndParent, szClass, 40);
		if ( ::_tcsnicmp(szClass, TEXT("KscTool"), 7) == 0 )
			m_bParentKsafe = true;
	}

	SetWindowTextW(L"恢复注册表");
	CenterWindow(hwndParent);
	
//	CreateBackpFile();

//	InitControlBtn();
	InitBackupList();
	Init();
	KscFindBackupFolder();
	_DoInsertItem();

	SetCtrlStatus();

	return TRUE;
}


// LRESULT CRegBackupDlg::OnCtlColorStatic(HDC hDC, HWND hWnd)
// {
// 	HBRUSH		hbr = NULL;
// 	CRect		rc;
// 
// 	::GetClientRect(hWnd, rc);
// 	hbr = ::CreateSolidBrush(RGB(247, 251, 255));
// 	::FillRect(hDC, rc, hbr);
// 	::DeleteObject(hbr);
// 
// 	return (LRESULT)GetStockObject(NULL_BRUSH);
// }


void CRegBackupDlg::OnClose()
{
	EndDialog(0);
}

BOOL CRegBackupDlg::CreateBackpFile()
{
	WCHAR szFilePath[MAX_PATH] = {0};
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BOOL bRet = FALSE;

	::GetModuleFileName(NULL, szFilePath, MAX_PATH - 1);
	::PathRemoveFileSpec(szFilePath);

//	::SHGetSpecialFolderPath(NULL, szFilePath, CSIDL_LOCAL_APPDATA, FALSE);
	::PathAppend(szFilePath, BACKUPDBNAME);
	
	if(::GetFileAttributes(szFilePath) == INVALID_FILE_ATTRIBUTES)
	{
		bRet = KCreateLongDir(szFilePath, NULL);	
		if (!bRet)
		{
			DWORD dwErr = GetLastError();
		}
		SetFileAttributes(szFilePath,FILE_ATTRIBUTE_HIDDEN);
	}
	m_strBackupPath = szFilePath;
	return bRet;
}


BOOL CRegBackupDlg::RestoreRegFile(wstring strFolder)
{
	BOOL bRet = FALSE;
	wstring sz = L"/s ";
	wstring strTemp;
	vector<wstring>::iterator it;
	WCHAR strReg[1024] = {0};
	
	for(it = m_vsBackup.begin(); it != m_vsBackup.end(); it++)
	{
		strTemp = sz;
		strTemp += L"\"";
		strTemp += m_strBackupPath;
		strTemp += L"\\";
		strTemp += strFolder;
		strTemp += L"\\";
		strTemp += *it;
		strTemp += L"\"";
		::wcscpy(strReg, strTemp.c_str());
	
		KscCreateProcessUser(L"regedit", strReg);
		strTemp = L"";
		ZeroMemory(strReg, MAX_PATH);

	}

//	m_vsBackup.clear();

	return bRet;
}


DWORD CRegBackupDlg::KscCreateProcessUser(wchar_t* lpApplicationName, wchar_t* lpCommandLine)
{
	BOOL bRet = FALSE;
	DWORD dwErr = -1;

	TCHAR szCommandLine[MAX_PATH * 4] = {0};
	if (lpApplicationName)
	{
		wcscpy_s(szCommandLine, (MAX_PATH * 4 -1), lpApplicationName);
	}
	if (lpCommandLine)
	{
		wcscat_s(szCommandLine, (MAX_PATH * 4 -1), L" ");
		wcscat_s(szCommandLine, (MAX_PATH * 4 -1), lpCommandLine);
	}

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi= {0};

	bRet = ::CreateProcess(
		NULL, szCommandLine, 
		NULL, NULL, TRUE, 0,
		NULL, NULL, 
		&si, &pi);
	if (bRet)
	{
		WaitForSingleObject(pi.hProcess, INFINITE); 
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}

	return dwErr;
}


BOOL CRegBackupDlg::KscFindBackupFile(wstring strFile)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	wstring strTemp;
	WIN32_FIND_DATA wfd = {0};
	WCHAR szTargetFile[MAX_PATH] = {0};
	strTemp = m_strBackupPath;
	strTemp += L"\\";
	strTemp += strFile;
	strTemp += L"\\*.reg";

	hFile = FindFirstFile(strTemp.c_str(), &wfd);
	do
	{
		if(hFile != INVALID_HANDLE_VALUE)
		{
			m_vsBackup.push_back(wfd.cFileName);
		}

	}while(FindNextFile(hFile, &wfd));
	FindClose(hFile);

	return TRUE;
}


BOOL CRegBackupDlg::KscFindBackupFolder()
{
	HANDLE			hFile = INVALID_HANDLE_VALUE;
	wstring			strTemp;
	WIN32_FIND_DATA	wfd = {0};

	BOOL			bRet = FALSE;
	int				nFileSize = 0;
	WCHAR			szBuff[MAX_PATH] = {0};
	WCHAR			szDate[MAX_PATH] = {0};
	CKBackupReg		backup;
//	SYSTEMTIME sysTime;

	strTemp = m_strBackupPath;
	strTemp += L"\\*.dat";

	bRet = backup.KscGetBackupInfo(m_vsBackupTemp, m_vsBackupFile);

	//hFile = FindFirstFile(strTemp.c_str(), &wfd);
	//do
	//{
	//	if(hFile != INVALID_HANDLE_VALUE && /*(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&*/ 
	//		(wcsicmp(wfd.cFileName, L".") != 0) && (wcsicmp(wfd.cFileName, L"..") != 0))
	//	{
	//		bRet = backup.KscGetBackupInfo(m_vsBackupFile, m_vsBackupTemp);
	//		FileTimeToSystemTime(&(wfd.ftLastWriteTime), &sysTime);
	//		swprintf_s(szDate, L"%u年%02u月%02u日 %02u时%02u分%02u秒", sysTime.wYear, sysTime.wMonth, 
	//			sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	//		nFileSize = wfd.nFileSizeLow;
	//		swprintf(szBuff, L"这个备份文件共有 %d 个文件，总大小为 %d ", nCount, nFileSize);
	//		m_vsBackupFile.insert(pair<wstring, wstring>(szDate, szBuff));
	//		m_vsBackupTemp.insert(pair<wstring, wstring>(szDate, wfd.cFileName));

	//		nCount = 0;
	//		nFileSize = 0;
	//		ZeroMemory(szBuff, MAX_PATH);
	//		ZeroMemory(szDate, MAX_PATH);
	//	}

	//}while(FindNextFile(hFile, &wfd));
	//FindClose(hFile);

	return TRUE;
}


BOOL CRegBackupDlg::_DoInsertItem()
{
	BOOL				bRet = FALSE;
	int					nSubItem = 0;
	std::wstring		strTimeTips;
	SYSTEMTIME			sysTime = {0};
	WCHAR				szDate[MAX_PATH] = {0};
	int					nLen = 0;
	MSSGS::iterator		it;

	::GetLocalTime(&sysTime);
	swprintf_s(szDate, MAX_PATH, L"%u年%02u月%02u日 ", sysTime.wYear, sysTime.wMonth, sysTime.wDay);
	nLen = (int)::_tcslen(szDate);

	for(it = m_vsBackupFile.begin(); it != m_vsBackupFile.end(); it++)
	{
		if ( ::_tcsnicmp(it->first.c_str(), szDate, nLen) == 0 )
		{
			strTimeTips = TEXT("今天 ");
			strTimeTips += it->first.c_str() + nLen;
		}
		else
		{
			strTimeTips = it->first;
		}

		nSubItem = m_listBackup.AddItem(strTimeTips.c_str(), ITEM_IMAGE_NONE, ITEM_FORMAT_NONE);
		m_listBackup.SetItemText(nSubItem, 1, it->second.c_str());
		m_listBackup.SetItemData(nSubItem, it->first.c_str());
	}
	return TRUE;
}

LRESULT	CRegBackupDlg::OnRestore(/*WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled*/)
{
	int nIndex = -1;
	UINT_PTR nRet;

	CKBackupReg backup;
	if (m_strSelctFolder == L"")
	{
		CKuiMsgBox msg;
		msg.ShowMsg(L"请您选择需要恢复的注册表项！", L"提示", MB_OK | MB_ICONINFORMATION);
		//MessageBox(L"请您选择需要恢复的注册表项！", L"提示", MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}

	wstring strSlectFolder = m_strSelctFolder;
//	wstring strTempPath;

	CKuiMsgBox msg;
	nRet = msg.ShowMsg(L"您确认恢复该备份中的注册表项吗？", L"提示", MB_YESNO | MB_ICONQUESTION);
	
//	nRet = MessageBox(L"您确认恢复该备份中的注册表项吗？", L"提示", MB_YESNO | MB_ICONQUESTION);

	if(nRet == IDYES)
	{
		backup.GetBackupValue(strSlectFolder);
		//KscFindBackupFile(strSlectFolder);
		//RestoreRegFile(strSlectFolder);
	}
	else
	{
		//m_strSelctFolder = L"";
		return TRUE;
	}

	CKuiMsgBox msgex;
	msgex.ShowMsg(L"本次注册表恢复已完成！", L"提示", MB_OK | MB_ICONINFORMATION);
	//MessageBox(L"本次注册表恢复已完成！", L"提示", MB_OK | MB_ICONINFORMATION);
	return TRUE;
}


LRESULT	CRegBackupDlg::OnDelete(/*WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled*/)
{
	if (m_strSelctFolder == L"")
	{
		CKuiMsgBox msg;
		msg.ShowMsg(L"请选择需要删除的备份！", L"提示", MB_OK | MB_ICONINFORMATION);
//		MessageBox(L"请选择需要删除的备份！", L"提示", MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}
	CKuiMsgBox msg;
	if ( IDYES != msg.ShowMsg(L"您确认删除此次注册表备份吗？", L"删除提示", MB_YESNO | MB_ICONQUESTION) )
	{
		return TRUE;
	}

	CKBackupReg backup;

// 	wstring strSlectFolder  = m_strBackupPath; 
// 	strSlectFolder += L"\\";
// 	strSlectFolder += m_strSelctFolder;
	
	// m_strSelectFolder为记录的resid
	backup.DeleteData(m_strSelctFolder.c_str());

//	DeleteBackupFolder(strSlectFolder);

	m_strSelctFolder = L"";
	//MessageBox(L"清除完成", L"提示", MB_OK);

	m_listBackup.DeleteAllItems();
	m_vsBackupFile.clear();

	KscFindBackupFolder();
	_DoInsertItem();

	SetCtrlStatus();

	return TRUE;
}


LRESULT	CRegBackupDlg::OnDeleteAll(/*WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled*/)
{
	CKBackupReg		backup;
	wstring			strSelectFolder;
	wstring 		strResult;
	int				nCount = 0;
	int				i = 0;
	MSSGS::const_iterator it;
	CKuiMsgBox msgex;

	// 获取备份项数
	nCount = m_listBackup.GetItemCount();
	if ( nCount <= 0 )
	{
		CKuiMsgBox msg;
		msg.ShowMsg(L"目前没有可删除的备份项。", L"删除提示", MB_OK | MB_ICONINFORMATION);
//		MessageBox(L"目前没有可删除的备份项。", L"删除提示", MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}
	else if ( IDYES != msgex.ShowMsg(L"您确认删除全部注册表备份吗？", L"删除提示", MB_YESNO | MB_ICONQUESTION) )
	{
		return TRUE;
	}

	// 逐项删除每份备份
	for (i = 0; i < nCount; ++i)
	{
		m_listBackup.GetItemData(i, strResult);
		if ( strResult.empty() )
			continue;

		it = m_vsBackupTemp.find(strResult);
		if (it != m_vsBackupTemp.end())
		{
			strSelectFolder = it->second;
		}

		backup.DeleteData(strSelectFolder.c_str());
	}

	m_strSelctFolder = L"";
	//MessageBox(L"清除完成", L"提示", MB_OK);

	// 删除所有项目
	m_listBackup.DeleteAllItems();
	m_vsBackupFile.clear();

	// 重新读取备份列表
	KscFindBackupFolder();
	_DoInsertItem();

	SetCtrlStatus();

	return TRUE;
}

LRESULT	CRegBackupDlg::OnClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	EndDialog(0);
	return TRUE;
}


void CRegBackupDlg::SetCtrlStatus()
{
	int				nCount = 0;

	// 获取备份项数
	nCount = m_listBackup.GetItemCount();
	if ( nCount > 0 )
	{
// 		::EnableWindow(m_btnDelete, TRUE);
// 		::EnableWindow(m_btnDeleteAll, TRUE);
// 		::EnableWindow(m_btnRestorek, TRUE);
	}
	else
	{
// 		::EnableWindow(m_btnDelete, FALSE);
// 		::EnableWindow(m_btnDeleteAll, FALSE);
// 		::EnableWindow(m_btnRestorek, FALSE);
	}
}


wstring CRegBackupDlg::GetSysDateString()
{
	SYSTEMTIME sysTime = {0};
	
	WCHAR szDate[MAX_PATH] = {0};
	GetLocalTime(&sysTime);

	swprintf_s(szDate, L"%u年%02u月%02u日 %02u:%02u:%02u", sysTime.wYear, sysTime.wMonth, 
			sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	return wstring(szDate);
}

BOOL CRegBackupDlg::Init()
{
	wstring strFolder;
	WCHAR szFilePath[MAX_PATH] = {0};
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BOOL bRet = FALSE;
//	wstring strTemp = GetSysDateString();
//	m_sBackupKey.clear();

	CreateBackpFile();

	strFolder = m_strBackupPath;
// 	strFolder +=L"\\";
// 	strFolder += strTemp;
	if(::GetFileAttributes(strFolder.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		bRet = KCreateLongDir(strFolder.c_str(), NULL);	
	}
//	m_strTempFolder = strTemp;

	return TRUE;
}

BOOL CRegBackupDlg::Unit()
{
	m_strTempFolder = L"";
	return TRUE;
}

void CRegBackupDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (InTitleZone(point)) 
	{
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	}

	return ;
}

BOOL CRegBackupDlg::GenBackupFileName()
{
	unsigned long ulFileNameNum = 0;
	WCHAR szFileName[MAX_PATH] = { 0 };
	WCHAR szTempString[MAX_PATH] = {0};
	wstring strBackupPath;

	strBackupPath = m_strBackupPath;
	strBackupPath += L"\\";
	strBackupPath += m_strTempFolder;

	ulFileNameNum = rand() % 100000000L;
	do 
	{
		ulFileNameNum= (++ulFileNameNum)% 100000000L;
		_sntprintf(szFileName, (sizeof(szFileName) / sizeof(TCHAR)), TEXT("%08dREG"), ulFileNameNum);

		::wcsncpy(szTempString, strBackupPath.c_str(), MAX_PATH - 1);
		::PathAppend(szTempString, szFileName);

		if (_taccess(szTempString, 0) != 0)
		{
			//文件不存在,生成的文件名可用
			break;
		}
		else
		{
			//生成的文件名已经存在,更新文件名种子
			ulFileNameNum = rand() % 100000000L;
			//printf(" RenameSeed %d ", m_ulFileNameSeed);
		}
	} while(TRUE);

	m_strBackupFileName = szFileName;
	return TRUE;
}

BOOL CRegBackupDlg::SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{

	int nRetCode = false;
	int nResult  = false;

	TOKEN_PRIVILEGES tp;
	LUID luid;

	//CString strOutMsg;

	nRetCode = LookupPrivilegeValue(NULL, lpszPrivilege, &luid);
	if (!nRetCode)
	{
		goto Exit0;
	}

	tp.PrivilegeCount       = 1;
	tp.Privileges[0].Luid   = luid;
	if(bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	nRetCode = AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL
		);
	if (!nRetCode)
		goto Exit0;

	nResult = true;

Exit0:

	return nResult;

}

int CRegBackupDlg::UpperMyPrivilege()
{
	int nRetCode = false;
	int nResult  = false;

	HANDLE hProcessToken = NULL;

	__try
	{
		nRetCode = OpenProcessToken(
			GetCurrentProcess(), 
			TOKEN_ALL_ACCESS, 
			&hProcessToken
			);
		if (!nRetCode)
			__leave;

		nRetCode = SetPrivilege(hProcessToken, SE_DEBUG_NAME, TRUE);
		if (!nRetCode)
			__leave;

		nResult = true;
	}
	__finally
	{
		if (hProcessToken)
		{
			CloseHandle(hProcessToken);
			hProcessToken = NULL;
		}
	}

	return nResult;

}

BOOL CRegBackupDlg::KCreateLongDir(
								   LPCTSTR lpPathName, 
								   LPSECURITY_ATTRIBUTES lpSecurityAttributes
								   )

{
	if (lpPathName == NULL)
	{
		return FALSE;
	}
	if (::PathIsDirectory(lpPathName))
	{
		return TRUE;
	}
	else 
	{
		TCHAR szFatherPath[MAX_PATH];
		::lstrcpyn(szFatherPath, lpPathName, MAX_PATH);
		LPTSTR lpSlash = ::_tcsrchr(szFatherPath, TEXT('\\'));
		if (lpSlash != NULL)
		{
			*lpSlash = 0;
		}
		// 递归，先创建基层目录，再创建子目录。
		if (lpSlash == NULL || KCreateLongDir(szFatherPath, lpSecurityAttributes))
		{
			if (::CreateDirectory(lpPathName, lpSecurityAttributes))
			{
				return TRUE;
			}
		}
		return FALSE;
	}
}

BOOL CRegBackupDlg::GetFileInfo(wstring srtFileFolder, int& nFileCount, int& nFileSize)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	wstring strFilePath = srtFileFolder;
	int nCount = 0;
	int nSize = 0;

	strFilePath += L"\\*.reg";
	hFile = FindFirstFile(strFilePath.c_str(), &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	do
	{
		nFileCount++;
		nFileSize += wfd.nFileSizeLow;
	}while(FindNextFile(hFile, &wfd));

	FindClose(hFile);
	return TRUE;
}

BOOL CRegBackupDlg::DeleteBackupFolder(wstring strFolder)
{
	wstring strFullPath;
	WIN32_FIND_DATA wfd;
	wstring strFileFolder;
	wstring strTemp;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	strFullPath = m_strBackupPath;
	strFullPath += L"\\";
	strFullPath += strFolder;
	strFileFolder = strFullPath;
	strFullPath += L"\\";
	strFullPath += L"*.*";

	hFile = FindFirstFile(strFullPath.c_str(), &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		::RemoveDirectory(strFileFolder.c_str());
		return TRUE;
	}
	do 
	{
		strTemp = strFileFolder;
		strTemp += L"\\";
		strTemp += wfd.cFileName;
		DeleteFile(strTemp.c_str());
		strTemp = L"";
	} while (FindNextFile(hFile, &wfd));
	
	FindClose(hFile);
	::RemoveDirectory(strFileFolder.c_str());
	return TRUE;
}
// 
// 
// void CRegBackupDlg::OnPaint(HDC hDc)
// {
// 	CPaintDC dc(m_hWnd);
// 	GetDialogSize();
// 
// 	RECT rcLeft = { 0, 0, 1, m_nHeight };
// 	RECT rcRigh = { m_nWidth - 1, 0, m_nWidth, m_nHeight };
// 	RECT rcTop = { 0, 0, m_nWidth, 1 };
// 	RECT rcBottomb = { 0, m_nHeight - 1, m_nWidth, m_nHeight };
// 
// 	dc.FillSolidRect(&rcLeft, RGB(50,71,102));
// 	dc.FillSolidRect(&rcRigh, RGB(50,71,102));
// 	dc.FillSolidRect(&rcTop, RGB(50,71,102));
// 	dc.FillSolidRect(&rcBottomb, RGB(50,71,102));
// 
// 	RECT rcMain = { 1, 24, m_nWidth - 1, m_nHeight - 1};
// 	dc.FillSolidRect(&rcMain, RGB(247, 251, 255));
// 
// 	RECT rcTitle = { 1, 1, m_nWidth - 1, 23 };
// 	RECT rcBottom = { 1, 23, m_nWidth - 1, 24};
// 	dc.FillSolidRect(&rcBottom, RGB(155, 198, 142));
// 	SkinDrawGradualColorRect(dc, rcTitle, RGB(239, 247, 255), RGB(207, 231, 255));
// 
// 	//CRect rcClient;
// 	//m_BackupList.Get
// 
// 	WCHAR szTitle[64] = { 0 };
// 	::GetWindowTextW(m_hWnd, szTitle, 64);
// 	CFontHandle wndFont(GetFont());
// 	CFont titleFont;
// 	LOGFONT logFont;
// 	wndFont.GetLogFont(&logFont);
// 	logFont.lfWeight = FW_BOLD;
// 	titleFont.CreateFontIndirect(&logFont);
// 	HGDIOBJ hOld = SelectObject(dc, titleFont);
// 	SetBkMode(dc, TRANSPARENT);
// 	RECT rcTextZone = { 24, 0, m_nWidth, 24 };
// 	DrawTextW(dc, szTitle, (int)wcslen(szTitle), &rcTextZone, DT_VCENTER|DT_SINGLELINE);
// 	SelectObject(dc, hOld);
// 
// 
// 	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON1), 
// 		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
// 	SetIcon(hIcon, TRUE);
// 	if (hIcon) {
// 		dc.DrawIconEx(5, 5, hIcon, 16, 16);
// 	}
// }

void CRegBackupDlg::GetDialogSize() 
{
	CRect rcClient;

	if (0 == m_nWidth && 0 == m_nHeight) 
	{
		GetClientRect(&rcClient);
		m_nWidth = rcClient.right;
		m_nHeight = rcClient.bottom;
	}
}

// void CRegBackupDlg::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
// {
// 	switch(nID)
// 	{
// 	case IDC_CLOSE:
// 	case IDOK:
// 		EndDialog(0);
// 		break;
// 
// 	case ID_DELETE:
// 		OnDelete();
// 		break;
// 
// 	case ID_DELETE_ALL:
// 		OnDeleteAll();
// 		break;
// 
// 	case ID_RESTORE:
// 		OnRestore();
// 		break;
// 	default:
// 		break;
// 	}
// }
// 
// BOOL CRegBackupDlg::InitControlBtn()
// {
// 	CRect		mainWndRect;
// 	CRect		rc;
// 	const int	nBmpWidth = 72, nBmpHeight = 26;
// 	const int	nRightPadding = 9;
// 	int			nLeft = 0;
//    
// 	GetClientRect(mainWndRect);
// 
// //	m_btnClose.Create(m_hWnd);
//  //   m_hbmpButton = ::LoadBitmap(_Module.m_hInst, MAKEINTRESOURCE(IDB_QUIT));
// //     if ( m_hbmpButton != NULL )
// //     {
// //         m_btnClose.InitButton(m_hbmpButton, -2, 2);
// //     }
// // 	m_btnClose.SetWindowLong(GWL_ID, IDOK);
// // 	m_btnClose.ShowWindow(SW_SHOWNA);
// 
// 	//	m_BackupList.SetSelectedItemColor(RGB(255,0,0));
// 
// 	m_bmpTip.SubclassWindow(GetDlgItem(IDC_PIC_TIP));
// 	m_imgListTip.CreateFromImage(IDB_GREEN, 0, 0 , RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
// 	m_bmpTip.mpImageList = &m_imgListTip;
// 	m_bmpTip.mnIndex = 0;
// 	m_bmpTip.ShowWindow(SW_SHOW);
// 
// 	m_sbTxt.SubclassWindow(GetDlgItem(IDC_TXT_STATIC));
// 	m_sbTxt.SetWindowText(L"选择备份的项目，点击“恢复”，将还原到清理之前的状态。");
// 	m_sbTxt.SetTransparent(TRUE);
// 
// 	SET_HYPERCR_LINK(m_btnDelete, ID_DELETE);
// 	SET_HYPERCR_LINK(m_btnDeleteAll, ID_DELETE_ALL);
// 
// 	m_btnClosek.SubclassWindow(GetDlgItem(IDC_CLOSE));
// 	m_btnClosek.SetBitmapButtonExtendedStyle(BMPBTN_HOVER);
// 	m_btnClosek.SetBtnImages(IDB_BTNNORMAL_B, IDB_BTNDOWN_B, IDB_BTNHOVER_B, IDB_BTNDISABLE_B);
// 	m_btnClosek.SetImages(0, 1, 2, 3);
// 	m_btnClosek.SetCursor((UINT)(ULONG_PTR)IDC_HAND);
// 	m_btnClosek.SetTitle(IDS_BACKUP_CLOSE);
// 
// 	m_btnClosek.GetWindowRect(rc);
// 	ScreenToClient(rc);
// 	nLeft = mainWndRect.right - nBmpWidth - nRightPadding;
// 	m_btnClosek.SetWindowPos(NULL, nLeft, rc.top, nBmpWidth, nBmpHeight, SWP_NOZORDER);
// 
// 	m_btnRestorek.SubclassWindow(GetDlgItem(ID_RESTORE));
// 	m_btnRestorek.SetBitmapButtonExtendedStyle(BMPBTN_HOVER);
// 	m_btnRestorek.SetBtnImages(IDB_BTNNORMAL_B, IDB_BTNDOWN_B, IDB_BTNHOVER_B, IDB_BTNDISABLE_B);
// 	m_btnRestorek.SetImages(0, 1, 2, 3);
// 	m_btnRestorek.SetCursor((UINT)(ULONG_PTR)IDC_HAND);
// 	m_btnRestorek.SetTitle(IDS_BACKUP_RESTORE);
// 
// 	m_btnRestorek.GetWindowRect(rc);
// 	ScreenToClient(rc);
// 	nLeft -= nBmpWidth + 10;
// 	m_btnRestorek.SetWindowPos(NULL, nLeft, rc.top, nBmpWidth, nBmpHeight, SWP_NOZORDER);
// 
// 	return TRUE;
// }
// 

BOOL CRegBackupDlg::InitBackupList()
{
	CString		sText;
	CRect		rc;
	COLORREF	color = RGB(222, 239, 214);
	CRect		mainWndRect;

	m_listBackup.RegisterClass();
//	m_listBackup.SubclassWindow(GetDlgItem(IDC_BACKUP_LIST));
	m_listBackup.Create(GetRichWnd(), NULL, NULL, NULL, 0, 4420, NULL );
	m_listBackup.EnableMultiSelection(false);
	GetClientRect(mainWndRect);
	m_listBackup.GetWindowRect(rc);
	ScreenToClient(rc);
	m_listBackup.SetWindowPos(NULL, 0, 0, mainWndRect.Width() - rc.left - 9, rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
	
	//CKListSetting::Instance().GetColor(TEXT(""), color);
	color = RGB(185, 219, 255);
	/*if ( m_bParentKsafe )
		color = RGB(185, 219, 255);
	else
		color = RGB(222, 239, 214);*/
	m_listBackup.SetSelectedItemColor(color);

	sText = TEXT("备份时间");
	m_listBackup.AddColumn(sText, 200);

	sText = TEXT("备份内容");
	m_listBackup.AddColumn(sText, 280);

	return TRUE;

}


LRESULT CRegBackupDlg::OnListSelect(int idCtrl, LPNMHDR pnmh,  BOOL &bHandled)
{
	CListNotify*				pNotify = (CListNotify *)pnmh;
	UINT						unTemp = 0;
	wstring 					strResult;
	MSSGS::const_iterator it;

	if ( pNotify->m_nItem == NULL_ITEM )
		return 0;

	// 自定义数据为resid
	m_listBackup.GetItemData(pNotify->m_nItem, strResult);
	if ( strResult == L"")
		return 0;

	it = m_vsBackupTemp.find(strResult);
	if (it != m_vsBackupTemp.end())
	{
		m_strSelctFolder = it->second;
	}
	return 0;
}
