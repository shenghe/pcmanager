#include "StdAfx.h"
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"
#include "RichEditCtrlEx.h"
#include "BeikeVulfixEngine.h"
#include <common/utility.h>
#include <bksafe/bksafeconfig.h>
#include "Vulfix/Defines.h"
#include <miniutil/bkprocprivilege.h>
#include <TlHelp32.h>
#include <vector>
#include <algorithm>

const COLORREF black = RGB(0,0,0);
const COLORREF red = RGB(255,0,0);
const COLORREF green = RGB(0,255,0);
const COLORREF blue = RGB(0,0,255);
const COLORREF yellow = RGB(255,255,0);
const COLORREF brown = RGB(0xb2, 0x5d, 0x25);

// void ShutDownComputer(BOOL toReboot)
// {
// 	BOOL go = TRUE;
// #ifdef _DEBUG
// 	go = IDYES == ::MessageBox(NULL, toReboot ? BkString::Get(IDS_VULFIX_5101):BkString::Get(IDS_VULFIX_5102), _T("ShutDownComputer"), MB_YESNO);
// #endif
// 	if( go )
// 	{
// 		UINT flags = toReboot ? EWX_REBOOT : EWX_SHUTDOWN;
// 		flags |= EWX_FORCE;//强制
// 		
// 		CBkProcPrivilege privilege;
// 		if (!privilege.EnableShutdown())
// 			return;
// 		::ExitWindowsEx(flags, 0);
// 	}
// }

void FormatSizeString(INT64 nFileSize, CString &str)
{
	const INT64 SIZE_KILO = 1024;
	const INT64 SIZE_MEGA = SIZE_KILO * SIZE_KILO;
	const INT64 SIZE_GIGA = SIZE_MEGA * SIZE_KILO;

	if(nFileSize==0)
		str = _T("0");
	else if(nFileSize<SIZE_KILO)
		str.Format(_T("%dB"), nFileSize);
	else if(nFileSize<SIZE_MEGA)
		str.Format(_T("%.2fKB"), nFileSize*1.0/SIZE_KILO );
	else if(nFileSize<SIZE_GIGA)
		str.Format(_T("%.2fMB"), nFileSize*1.0/SIZE_MEGA );
	else
		str.Format(_T("%.2fGB"), nFileSize*1.0/SIZE_GIGA );
}


INT64 GetFolderSize(LPCTSTR szFolder, BOOL bIncludeChild, FolderFilter fn)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	CString strFolder(szFolder);
	strFolder.Append( _T("\\*") );

	INT64 folderSize = 0;
	hFind = FindFirstFile(strFolder, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		printf ("First file name is %s\n", FindFileData.cFileName);
		do 
		{
			printf ("Next file name is %s\n", FindFileData.cFileName);

			if( _tcsicmp(_T("."), FindFileData.cFileName)==0 
				|| _tcsicmp(_T(".."), FindFileData.cFileName)==0 )
			{
				continue;
			}
			if( FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				if( bIncludeChild )
				{
					CString strResult = szFolder;
					LPCTSTR pszResult;
					LPCTSTR pchLast;
					pszResult = strResult;
					pchLast = _tcsdec( pszResult, pszResult+strResult.GetLength() );
					ATLASSERT(pchLast!=NULL);
					if ((*pchLast != _T('\\')) && (*pchLast != _T('/')))
						strResult += _T('\\');
					strResult += FindFileData.cFileName;
					folderSize += GetFolderSize( strResult, bIncludeChild );
				}
			}
			else
			{
				if( !fn || fn(szFolder, FindFileData.cFileName) )
				{
					ULARGE_INTEGER nFileSize;
					nFileSize.LowPart = FindFileData.nFileSizeLow;
					nFileSize.HighPart = FindFileData.nFileSizeHigh;
					folderSize += nFileSize.QuadPart;
				}
			}

		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return folderSize;
}

BOOL IsBkDownloadFile(LPCTSTR szFolder, LPCTSTR szFilename)
{
	return _tcsnicmp(szFilename, BK_FILE_PREFIX, _tcslen(BK_FILE_PREFIX))==0;
}

BOOL SelectFolder(CString &strPath, HWND hWnd)
{
	BROWSEINFO bInfo = {0};
	bInfo.hwndOwner = hWnd;
	bInfo.lpszTitle = BkString::Get(IDS_VULFIX_5103);
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


static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	switch(uMsg)
	{
	case BFFM_INITIALIZED:    //初始化消息
		//传递默认打开路径 （方法一）::SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)"C:\\Program Files");   
		//传递默认打开路径 （方法二，前提是lpData提前设置好）
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		break;

	case BFFM_SELCHANGED:    //选择路径变化，
		{
			TCHAR curr[MAX_PATH];   
			SHGetPathFromIDList((LPCITEMIDLIST)lParam, curr);   
			::SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)curr);   
		}
		break;
	default:
		break;
	}
	return 0;   
}

