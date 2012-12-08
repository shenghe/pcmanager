#include "StdAfx.h"
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"
#include "DlgDisalbeComPrompt.h"
#include "RichEditCtrlEx.h"
#include "BeikeVulfixEngine.h"

const COLORREF black = RGB(0,0,0);
const COLORREF red = RGB(255,0,0);
const COLORREF green = RGB(0,255,0);
const COLORREF blue = RGB(0,0,255);


void ShutDownComputer_2(BOOL toReboot)
{
#ifdef _DEBUG
	::MessageBox(NULL, _T(""), _T("ShutDownComputer"), MB_OK);
#else
	UINT flags = toReboot ? EWX_REBOOT : EWX_SHUTDOWN;
	::ExitWindowsEx( flags | EWX_FORCE, 0);
#endif
}

void FormatSizeString(int nFileSize, CString &str)
{
	const int SIZE_KILO = 1024;
	const int SIZE_MEGA = SIZE_KILO * SIZE_KILO;
	const int SIZE_GIGA = SIZE_MEGA * SIZE_KILO;
	
	if(nFileSize==0)
		str = _T("");
	else if(nFileSize<SIZE_KILO)
		str.Format(_T("%dB"), nFileSize);
	else if(nFileSize<SIZE_MEGA)
		str.Format(_T("%.2fKB"), nFileSize*1.0/SIZE_KILO );
	else if(nFileSize<SIZE_GIGA)
		str.Format(_T("%.2fMB"), nFileSize*1.0/SIZE_MEGA );
	else
		str.Format(_T("%.2fGB"), nFileSize*1.0/SIZE_GIGA );
}

BOOL SelectFolder(CString &strPath)
{
	BROWSEINFO bInfo = {0};
	bInfo.hwndOwner = NULL;
	bInfo.lpszTitle = _T("请选择路径: ");
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST lpDlist;
	lpDlist = SHBrowseForFolder(&bInfo);
	if(lpDlist==NULL)
		return FALSE;

	SHGetPathFromIDList(lpDlist, strPath.GetBuffer(MAX_PATH) );
	strPath.ReleaseBuffer(MAX_PATH);
	return TRUE;
}

T_VulListItemData *CreateListItem( LPTUpdateItem pItem )
{
	T_VulListItemData *pVulItem = new T_VulListItemData;
	pVulItem->nType = pItem->m_nType;
	pVulItem->nID = pItem->nID;
	pVulItem->nWarnLevel = pItem->nWarnLevel;
	
	pVulItem->nDownloaded = 0;
	pVulItem->nFileSize = pItem->detail.nPatchSize;
	
	pVulItem->strName = pItem->strName;
	pVulItem->strDesc = pItem->strDescription;
	pVulItem->strPubDate = pItem->strPubdate;
	pVulItem->strAffects = pItem->strSuggestion;
	// pVulItem->strSideAffectes = pItem->strs
	pVulItem->strWebPage = pItem->strWebpage;
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
	
	pVulItem->strName.Format(_T("该补丁已被最新补丁KB%d替换"), pItem->nKBID2);
	pVulItem->strPubDate = pItem->strPubdate;
	// pVulItem->strDesc = _T("");
	return pVulItem;
}

T_VulListItemData *CreateListItem( TVulSoft *pItem )
{
	T_VulListItemData *pVulItem = new T_VulListItemData;
	pVulItem->nType = VTYPE_SOFTLEAK;
	pVulItem->nID = pItem->nID;
	pVulItem->nWarnLevel = pItem->nDisableCom;	// DISABLE com 
	
	pVulItem->nDownloaded = 0;
	pVulItem->nFileSize = 0;
	
	pVulItem->strName = pItem->matchedItem.strName;
	pVulItem->strDesc = pItem->strDescription;
	pVulItem->strPubDate = pItem->strPubdate;
	pVulItem->strAffects = pItem->strAffects;
	pVulItem->strSideAffectes = pItem->strSideaffect;
	pVulItem->strWebPage = pItem->matchedItem.strWebpage;
	
	pVulItem->state = pItem->state;
	return pVulItem;
}

int AppendItem2List( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strTitle;
	strTitle.Format(_T("KB%d"), pVulItem->nID);
	
	LPCTSTR lpszWarnDesc = GetLevelDesc(pVulItem->nWarnLevel);
	
	int nItem = listCtrl.Append(lpszWarnDesc, pVulItem->nWarnLevel>=0);
	listCtrl.AppendSubItem(nItem, strTitle);
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
	listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
	listCtrl.AppendSubItem(nItem, pVulItem->nWarnLevel==-1 ? _T("无需修复") : (pVulItem->nWarnLevel>0 ? _T("未修复") : _T("可不修复")) );
	
	if(pVulItem->nWarnLevel>3)
		listCtrl.SetSubItemColor(nItem, 0, red, false);
	
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}

LPCTSTR GetLevelDesc( int nLevel )
{
	static LPCTSTR descs[] = {
		_T(""), _T("低"), _T("中等"), _T("重要"), _T("严重"), _T("紧急"), 
	};
	if(nLevel==-1)
		return _T("已过期");

	ATLASSERT(nLevel>=0 && nLevel<=5);
	if(nLevel<0 || nLevel>5)
		nLevel = 0;
	return descs[nLevel];	
}

