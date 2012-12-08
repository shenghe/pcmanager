#include "StdAfx.h"
#include "MainDlg.h"
#include "../libDownload/Utilfunction.h"
#include "../../publish/vulfix/BeikeUtils.h"
#include "../libDownload/HttpAsyncRequest.h"
#include <comutil.h>
#include "VectorPersist.h"
#include "DlgInputUrl.h"
#include "../libDownload/MD5Checksum.h"
#include <algorithm>
#include "../libDownload/stdafx.h"
#include <map>

struct FileMd5{
	LPCTSTR szMD5, szFileName;
};
static FileMd5 files_md5[] =
{
	{_T("ea16789f6fc1d2523f704e8f9afbe906"), _T("IE7-WindowsXP-x86-enu.exe")}, 
	{_T("226d31305c5375f62de7933b0d6c53e9"), _T("IE8-WindowsXP-KB973874-x86-ENU.exe")},
	{_T("acde10a1f8659cf91a3a323590195b48"), _T("IE8-WindowsXP-x86-CHS.exe")},
	{_T("8ddc0ebea651bab92d189ac1f37000d4"), _T("oe5.5sp2-KB897715-Windows2000-x86-CHS.exe")},
	{_T("4ea9e067922b9bfe41f7c612687bd85c"), _T("office2003-KB956077-FullFile-ENU.exe")},
	{_T("cbfe6f6ebea03870721812a685faea03"), _T("Opera_1051_int_Setup.rar")},
	{_T("6b6c30de9067a3983aebc1b59e7939ea"), _T("QQSoftMgr_Setup_1.0.359.203.exe")},
	{_T("cf728aa8ef82979ad4e6d18542bd9927"), _T("TM2009Beta2.3_chs.exe")},
	{_T("70718a0f5f36f626d00b03cdd3aa4131"), _T("TM2009Beta3.0_chs.exe")},
	{_T("834ea97a22fbf2b2a0fb51c7248a31f0"), _T("Windows6.1-KB974431-x86.msu")},
	{_T("c3fb747e5960088ab90b4fd104055495"), _T("Windows6.1-KB978207-x86.msu")},
	{_T("cd74a575329fb91f2b084e759dadf70e"), _T("WindowsXP-WindowsMedia-KB973540-x86-CHS.exe")},
	{_T("33a45b140b607144f9cdd9330bec42d8"),_T("IE6.0sp1-KB976325-Windows2000-x86-ENU.exe")},
	{_T("b0d7a6df573b9f78cd4fd684602c7320"),_T("IE8-WindowsServer2003-KB969497-x86-CHS.exe")},


	{_T("1373d2953b1a0385a9904470953f37ae"),_T("OE6.0sp1-KB973354-Windows2000-x86-ENU.exe")},
	{_T("80395531001af853692edb803b0609f1"),_T("office2003-KB975051-FullFile-ENU.exe")},
	{_T("ba3b0747c605e3eb5cb7a8783860bf4f"),_T("Windows2000-KB948745-x86-CHS.EXE")},
	{_T("fb660b8caa29209a3650a1d5d00e9199"),_T("Windows2000-KB970238-x86-CHS.EXE")},
	{_T("51f2c71fcbfba0eb740fa676c973d81f"),_T("Windows6.0-KB938127-x86.msu")},
	{_T("c882acf93361446a6adfc0e1eb14071e"),_T("Windows6.0-KB952287-x64.msu")},
	{_T("216704f3f723678b0a8bf2570f4fc58e"),_T("WindowsMedia9-KB936782-x86-ENU.exe")},
	{_T("6f27d2118e48f944f914ca69d8f3c13b"),_T("WindowsMedia9-KB954156-INTL.exe")},
	{_T("21359ec937337974335a352893a1e329"),_T("WindowsServer2003-KB929123-x86-CHS.exe")},
	{_T("2ce0c4b9d4ede09fa715b1f175c5a546"),_T("WindowsServer2003-KB935840-x86-ENU.exe")},
	{_T("6810949f5b2fa1bd8f568cee9f01ce2f"),_T("WindowsServer2003-KB936021-x86-CHS.exe")},
	{_T("440b17938df1aceccb1330b4111ad192"),_T("WindowsServer2003-KB942831-x86-ENU.exe")},
	{_T("a5cef2d702a60288ca6e81e6d7747dab"),_T("WindowsServer2003-KB950224-v3-x86-ENU.exe")},
	{_T("ffe68b821e4e39e1b16e0f40ef12ce45"),_T("WindowsServer2003-KB972270-x86-CHS.exe")},
	{_T("495874bcb20cb97e9e4cd3a98ba0fa89"),_T("WindowsServer2003.WindowsXP-KB958644-x64-CHS.exe")},
	{_T("f9cc2530805ada31e72f5016df37d66e"),_T("WindowsXP-KB918118-x86-CHS.exe")},
	{_T("6785b7093a117fd263b0eb349eab4cc3"),_T("WindowsXP-KB933729-x86-CHS.exe")},
	{_T("12f92378a8185377ae9e7143c8f5ba15"),_T("WindowsXP-KB935448-x86-ENU.exe")},
	{_T("31361beb8b17c1ef18c820168e29a8d0"),_T("WindowsXP-KB943198-v2-x86-CHS.exe")},
};