BOOL SelectFolderEx(CString &strPath, HWND hWnd)
{
	TCHAR szDefaultDir[MAX_PATH], szPath[MAX_PATH];
	_tcscpy(szDefaultDir, strPath);

	BROWSEINFO br = {0};
	br.hwndOwner = hWnd;
	br.pidlRoot = 0;
	br.pszDisplayName = 0;
	br.lpszTitle = BkString::Get(IDS_VULFIX_5103);
	br.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_STATUSTEXT;
	br.lpfn = BrowseCallbackProc;        //设置CALLBACK函数
	br.iImage = 0;
	br.lParam = long(&szDefaultDir);    //设置默认路径

	ITEMIDLIST* pItem = SHBrowseForFolder(&br);
	if(pItem && SHGetPathFromIDList(pItem,szPath))
	{
		strPath = szPath;
		return TRUE;
	}
	return FALSE;
}

T_VulListItemData *CreateListItem( LPTUpdateItem pItem )
{
	T_VulListItemData *pVulItem = new T_VulListItemData;
	pVulItem->nType = pItem->m_nType;
	pVulItem->nID = pItem->nID;
	pVulItem->nWarnLevel = pItem->isExclusive ? 0 : pItem->nWarnLevel;
	pVulItem->dwFlags = pItem->dwVFlags & (DWORD)VFLAG_SERVICE_PATCH;
	
	pVulItem->nDownloaded = 0;
	pVulItem->nFileSize = pItem->detail.nPatchSize;
	
	pVulItem->strName = pItem->strName;
	pVulItem->strMSID = pItem->strMsid;
	pVulItem->strDesc = pItem->strDescription;
	pVulItem->strPubDate = pItem->strPubdate;
	pVulItem->strSuggest = pItem->strSuggestion;
	pVulItem->strAffects = pItem->strAffects;
	// pVulItem->strSideAffectes = pItem->strs
	pVulItem->strWebPage = pItem->strWebpage;
	pVulItem->strDownloadUrl = pItem->detail.strDownurl;
	return pVulItem;
}

T_VulListItemData *CreateListItem( TItemFixed* pItem )
{
	T_VulListItemData *pVulItem = new T_VulListItemData;
	pVulItem->nType = VTYPE_WINDOWS;
	pVulItem->nID = pItem->nID;
	if(pItem->strName.IsEmpty())
		pVulItem->strName.Format( BkString::Get(IDS_VULFIX_5165), pItem->nID );
	else
		pVulItem->strName = pItem->strName;

	pVulItem->strWebPage = pItem->strWebPage;
	pVulItem->strPubDate = pItem->strInstallDate;
	return pVulItem;
}

T_VulListItemData *CreateListItem( TReplacedUpdate *pItem )
{
	T_VulListItemData *pVulItem = new T_VulListItemData;
	pVulItem->nType = VTYPE_IGNORED;
	pVulItem->nID = pItem->nKBID;
	pVulItem->nWarnLevel = -1;
	
	pVulItem->nDownloaded = 0;
	pVulItem->nFileSize = 0;
	
	pVulItem->strName.Format(BkString::Get(IDS_VULFIX_5105), pItem->nKBID2);
	pVulItem->strPubDate = pItem->strPubdate;
	// pVulItem->strDesc = _T("");
	return pVulItem;
}

T_VulListItemData *CreateListItem( TVulSoft *pItem )
{
	T_VulListItemData *pVulItem = new T_VulListItemData;
	pVulItem->nType = VTYPE_SOFTLEAK;
	pVulItem->nID = pItem->nID;
	pVulItem->nWarnLevel = pItem->nLevel;
	
	pVulItem->nDownloaded = 0;
	pVulItem->nFileSize = pItem->matchedItem.nSize;	
	pVulItem->strName = pItem->matchedItem.strName;
	pVulItem->strPubDate = pItem->strPubdate;
	pVulItem->strAffects = pItem->strAffects;
	pVulItem->strSideAffectes = pItem->strSideaffect;
	pVulItem->strWebPage = pItem->matchedItem.strWebpage;
	
	pVulItem->strDesc = pItem->strDescription;
	pVulItem->strDetailDescription = pItem->strDetailDescription;
	
	pVulItem->state = pItem->state;
	pVulItem->nAction = pItem->matchedItem.nAction;
	pVulItem->strDownloadUrl = pItem->matchedItem.strDownUrl;
	return pVulItem;
}