void SetListCheckedAll(CListViewCtrlEx &listCtrl,bool bCheck)
{
	for(int i=0; i<listCtrl.GetItemCount(); ++i)
	{
		listCtrl.SetCheckState(i, bCheck);
	}
}

bool GetListCheckedItems(CListViewCtrlEx &listCtrl, CSimpleArray<int> &arr)
{
	for(int i=0; i<listCtrl.GetItemCount(); ++i)
	{
		if( listCtrl.GetCheckState(i) )
		{
			T_VulListItemData *pItem = (T_VulListItemData*) listCtrl.GetItemData( i );
			if(pItem)
				arr.Add( pItem->nID );
		}
	}
	if(arr.GetSize()==0)
	{
		::MessageBox(NULL, _T("没有选择内容"), NULL, MB_OK);
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
			delete pItem;
		}
	}
}

int RepairCOMVul(const CSimpleArray<LPTVulSoft>& arr)
{
	int count = 0;
	for(int i=0; i<arr.GetSize(); ++i)
	{
		LPTVulSoft pItem = arr[i];
		int state = GetSoftItemState(pItem->state, pItem->nDisableCom);
		if(state==VUL_DISABLE_COM)
		{
			++count;
			theEngine->m_pSoftVulScan->EnableVulCOM( pItem->nID, FALSE );
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
					theEngine->m_pSoftVulScan->EnableVulCOM( pItem->nID, FALSE );
				}
			}
		}
	}
	return count;
}

int GetSoftItemState( T_VulListItemData * pItem )
{
	return GetSoftItemState(pItem->state, pItem->nWarnLevel);
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
		str.Format(_T("%s,已经无效, 无需修复"), pItemData->strName);
		ctlRichEdit.AddText( str );
	}
	else
	{
		CString strFileSize;
		FormatSizeString(pItemData->nFileSize, strFileSize);
		
		ctlRichEdit.AddText(_T("补丁名称: "), true);
		ctlRichEdit.AddText(pItemData->strName);
		ctlRichEdit.AddNewLine();
		
		// 软件的话, 提供操作 
		if(pItemData->nType==VTYPE_SOFTLEAK && bOfferOperate)
		{
			ctlRichEdit.AddText(_T("操作: "), true);
			//
			int state = GetSoftItemState( pItemData );
			ctlRichEdit.AddLink( SoftVulStatusOP[state] );
		}
		else
		{
			ctlRichEdit.AddText(_T("补丁大小: "), true);
			ctlRichEdit.AddText(strFileSize);
		}
		ctlRichEdit.AddNewLine();

		ctlRichEdit.AddText(_T("发布日期: "), true);
		ctlRichEdit.AddText(pItemData->strPubDate);
		ctlRichEdit.AddNewLine();

		ctlRichEdit.AddText(_T("漏洞影响: "), true);
		ctlRichEdit.AddText(pItemData->strAffects);
		ctlRichEdit.AddNewLine();

		ctlRichEdit.AddText(_T("漏洞描述: "), true);
		ctlRichEdit.AddText(pItemData->strDesc);

		ctlRichEdit.AddNewLine();
		ctlRichEdit.AddNewLine();
		ctlRichEdit.AddText(_T("     "));
		ctlRichEdit.AddLink(_T("了解更多"));
		ctlRichEdit.AddNewLine();
		ctlRichEdit.AddText(_T(""));
	}
}

BOOL CreateDir(LPCTSTR szPath, BOOL bForce)
{
	if(!bForce)
	{
		return CreateDirectory(szPath, NULL);
	}
	else
	{
		TCHAR dir[MAX_PATH] = {0};

		LPCTSTR pos = szPath;
		LPCTSTR slash = NULL;
		while(*pos)
		{
			slash =_tcschr(pos, _T('\\'));
			if(slash)
			{
				_tcsncpy(dir, szPath, slash-szPath);
				pos = slash;
			}
			else if( _tcslen(pos)>0 )	// c:\\asdf\asdfasdf
			{
				_tcscpy(dir, szPath);
				pos = szPath + _tcslen(szPath);
			}
			else
				break; 

			if( _tcslen(dir) == 2 && dir[1] == _T(':') )
			{
			}
			else
			{
				if( !CreateDirectory(dir, NULL) )
				{
					if(ERROR_ALREADY_EXISTS != GetLastError())
						return FALSE;
				}
			}

			while(*pos==_T('\\'))
				++pos;
		}
		return TRUE;
	}
}

CString GetIniFilename()
{
	TCHAR szPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szPath, MAX_PATH);
	PathAppend(szPath, _T("..\\vulfix.ini"));
	return szPath;
}

void GetDownloadPath(CString &strPath)
{
	TCHAR szPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szPath, MAX_PATH);
	PathAppend(szPath, _T("..\\hotfix"));
	
	GetPrivateProfileString(_T("Vulfix"), _T("downpath"), szPath, strPath.GetBuffer(MAX_PATH), MAX_PATH, GetIniFilename());
	strPath.ReleaseBuffer(MAX_PATH);
}

void SaveDownloadPath( LPCTSTR szDownloadPath )
{
	WritePrivateProfileString(_T("Vulfix"), _T("downpath"), szDownloadPath, GetIniFilename() );
}