LPCSTR DTINFO_FILE_HEADER = "dtinfo";
enum DOWN_STATE
{
	DOWN_STOP,
	DOWN_RUN,
	DOWN_DONE,
};
struct TDownItem
{
	TDownItem()
	{
		ZeroMemory(szUrl, MAX_PATH);
		state = DOWN_STOP;
		code = 0;
		tmBegin.dwHighDateTime = tmBegin.dwLowDateTime = 0;
		tmEnd.dwHighDateTime = tmEnd.dwLowDateTime = 0;
	}
	TCHAR szUrl[MAX_PATH];
	INT state;
	FILETIME tmBegin, tmEnd;
	INT code;
};

LPCTSTR GetDownStateDesc(TDownItem *pItem)
{
	switch(pItem->state)
	{
	case DOWN_STOP:
		return _T("Stop");
	case DOWN_RUN:
		return _T("Wait");
	case DOWN_DONE:
		return _T("Done");
	default:
		return _T("Err State");
	}
}

LRESULT CMainDlg::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
	
	UIAddChildWindowContainer(m_hWnd);

#ifdef _DEBUG
//#if 0 
	// --init 
	AllocConsole();
	freopen("CONOUT$","w+t",stdout); 
	freopen("CONIN$","r+t",stdin); 
#endif
	
	m_btnCheckTestMode.Attach(GetDlgItem(IDC_CHECK_TESTDOWNMODE));

	//
	LPCTSTR szLocalfile = _T("C:\\Downloads_2");
	
	CompletePathWithModulePath(m_strIniFile, _T("_ldt.ini"));
	CompletePathWithModulePath(m_strDatFile, _T("_ldt.dat"));
	CString strLocalPath;
	GetPrivateProfileString(_T("Main"), _T("downpath"), szLocalfile, strLocalPath.GetBuffer(MAX_PATH), MAX_PATH-1, m_strIniFile);
	strLocalPath.ReleaseBuffer();

	m_nCoTask = GetPrivateProfileInt(_T("Main"), _T("cotask"), 1, m_strIniFile);
	m_nCorrurent = GetPrivateProfileInt(_T("Main"), _T("cothread"), 4, m_strIniFile);

	SetDlgItemText(IDC_EDIT_LOCALPATH, strLocalPath);
	
	// 
	m_ctlCocurrent.Attach(GetDlgItem(IDC_COMBO_COCURRENT));
	for(int i=2; i<=8; ++i)
	{
		CString str;
		str.Format(_T("%d"), i);
		m_ctlCocurrent.AddString(str);
	}
	//m_nCorrurent = 4;
	m_ctlCocurrent.SetCurSel(m_nCorrurent-2);
	
	m_ctlCoTask.Attach(GetDlgItem(IDC_COMBO_COCURRENTTASK));
	for(int i=1; i<=8; ++i)
	{
		CString str;
		str.Format(_T("%d"), i);
		m_ctlCoTask.AddString(str);
	}
	m_ctlCoTask.SetCurSel(m_nCoTask-1);
	
	
	m_btnSelPath.Attach(GetDlgItem(IDC_BUTTON_SEL_DOWNPATH));

	m_lstUrls.Attach( GetDlgItem(IDC_LIST_URLPROCESSING) );
	m_lstUrls.InsertColumn(0, _T("URL"), 0, 300);
	m_lstUrls.InsertColumn(1, _T("File Size"), 0, 60);
	m_lstUrls.InsertColumn(2, _T("Status"), 0, 200);
	//m_lstUrls.ModifyStyle(0, LVS_SINGLESEL, 0);
	m_lstUrls.SetExtendedListViewStyle( LVS_EX_FULLROWSELECT|m_lstUrls.GetExtendedListViewStyle() );	

	CString strDateFile;
	SYSTEMTIME tm;
	std::vector<TDownItem> items;
	CVectorPersist<TDownItem>(m_strDatFile).Load(DTINFO_FILE_HEADER, items, tm);
	for(int i=0; i<items.size(); ++i)
	{
		TDownItem *pd = new TDownItem;
		memcpy(pd, &(items[i]), sizeof(TDownItem));
		
		INT nItem = m_lstUrls.GetItemCount();
		nItem = m_lstUrls.AddItem(nItem, 0, pd->szUrl);
		m_lstUrls.SetItemText(nItem, 2, GetDownStateDesc(pd));
		m_lstUrls.SetItemData(nItem, (DWORD_PTR)pd);
	}
	
	CString strUrlfile;