int AppendItem2List( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strTitle;
	FormatKBString(pVulItem->nID, strTitle);
	
	LPCTSTR lpszWarnDesc = GetLevelDesc(pVulItem->nWarnLevel);
	
	int nItem = listCtrl.Append(lpszWarnDesc, pVulItem->nWarnLevel>=0 ? LISTITEM_CHECKBOX:0);
	listCtrl.AppendSubItem(nItem, strTitle);
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
	listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
	listCtrl.AppendSubItem(
        nItem, 
        pVulItem->nWarnLevel == -1 ? BkString::Get(IDS_VULFIX_5106)
                                   : (pVulItem->nWarnLevel > 0 ? BkString::Get(IDS_VULFIX_5107)
                                                               : BkString::Get(IDS_VULFIX_5108)) );
	
	if(pVulItem->nWarnLevel>3)
		listCtrl.SetSubItemColor(nItem, 0, red, false);
	
	/*listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);*/
	listCtrl.SetItemData(nItem, (DWORD)pVulItem);
	return nItem;
}

LPCTSTR GetLevelDesc( int nLevel )
{
	static LPCTSTR descs[] = {
		BkString::Get(IDS_VULFIX_5109), 
        BkString::Get(IDS_VULFIX_5110), 
        BkString::Get(IDS_VULFIX_5111), 
        BkString::Get(IDS_VULFIX_5112), 
        BkString::Get(IDS_VULFIX_5113), 
        BkString::Get(IDS_VULFIX_5114), 
	};
	if(nLevel==-1)
		return BkString::Get(IDS_VULFIX_5115);

	ATLASSERT(nLevel>=0 && nLevel<=5);
	if(nLevel<0 || nLevel>5)
		nLevel = 0;
	return descs[nLevel];	
}

bool GetListCheckedItems(CListViewCtrlEx &listCtrl, CSimpleArray<int> &arr, BOOL *pIsRadio)
{
	if(pIsRadio) *pIsRadio = FALSE;
	for(int i=0; i<listCtrl.GetItemCount(); ++i)
	{
		if( listCtrl.GetCheckState(i, pIsRadio) )
		{
			T_VulListItemData *pItem = (T_VulListItemData*) listCtrl.GetItemData( i );
			if(pItem)
				arr.Add( pItem->nID );
		}
	}
	return arr.GetSize()>0;
}

void ListDeleteItems(CListViewCtrlEx &listCtrl, CSimpleArray<int> &arr)
{
	for(int i=listCtrl.GetItemCount()-1; i>=0; --i)
	{
		T_VulListItemData *pItem = (T_VulListItemData*)listCtrl.GetItemData(i);	
		if(pItem && arr.Find(pItem->nID)!=-1)
		{
			listCtrl.DeleteItem( i );
		}
	}
}

bool SoftComRepaired( LPTVulSoft pItem )
{
	return pItem->state.comState==COM_ALL_DISABLED;
}

int RepairCOMVul(const CSimpleArray<LPTVulSoft>& arr)
{
	int count = 0;
	for(int i=0; i<arr.GetSize(); ++i)
	{
		LPTVulSoft pItem = arr[i];
		int state = GetSoftItemState(pItem->state, 0 != pItem->nDisableCom);
		if(state==VUL_DISABLE_COM)
		{
			++count;
			theEngine->m_pVulScan->EnableVulCOM( pItem->nID, FALSE );
		}
	}
	return count;
}

int RepairCOMVul(CListViewCtrlEx &listCtrl, bool bRepairAll)
{
	int count = 0;
	for(int i=0; i<listCtrl.GetItemCount(); ++i)
	{
		if(bRepairAll || listCtrl.GetCheckState(i))
		{
			T_VulListItemData *pItem = (T_VulListItemData*) listCtrl.GetItemData( i );
			ATLASSERT(pItem);
			if(pItem && pItem->nType==VTYPE_SOFTLEAK)
			{
				int state = GetSoftItemState( pItem );
				if(state==VUL_DISABLE_COM)
				{
					++count;
					theEngine->m_pVulScan->EnableVulCOM( pItem->nID, FALSE );
				}
			}
		}
	}
	return count;
}


