#pragma once
#include "Vulfix/Vulfix.h"
#include "vulfix/BeikeUtils.h"

extern const COLORREF black, red, green, blue, yellow, brown;

struct T_VulListItemData 
{
public:
	T_VulListItemData() 
		: nType(-1), nID(-1), nWarnLevel(-1), nDownloaded(0), nFileSize(0), isExclusive(false), isIgnored(false), dwFlags(0)
	{}

	bool isCOMAction()
	{
		ATLASSERT(nType==VTYPE_SOFTLEAK);
		return FALSE;
	}

	int nType;		// office, windows, soft  
	int nID;		
	int nWarnLevel;	// disable.com 
	DWORD dwFlags;

//	int nDisableCom;
	
	int nDownloaded;
	int nFileSize;
	
	CString strName, strDesc, strPubDate, strAffects, strSideAffectes, strMSID, strSuggest, strDetailDescription;
	CString strWebPage;	// Relate web page 
	CString strDownloadUrl;	// œ¬‘ÿµÿ÷∑
	
	// 
	T_VulState state;		
	bool isExclusive;
	bool isIgnored;

	INT nAction;	// Action of soft 
};

class CListViewCtrlEx;
class CRichEditCtrlEx;

//void ShutDownComputer(BOOL toReboot=FALSE);
void FormatSizeString(INT64 nFileSize, CString &str);

BOOL SelectFolder(CString &strPath, HWND hWnd=NULL);
BOOL SelectFolderEx(CString &strPath, HWND hWnd=NULL);

typedef BOOL (*FolderFilter)(LPCTSTR szFolder, LPCTSTR szFilename);
INT64 GetFolderSize(LPCTSTR szFolder, BOOL bIncludeChild=FALSE, FolderFilter fn=NULL);
BOOL IsBkDownloadFile(LPCTSTR szFolder, LPCTSTR szFilename);

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
	listCtrl.Reset();
}
T_VulListItemData *CreateListItem( LPTUpdateItem pItem );
T_VulListItemData *CreateListItem( TItemFixed* pItem );
T_VulListItemData *CreateListItem( TReplacedUpdate *pItem );
T_VulListItemData *CreateListItem( TVulSoft *pItem );
int AppendItem2List( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem );

LPCTSTR GetLevelDesc(int nLevel);
bool GetListCheckedItems(CListViewCtrlEx &listCtrl, CSimpleArray<int> &arr, BOOL *pIsRadio=NULL);
void ListDeleteItems(CListViewCtrlEx &listCtrl, CSimpleArray<int> &arr);
bool SoftComRepaired( LPTVulSoft pItem );


BOOL IsOfficeProcessExists(CString &strProductName);

template<typename T, typename Func>
int CountItems(const CSimpleArray<T>& arr, Func fn)
{
	int count = 0;
	for(int i=0; i<arr.GetSize(); ++i)
	{
		if( fn(arr[i]) )
			++count;
	}
	return count;
}

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
void GetDownloadPath(CString &strPath);


template<typename T>
class CIconAnimate
{
public:
	CIconAnimate() : m_nTimer(0), m_uItemID(0), m_nProgressID(0)
	{ }
	
	void StartIconAnimate(int uItemID, int nProgressID=0, int nMaxPos=100)
	{
		m_uItemID = uItemID;
		m_nProgressID = nProgressID;
		m_dwPos = 0;
		m_dwMaxPos = nMaxPos;
		if(!m_nTimer)
		{
			T *dlg = static_cast<T*>(this);
			m_nTimer = dlg->SetTimer(TIMER_RUNNING, 200, NULL);
		}
	}
	
	void StopProgressAnimate()
	{
		m_nProgressID = 0;
	}
	
	void StopIconAnimate()
	{
		if(m_nTimer)
		{
			T *dlg = static_cast<T*>(this);
			dlg->KillTimer(TIMER_RUNNING);
			m_nTimer = 0;
		}
		m_nProgressID = 0;
	}

	void OnTimer( UINT_PTR nIDEvent )
	{
		if(nIDEvent==TIMER_RUNNING && m_uItemID>0)
		{
			static int nSubImage = 0;
            if (m_nProgressID == IDC_PROGRESS_VULFIX_SCANNING)
			    nSubImage = ++nSubImage % 10;
            else
                nSubImage = ++nSubImage % 4;
			
			T *dlg = static_cast<T*>(this);
			dlg->SetItemIntAttribute(m_uItemID, "sub", nSubImage);
			
			if( m_nProgressID>0 )
			{
				INT nStep = m_dwMaxPos/100;
				if(nStep<1)
					nStep = 1;
				m_dwPos += nStep;
				dlg->SetItemDWordAttribute(m_nProgressID, "value", m_dwPos);
				if(m_dwPos>=m_dwMaxPos)
					m_dwPos = 0;
			}
		}
		return;
	}

private:
	UINT_PTR m_nTimer;
	int m_uItemID, m_nProgressID;
	int m_dwPos, m_dwMaxPos;
};

CString FormatErrorMessage(HRESULT hr);
void SetWindowRunBackGround(HWND hWnd, BOOL bBackground);
BOOL IsWindowRunBackGround(HWND hWnd);
unsigned short GetPercent( INT p, INT total );