#if 0
	//LPCTSTR szUrlfile = _T("C:\\download.urls.txt");
	LPCTSTR szUrlfile = _T("c:\\test_urls.txt");
	strUrlfile = szUrlfile;
#else	
	CompletePathWithModulePath(strUrlfile, _T("u2.txt"));
#endif
	
	if(m_lstUrls.GetItemCount()==0)
	{
		if(PathFileExists(strUrlfile))
		{
			ImportUrlList(strUrlfile);
		}
	}	
	_EnableFuncButtons(TRUE);

	SetTimer(1, 1000, NULL);


	CompletePathWithModulePath(m_strMd5File, _T("filemd5.txt"));
	m_filemd5s.Load(m_strMd5File);
	m_nRestartTickCount = 0;
	return TRUE;
}

void CMainDlg::CloseDialog( int nVal )
{
	GetDlgItemText(IDC_EDIT_LOCALPATH, m_strPath);
	WritePrivateProfileString(_T("Main"), _T("downpath"), m_strPath, m_strIniFile);
	CString strNum;
	m_ctlCoTask.GetWindowText(strNum);
	WritePrivateProfileString(_T("Main"), _T("cotask"), strNum, m_strIniFile);
	m_ctlCocurrent.GetWindowText(strNum);
	WritePrivateProfileString(_T("Main"), _T("cothread"), strNum, m_strIniFile);
	
	//
	std::vector<TDownItem> items;
	for(int i=0; i<m_lstUrls.GetItemCount(); ++i)
	{
		TDownItem *p = (TDownItem*)m_lstUrls.GetItemData(i);
		if(p)
		{
			items.push_back(*p);
		}
	}
	CVectorPersist<TDownItem>(m_strDatFile).Save(DTINFO_FILE_HEADER, items);
	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetDlgItemText(IDC_EDIT_LOCALPATH, m_strPath);
	if(!CreateDirectoryNested(m_strPath))
	{
		MessageBox(_T("Prepare Direct Error"));
		return 0;
	}
	
	for(int i=0; i<m_lstUrls.GetItemCount(); ++i)
	{
		TDownItem *pitem = (TDownItem*)m_lstUrls.GetItemData(i);
		if(pitem)
		{
			if(pitem->state!=DOWN_DONE)
				pitem->state = DOWN_RUN;

			if(pitem->state==DOWN_RUN)
			{
				pitem->code = 0;
			}
		}
	}
	
	m_nCorrurent = m_ctlCocurrent.GetCurSel() + 2;
	m_nCoTask = m_ctlCoTask.GetCurSel() + 1;
	
	m_arrDownloadsQueuing.clear();
	std::copy(m_arrDownloads.begin(), m_arrDownloads.end(), std::back_inserter(m_arrDownloadsQueuing));
	
	while(_DownloadNext(0))
		;
	
	if(!m_arrDownloadsRunning.empty())
		_EnableFuncButtons(FALSE);
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for(IDownloadPtrs::iterator it=m_arrDownloadsRunning.begin(); it!=m_arrDownloadsRunning.end(); ++it)
	{
		(*it)->Stop();
	}
	m_arrDownloadsRunning.clear();
	_EnableFuncButtons(TRUE);
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	USES_CONVERSION;
	TCHAR szFilters[]=_T("All files(*.*)\0*.*\0\0");
	CFileDialog dlg(true, _T("*"), _T("*.*"), OFN_FILEMUSTEXIST,szFilters);
	if(dlg.DoModal()==IDOK)
	{
		ImportUrlList(dlg.m_szFileName);
	}
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for(int i=0; i<m_lstUrls.GetItemCount(); ++i)
	{
		TDownItem *pItem = (TDownItem *)m_lstUrls.GetItemData(i);
		if(pItem)
			delete pItem;
	}
	m_lstUrls.DeleteAllItems();
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonAddurl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CDlgInputUrl dlg;
	if(IDOK==dlg.DoModal())
	{
		LPCTSTR httpBegin = _T("http://");
		if(_tcsnicmp(httpBegin, dlg.m_strUrl, _tcslen(httpBegin))==0 && dlg.m_strUrl.GetLength()>10)
		{
			AddUrl(dlg.m_strUrl);
		}
		else
		{
			MessageBox(_T("Url 无效"));
		}
	}
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonDelurl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	INT nItem = -1;
	std::vector<INT> indexs;
	while(TRUE)
	{
		nItem = m_lstUrls.GetNextItem(nItem, LVNI_SELECTED);
		if(nItem==-1)
			break;
		indexs.push_back(nItem);
	}
	
	for(std::vector<INT>::reverse_iterator it=indexs.rbegin(); it!=indexs.rend(); ++it)
	{
		INT nItem = *it;
		TDownItem *pItem = (TDownItem *)m_lstUrls.GetItemData(nItem);
		if(pItem)
			delete pItem;
		m_lstUrls.DeleteItem(nItem);		
	}
	return 0;
}

