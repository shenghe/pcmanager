#pragma once
#include "Vulfix/Vulfix.h"

extern const COLORREF black, red, green, blue;

struct T_VulListItemData 
{
	T_VulListItemData() 
		: nType(-1), nID(-1), nWarnLevel(-1), nDownloaded(0), nFileSize(0), isExclusive(false), isIgnored(false)
	{}
	int nType;		// office, windows, soft  
	int nID;		
	int nWarnLevel;	// disable.com 
	
	int nDownloaded;
	int nFileSize;
	
	CString strName, strDesc, strPubDate, strAffects, strSideAffectes;
	CString strWebPage;	// Relate web page 

	// 
	T_VulState state;		
	bool isExclusive;
	bool isIgnored;
};

class CListViewCtrlEx;
class CRichEditCtrlEx;

void ShutDownComputer_2(BOOL toReboot=FALSE);
void FormatSizeString(int nFileSize, CString &str);
BOOL SelectFolder(CString &strPath);

template<typename T>
void ResetListCtrl( T &listCtrl )
{
	if(listCtrl.GetItemCount()==0)
		return; 
	
	for(int i=0; i<listCtrl.GetItemCount(); ++i)
	{
		T_VulListItemData *pItem = (T_VulListItemData *)listCtrl.GetItemData( i );
		if(pItem)
			delete pItem;
	}
	listCtrl.DeleteAllItems();
}
T_VulListItemData *CreateListItem( LPTUpdateItem pItem );
T_VulListItemData *CreateListItem( TReplacedUpdate *pItem );
T_VulListItemData *CreateListItem( TVulSoft *pItem );
int AppendItem2List( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem );

LPCTSTR GetLevelDesc(int nLevel);

// 

template<typename Func>
void SetListCheckedAll(CListViewCtrlEx &listCtrl,Func fn)
{
	for(int i=0; i<listCtrl.GetItemCount(); ++i)
	{
		listCtrl.SetCheckState(i, fn((T_VulListItemData*)listCtrl.GetItemData(i)));
	}
}

void SetListCheckedAll(CListViewCtrlEx &listCtrl,bool bCheck);
bool GetListCheckedItems(CListViewCtrlEx &listCtrl, CSimpleArray<int> &arr);
void ListDeleteItems(CListViewCtrlEx &listCtrl, CSimpleArray<int> &arr);

template<typename T>
void CopySimpleArray(const CSimpleArray<T> &arrFrom, CSimpleArray<T> &arrTo)
{
	for(int i=0; i<arrFrom.GetSize(); ++i)
	{
		arrTo.Add( arrFrom[i] );
	}
}


template<typename T>
void ClearResult(CSimpleArray<T>& arrResult )
{
	for ( int i = 0; i < arrResult.GetSize(); i ++ )
	{
		if( arrResult[i] )
			delete arrResult[i];
	}
	arrResult.RemoveAll();
}


int RepairCOMVul(const CSimpleArray<LPTVulSoft>& arr);
int RepairCOMVul(CListViewCtrlEx &listCtrl, bool bRepairAll);

int GetSoftItemState(const T_VulState &vulstate, bool bDisableCom );
int GetSoftItemState( T_VulListItemData * pItem );


void SetRelateInfo(CRichEditCtrlEx &m_ctlRichEdit, T_VulListItemData *pItemData, BOOL bOfferOperate);



BOOL CreateDir(LPCTSTR szPath, BOOL bForce);

void GetDownloadPath(CString &strPath);
void SaveDownloadPath(LPCTSTR szDownloadPath);