int GetSoftItemState( T_VulListItemData * pItem )
{
	return GetSoftItemState(pItem->state, pItem->nAction==SOFTACTION_DISABLECOM);
}

int GetSoftItemState(const T_VulState &vulstate, bool bDisableCom )
{
	int state = VUL_UPDATE;
	if( vulstate.bFounded && bDisableCom )
		// 有可禁用的COM 组件, 且方法也是可以禁用 
	{
		if(vulstate.bMatched)
		{
			if(vulstate.comState==COM_ALL_DISABLED)
				state = VUL_COM_DISABLED;
			else
				state = VUL_DISABLE_COM;
		}
		else 
		{
			if(vulstate.comState==COM_ALL_DISABLED || vulstate.comState==COM_PART_DISABLED)
			{
				state = VUL_COM_DISABLED;
			}
		}
	}
	else
	{
		state = VUL_UPDATE;
	}
	return state;
}

void SetRelateInfo(CRichEditCtrlEx &ctlRichEdit, T_VulListItemData *pItemData, BOOL bOfferOperate)
{
	// Set Info Page Data 

	ctlRichEdit.ClearText();

	if(pItemData->nType==VTYPE_IGNORED)
	{
		CString str;
		str.Format(BkString::Get(IDS_VULFIX_5116), pItemData->strName);
		ctlRichEdit.AddText( str );
	}
	else
	{
		CString strText;

		// 软件的话, 提供操作 
		if(pItemData->nType==VTYPE_SOFTLEAK)
		{
			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5117), true);
			ctlRichEdit.AddText( pItemData->strName );
			ctlRichEdit.AddNewLine();
			
			if(bOfferOperate)
			{
				ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5118), true);
				int state = GetSoftItemState( pItemData );
				ctlRichEdit.AddText( SoftVulStatusOP[state] );
				ctlRichEdit.AddNewLine();
			}
			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5119), true);
			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5120));
			
			ctlRichEdit.AddNewLine();
			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5121), true);
			ctlRichEdit.AddText(pItemData->strDesc);
		}
		else
		{
			FormatKBString(pItemData->nID, strText);
			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5122), true);
			ctlRichEdit.AddText(strText);
			ctlRichEdit.AddNewLine();

			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5123), true);
			ctlRichEdit.AddText(pItemData->strMSID);
			ctlRichEdit.AddNewLine();

			FormatSizeString(pItemData->nFileSize, strText);
			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5124), true);
			ctlRichEdit.AddText(strText);
			ctlRichEdit.AddNewLine();

// 			ctlRichEdit.AddText(_T("发布日期："), true);
// 			ctlRichEdit.AddText(pItemData->strPubDate);
// 			ctlRichEdit.AddNewLine();

			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5125), true);
//			ctlRichEdit.AddNewLine();
			COLORREF	clr=RGB(255,0,0);
			ctlRichEdit.AddText(pItemData->strAffects,FALSE,&clr);
			ctlRichEdit.AddNewLine();
			
			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5126), true);
			if ( !pItemData->isIgnored && pItemData->nWarnLevel > 0 )
				ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5120));
			else
				ctlRichEdit.AddText(pItemData->strSuggest);
			ctlRichEdit.AddNewLine();

			ctlRichEdit.AddText(BkString::Get(IDS_VULFIX_5127), true);
//			ctlRichEdit.AddNewLine();
			ctlRichEdit.AddText(pItemData->strDesc);			
		}
		
		ctlRichEdit.AddNewLine();
		ctlRichEdit.AddNewLine();
		ctlRichEdit.AddText(_T("  "));
		ctlRichEdit.AddLink(BkString::Get(IDS_VULFIX_5128));

		if( !pItemData->strDownloadUrl.IsEmpty() )
		{
			ctlRichEdit.AddText(_T("    "));
			ctlRichEdit.AddLink(BkString::Get(IDS_VULFIX_5129));
		}
		ctlRichEdit.AddNewLine();
		ctlRichEdit.AddText(_T("                    "));
		ctlRichEdit.EndLink();
	}
}

void GetDownloadPath(CString &strPath)
{
	BKSafeConfig::Get_Vulfix_DownloadPath( strPath );
}