BOOL SelectFolder(CString &strPath, HWND hWnd)
{
	BROWSEINFO bInfo = {0};
	bInfo.hwndOwner = hWnd;
	bInfo.lpszTitle = _T("请选择路径: ");
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_STATUSTEXT;

	LPITEMIDLIST lpDlist;
	lpDlist = SHBrowseForFolder(&bInfo);
	if(lpDlist==NULL)
		return FALSE;

	TCHAR szPath[MAX_PATH] = {0};
	SHGetPathFromIDList(lpDlist, szPath );
	strPath = szPath;
	return TRUE;
}

LRESULT CMainDlg::OnBnClickedButtonSelDownpath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString strNewPath;
	if(SelectFolder(strNewPath, m_hWnd))
	{
		SetDlgItemText(IDC_EDIT_LOCALPATH, strNewPath);
	}
	return 0;
}

BOOL CMainDlg::OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam )
{
	PostMessage(UWM_DOWNLOADCALLBACK, (WPARAM)pDownload, (LPARAM)state);
	return TRUE;
}

LRESULT CMainDlg::OnDownloadEvent( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ )
{
	CString strMsg;
	IDownload *pDownload = (IDownload*) wParam;
	int nitem = (int)pDownload->GetUserData();
	
	switch(lParam)
	{
	case ProcessState_Finished:
	case ProcessState_UserCanceled:
	case ProcessState_Failed:
		{
			CString strSpeed;
			IDownStat *pdownStat = pDownload->GetDownloadStat();
			FormatSizeString(pdownStat->AverageSpeed(), strSpeed);
			if(lParam==ProcessState_Finished)
				strMsg.Format(_T("已完成下载: 平均速度:%s"), strSpeed);
			else if(lParam==ProcessState_UserCanceled)
				strMsg.Format(_T("被用户取消: 平均速度:%s"), strSpeed);
			else if(lParam==ProcessState_Failed)
				strMsg.Format(_T("下载失败, 错误码: %d"), pDownload->LastError() );
			m_lstUrls.SetItemText(nitem, 2, strMsg);
			
			TDownItem *pItem = (TDownItem *)m_lstUrls.GetItemData(nitem);
			if(pItem)
			{
				if(lParam==ProcessState_Finished)
					pItem->state = DOWN_DONE;
				
				CString strUrl, strFilePath;
				pDownload->GetDownloadInfo(strUrl, strFilePath);
				BOOL bMatched = FALSE;
				if(m_filemd5s.IsFileMd5Matched(strFilePath, bMatched))
				{
					LPCTSTR szMd5Result = bMatched?_T("MD5一致"):_T("MD5错误!");
					MYTRACE(_T("%s %s\r\n"), strFilePath, szMd5Result);
					m_lstUrls.SetItemText(nitem, 1, szMd5Result);
				}
			}
			
			//m_lstUrls.SetItemData(nitem, lParam);
			if(lParam!=ProcessState_UserCanceled )
			{
				IDownloadPtrs::iterator it=std::find(m_arrDownloadsRunning.begin(), m_arrDownloadsRunning.end(), pDownload);
				ATLASSERT(it!=m_arrDownloadsRunning.end());
				if(it!=m_arrDownloadsRunning.end())
					m_arrDownloadsRunning.erase(it);
				pDownload->Stop();
				it=std::find(m_arrDownloadsQueuing.begin(), m_arrDownloadsQueuing.end(), pDownload);
				ATLASSERT(it==m_arrDownloadsQueuing.end());
				if(it==m_arrDownloadsQueuing.end())
					m_arrDownloadsQueuing.push_back(pDownload);
				_DownloadNext(nitem);

				if(m_arrDownloadsRunning.empty())
				{
					_OnDownloadDone();
				}
			}
		}
		break;

	case ProcessState_ReceiveData:
		{
			RemoteFileInfo *fi = pDownload->GetRemoteFileInfo();
			IDownStat *ds = pDownload->GetDownloadStat();

			CString strSize;
			FormatSizeString(fi->fileSize, strSize);
			m_lstUrls.SetItemText(nitem, 1, strSize);

			CString strSpeed;
			FormatSizeString(ds->Speed(), strSpeed);
			strSpeed += _T("/S");
			strSpeed = _T("  ") + strSpeed;
			if( fi->fileSize>0 )
			{
				int64 progress = (fi->fileDownloaded*100)/fi->fileSize;
				CString strPercent;
				strPercent.Format(_T("%d%%"), progress);
				m_lstUrls.SetItemText(nitem, 2, strPercent + strSpeed);
			}
			else
			{
				CString strDownloaded;
				FormatSizeString(fi->fileDownloaded, strDownloaded);
				m_lstUrls.SetItemText(nitem, 2, strDownloaded+strSpeed);
			}
		}
		break;
	}
	return TRUE;
}

void CMainDlg::ImportUrlList( LPCTSTR szFilename )
{
	std::vector<std::string> urls;
	GetUrlsFromFile(szFilename, urls);
	for(int i=0; i<urls.size(); ++i)
	{
		CString str = CA2CT(urls[i].c_str());
		AddUrl(str);
	}
}

void CMainDlg::AddUrl( LPCTSTR szUrl )
{
	INT nItem = m_lstUrls.GetItemCount();
	TDownItem *pItem = new TDownItem;
	ZeroMemory(pItem, sizeof(TDownItem));
	_tcscpy(pItem->szUrl, szUrl);
	pItem->state = DOWN_RUN;

	nItem = m_lstUrls.AddItem(nItem, 0, szUrl);
	m_lstUrls.SetItemText(nItem, 2, GetDownStateDesc(pItem));
	m_lstUrls.SetItemData(nItem, (DWORD_PTR)pItem);
}

void CMainDlg::_EnableFuncButtons( BOOL enableStart )
{
	GetDlgItem(IDC_BUTTON_START).EnableWindow(enableStart);
	GetDlgItem(IDC_BUTTON_STOP).EnableWindow(!enableStart);
	GetDlgItem(IDC_BUTTON_IMPORT).EnableWindow(enableStart);
	GetDlgItem(IDC_BUTTON_CLEAR).EnableWindow(enableStart);
	m_ctlCocurrent.EnableWindow(enableStart);
	m_ctlCoTask.EnableWindow(enableStart);
	GetDlgItem(IDC_BUTTON_ADDURL).EnableWindow(enableStart);
	GetDlgItem(IDC_BUTTON_DELURL).EnableWindow(enableStart);
	GetDlgItem(IDC_EDIT_LOCALPATH).EnableWindow(enableStart);
	m_btnSelPath.EnableWindow(enableStart);
}