CString FormatErrorMessage(HRESULT hr)
{
	static LPCTSTR szDescs[] = {
        BkString::Get(IDS_VULFIX_5130),
        BkString::Get(IDS_VULFIX_5131),
        BkString::Get(IDS_VULFIX_5132),
        BkString::Get(IDS_VULFIX_5133),
        BkString::Get(IDS_VULFIX_5134),
        BkString::Get(IDS_VULFIX_5135),
        BkString::Get(IDS_VULFIX_5136),
	};	

	if(hr==S_OK)
	{
		return _T("");
	}
	else if( hr>=KERR_GENERAL && (hr-KERR_GENERAL)<sizeof(szDescs)/sizeof(szDescs[0]) )
	{
		return szDescs[ hr-KERR_GENERAL ];
	}
	else if( hr==E_POINTER )
	{
		return BkString::Get(IDS_VULFIX_5137);
	}
	else
	{
		CString str;
		str.Format( BkString::Get(IDS_VULFIX_5138), hr );
		return str;
	}
}

void SetWindowRunBackGround(HWND hWnd, BOOL bBackground)
{
#if 0
	DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
	if(bBackground)
	{
		//dwExStyle |= WS_EX_TOOLWINDOW;
		dwExStyle &= ~WS_EX_APPWINDOW;
	}
	else
	{
		//dwExStyle &= (~ WS_EX_TOOLWINDOW);
		dwExStyle |= WS_EX_APPWINDOW;
	}
	::SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);
#endif
	::ShowWindow( hWnd, bBackground ? SW_HIDE:SW_SHOW );
}

BOOL IsWindowRunBackGround(HWND hWnd)
{
#if 0 
	DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
	return !(dwExStyle & WS_EX_APPWINDOW) && !::IsWindowVisible(hWnd);
#endif 
	
	return !::IsWindowVisible(hWnd);
}

unsigned short GetPercent( INT p, INT total )
{
	ATLASSERT(total>0 && p>=0 );
	if( total<=0 )
		return 100;
	if(p<0)
		return 0;
	if(p>=total)
		return 100;
	INT64 n1 = p, n2 = total;
	n1 *= 100;
	return n1/n2;
}

BOOL IsOfficeProcessExists(CString &strProductName) 
{
	static struct{
		LPCTSTR szProcessName, szProductName;
	}
	szOfficeProducts [] = {
		{_T("MSACCESS.EXE"),	_T("Microsoft Office Access") },
		{_T("MSEXCEL.EXE"),		_T("Microsoft Office Excel") },
		{_T("EXCEL.EXE"),		_T("Microsoft Office Excel") },

		{_T("POWERPNT.EXE"),	_T("Microsoft Office PowerPoint") },
		{_T("WINWORD.EXE"),		_T("Microsoft Office Word") },
		{_T("OUTLOOK.EXE"),		_T("Microsoft Office Outlook") }, 

		{_T("INFOPATH.EXE"),	_T("Microsoft Office InfoPath") },
		{_T("MSPUB.EXE"),		_T("Microsoft Office Publisher") },
		{_T("VISIO.EXE"),		_T("Microsoft Office Visio") },
	};

	HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hSnapshot == INVALID_HANDLE_VALUE)
		return FALSE;
	
	std::vector<CString> arrFoundedProcess;

	PROCESSENTRY32 pe;
	ZeroMemory(&pe,sizeof(pe));
	pe.dwSize=sizeof(pe);
	
	BOOL bFounded = FALSE;
	BOOL bMore=Process32First(hSnapshot,&pe);
	while(bMore)
	{
		//_tcprintf(_T("ID:0x%08x %s\r\n"), pe.th32ProcessID, pe.szExeFile);
		for(int i=0; i<sizeof(szOfficeProducts)/sizeof(szOfficeProducts[0]); ++i)
		{
			if(_tcsicmp(szOfficeProducts[i].szProcessName, pe.szExeFile)==0)
			{
				CString str = pe.szExeFile;
				str.MakeLower();
				if(std::find(arrFoundedProcess.begin(), arrFoundedProcess.end(), str)==arrFoundedProcess.end())
				{
					if(arrFoundedProcess.empty())
						strProductName = szOfficeProducts[i].szProductName;
					else
						strProductName.AppendFormat(_T("\r\n%s"), szOfficeProducts[i].szProductName);
					arrFoundedProcess.push_back(str);
				}
				bFounded = TRUE;
				break;
			}
		}
		bMore=Process32Next(hSnapshot, &pe);
	}
	CloseHandle(hSnapshot);
	return bFounded;
}