BOOL CMainDlg::_DownloadNext( INT nCurrent )
{
	if(m_arrDownloadsRunning.size()>=m_nCoTask)
		return FALSE;
	
	int next = -1;
	CString strUrl, strFilename, strFile;
	for(int i=nCurrent; i<m_lstUrls.GetItemCount(); ++i)
	{
		TDownItem *pItem = (TDownItem*)m_lstUrls.GetItemData(i);
		if(!pItem)
			continue;
		if(pItem->state==DOWN_DONE)
			continue;
		if(pItem->state==DOWN_RUN && pItem->code==0)
		{
			BSTR bs=NULL;
			m_lstUrls.GetItemText(i, 0, bs);
			strUrl = bs;
			SysFreeString( bs );

			GetFileNameFromUrl( strUrl, strFilename );
			if(strFilename.IsEmpty())
				strFilename = _T("index.html");
			strFile = m_strPath + _T("\\");
			strFile += strFilename;

			if( PathFileExists(strFile) )
			{
				m_lstUrls.SetItemText(i, 2, _T("文件已存在"));
			}
			else
			{
				next = i;
				pItem->code = 1;
				break;
			}
		}
	}
	
	if(next==-1)
		return FALSE;
	
	//m_lstUrls.SelectItem( next );
	m_lstUrls.EnsureVisible(next, TRUE);
	
	if(m_arrDownloadsQueuing.empty())
	{
		IDownloadPtr pd = NULL;
		ATLVERIFY( SUCCEEDED(CreateDownloadObject(__uuidof(IDownload), (VOID**)&pd)) );
		pd->SetObserver(this);

		m_arrDownloads.push_back(pd);
		m_arrDownloadsQueuing.push_back(pd);
	}
	IDownloadPtr pd = m_arrDownloadsQueuing[m_arrDownloadsQueuing.size()-1];
	m_arrDownloadsQueuing.pop_back();
	
	pd->Stop();
	pd->SetDownloadInfo( strUrl, strFile );
	pd->SetUserData( (LPVOID)next );
	pd->Start(m_nCorrurent);
	
	m_arrDownloadsRunning.push_back(pd);
	return TRUE;
}

#include <atlfile.h>
typedef __int64 int64;
LRESULT CMainDlg::OnBnClickedButtonJumpdownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LPCTSTR szUrl = _T("http://download.microsoft.com/download/0/4/3/0438ED90-993C-4F5B-A6BA-E31E516E5473/IE8-WindowsXP-KB971930-x86-CHS.exe");

	UrlInfo urlinfo;
	CrackUrl(szUrl, urlinfo);

	CAtlFile f;
	if( FAILED(f.Create(_T("c:\\IE8-WindowsXP-KB971930-x86-CHS.exe"), GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS)) )
		return 0;
	
	int64 len = 512880;
	int npart = 10;
	int nsize = len/npart;
	CHttpAsyncIn http;
	BOOL bFirst = TRUE;
	for(int i=0; i<npart; ++i)
	{
		int64 b = nsize*i, e = nsize*(i+1);
		if(e>len)
			e = len;
		if(bFirst)
		{
			http.Open(&urlinfo, b, e, len);
			bFirst = FALSE;
		}
		else
			http.OpenRange(b, e);

		while(true)
		{
			WaitForSingleObject(http.GetWaitHandle(), INFINITE);
			EAsyncInState st = http.Process();
			if(st==EAsync_Failed || st==EAsync_Finished)
				break;

			if(st==EAsync_Data)
			{
				INTERNET_BUFFERS *pbuffer = (INTERNET_BUFFERS*) http.GetLastBuffer();
				f.Write(pbuffer->lpvBuffer, pbuffer->dwBufferLength);
			}
		}
	}
	f.Close();
	return 0;
}

void CMainDlg::OnTimer( UINT_PTR nIDEvent )
{
	if(nIDEvent==1)
	{
		if(!m_arrDownloadsRunning.empty())
		{
			INT64 lSpeed = 0;
			for(IDownloadPtrs::iterator it=m_arrDownloadsRunning.begin(); it!=m_arrDownloadsRunning.end(); ++it)
			{
				IDownStat *ds = (*it)->GetDownloadStat();
				lSpeed += ds->Speed();
			}

			CString strSpeed;
			FormatSizeString(lSpeed, strSpeed);
			SetWindowText(strSpeed);
		}
		else
		{
			SetWindowText(_T("无运行任务"));
		}
	}
	else
	{
		 --m_nRestartTickCount;
		 if(m_nRestartTickCount<0)
		 {
			m_nRestartTickCount = (1+(rand()%15)) * 60 ; // 1- 15 分钟 
			
			BOOL bHandled = FALSE;
			GetDlgItem(IDC_BUTTON_STOP).PostMessage(BM_CLICK);
			Sleep(5000);
			GetDlgItem(IDC_BUTTON_START).PostMessage(BM_CLICK);	
		 }
	}
}
LRESULT CMainDlg::OnBnClickedCheckTestdownmode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	INT nStat = m_btnCheckTestMode.GetCheck();
	MessageBox(nStat ? _T("开始自动测试模式") : _T("停止自动测试模式")); 

	if(nStat)
	{
		m_nRestartTickCount = (1+(rand()%15)) * 60 ; // 1- 15 分钟 
		//m_nRestartTickCount = 10;
		SetTimer(2, 1000, NULL);
	}
	else
	{
		KillTimer(2);
	}

	return 0;
}

void CMainDlg::_OnDownloadDone()
{
	// Save the stat 
	INT nStat = m_btnCheckTestMode.GetCheck();
	if(nStat)
	{
		// TODO : 最好根据数字来切换目录名 
		GetDlgItemText(IDC_EDIT_LOCALPATH, m_strPath);
		while(PathIsDirectory(m_strPath))
		{
			m_strPath += _T("_");
		}
		SetDlgItemText(IDC_EDIT_LOCALPATH, m_strPath);
		CreateDirectoryNested(m_strPath);
		
		// Begin Next Round download 
		for(int i=0; i<m_lstUrls.GetItemCount(); ++i)
		{
			TDownItem *pitem = (TDownItem*)m_lstUrls.GetItemData(i);
			if(pitem)
			{
				pitem->state = DOWN_STOP;
			}
		}
		_EnableFuncButtons(TRUE);
		GetDlgItem(IDC_BUTTON_START).PostMessage(BM_CLICK);
	}
	else
		_EnableFuncButtons(TRUE);
}

BOOL CFileMd5List::Load( LPCTSTR szFilename )
{
	FILE *hf = _tfopen(szFilename, _T("rb"));
	if(hf)
	{
		CHAR buffer[MAX_PATH];
		while(fgets(buffer, MAX_PATH, hf))
		{
			CString strFilename, strMd5;
			std::string s;
			const char *pblank = strchr(buffer, ' ');
			if(pblank)
			{
				s.assign(buffer, pblank-buffer);
				strMd5 = CA2CT(s.c_str());

				++pblank;
				const char *pend = strchr(pblank, '\r');
				if(pend)
				{
					s.assign(pblank, pend-pblank);
					strFilename = CA2CT(s.c_str());
					strFilename.MakeLower();
					m_md5s[strFilename] = strMd5;
				}
			}
		}
		fclose(hf);
		return TRUE;
	}
	return FALSE;
}

BOOL CFileMd5List::IsFileMd5Matched( LPCTSTR szFilePath, BOOL &bMatched )
{
	if(!PathFileExists(szFilePath))
		return FALSE;

	LPCTSTR pslash = _tcsrchr(szFilePath, _T('\\'));
	if(pslash)
	{
		LPCTSTR szFileName = pslash + 1;
		CString sFilename = szFileName;
		sFilename.MakeLower();

		CStringMap::iterator it = m_md5s.find(sFilename);
		if(it!=m_md5s.end())
		{
			CString smd5 = CMD5Checksum::GetMD5( szFilePath );
			bMatched = _tcsicmp(smd5, it->second)==0;
			return TRUE;
		}
	}
	return FALSE;
}
