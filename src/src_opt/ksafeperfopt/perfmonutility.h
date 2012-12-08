#pragma once

#include "runoptimize/processmon.h"
#include "perfmonlistitemdata.h"
#include "appwndlistitemdata.h"
//创建于现实列表相关的缓存数据
class CPerfMonitorCacheData
{
public:
	CPerfMonitorCacheData(){}
	CPerfMonitorCacheData(KProcInfoItem procPerfData, DWORD dwID)
	{
		m_procPerfData = procPerfData;
		m_dwCurSelID = dwID;
		m_bRemember = FALSE;
	}
	~CPerfMonitorCacheData(){}
	KProcInfoItem& GetProcNetData()
	{
		return m_procPerfData;
	}

	CPerfMonitorCacheData(CPerfMonitorCacheData& perfData)
	{
		memcpy(this, &perfData, sizeof(CPerfMonitorCacheData));
	}

	CPerfMonitorCacheData&  operator = (CPerfMonitorCacheData& perfData)
	{
		this->m_procPerfData = perfData.GetProcNetData();
		m_bRemember = perfData.IsRemember();

		return *this;
	}
	DWORD	SetIndexID(DWORD dwID){DWORD dw = m_dwCurSelID; m_dwCurSelID = dwID;return dw;}
	DWORD   GetIndexID(){return m_dwCurSelID;}
	BOOL	IsRemember(){return m_bRemember;}
	BOOL    SetRemember(BOOL bSet){BOOL b = m_bRemember; m_bRemember = bSet; return b;}
private:
	KProcInfoItem	m_procPerfData;
	DWORD			m_dwCurSelID;
	BOOL			m_bRemember;
};

//创建于listbox相关的一些全局操作或者是数据状态
class CListBoxItemData
{
public:
	~CListBoxItemData(){
		if (NULL != m_hDefaultIcon)
			::DestroyIcon(m_hDefaultIcon);
		m_mapProcInfoFromLib.RemoveAll();
	}
	static CListBoxItemData* GetDataPtr()
	{
		static CListBoxItemData data;
		return &data;
	}
	BKSafeLog::CBKSafeLog GetLogModule(){return m_NetMonitorLog;};
	CAtlArray<UINT>& GetStateArray(){return m_arrColumState;}
	BOOL IsNeedSort(int& nColum, int &nState)
	{
		nColum = -1, nState = -1;
		for (int i = 0; i < m_arrColumState.GetCount(); i++)
		{
			if (-1 != m_arrColumState[i])
			{
				nColum = i;
				nState = m_arrColumState[i];
				break;
			}
		}

		if (-1 == nColum ||
			-1 == nState)
			return FALSE;

		return TRUE;
	}

	DWORD	GetTmpProcID(){return m_nProcID;}
	DWORD	SetTmpProcID(const DWORD dwID){ DWORD dw = m_nProcID; m_nProcID = dwID; return dw;}
	void	SetCurSelData(KPerfMonListItemData& Data){m_nSelListBoxItemData = Data;}
	KPerfMonListItemData& GetCurSelData(){return m_nSelListBoxItemData;}
	int		GetMaxHeightSel(){return m_nMaxHeightSel;}
	int		SetMaxHeightSel(int nSel){int sel = m_nMaxHeightSel; m_nMaxHeightSel = nSel; return sel;}
	HICON	GetDefaultIcon(){ return m_hDefaultIcon;}
	KAppWndListItemData GetCurSelAppWndData(){return m_AppWndData;}
	void	SetCurSelAppWndData(KAppWndListItemData Data){m_AppWndData = Data;}
	//查询进程信息接口
	int		QueryProcInfo(LPCTSTR pszFullPath, KSProcessInfo& ProcInfo)
	{
		int nRet = -1;
		do 
		{
			KSProcessInfo* pInfo = NULL;
			if (NULL == pszFullPath || NULL == m_pKProcQuery)
				break;

			if (NULL != m_mapProcInfoFromLib.Lookup(pszFullPath))
			{
				ProcInfo = m_mapProcInfoFromLib[pszFullPath];
				nRet = 0;
				break;
			}

			m_pKProcQuery->GetInfoByPath(pszFullPath, &pInfo);

			if (NULL == pInfo)
				break;

			ProcInfo = *pInfo;
			m_mapProcInfoFromLib[pszFullPath] = ProcInfo;

			m_pKProcQuery->DeleteInfo(pInfo);
			nRet = 0;
		} while (FALSE);


		return nRet;
	}

	//获取图标
	HICON GetIcon(const CString& strPath)
	{
		CAtlMap<CString, HICON>::CPair* ptr = m_mapLargeIcon.Lookup(strPath);
		if (NULL != ptr)
			return ptr->m_value;

		if (TRUE == PathFileExists(strPath))
		{
			SHFILEINFO fileInfo = {0};
			if (::SHGetFileInfo(strPath, 0, &fileInfo, sizeof(fileInfo), SHGFI_ICON | SHGFI_LARGEICON |SHGFI_DISPLAYNAME) &&
				NULL != fileInfo.hIcon)
			{
				m_mapLargeIcon[strPath] = fileInfo.hIcon;
				return fileInfo.hIcon;
			}
		}
		return NULL;
	}

	HICON GetSmallIcon(const CString& strPath)
	{
		CAtlMap<CString, HICON>::CPair* ptr = m_mapSmallIcon.Lookup(strPath);
		if (NULL != ptr)
			return ptr->m_value;

		if (TRUE == PathFileExists(strPath))
		{
			SHFILEINFO fileInfo = {0};
			if (::SHGetFileInfo(strPath, 0, &fileInfo, sizeof(fileInfo), SHGFI_ICON | SHGFI_SMALLICON |SHGFI_DISPLAYNAME) &&
				NULL != fileInfo.hIcon)
			{
				m_mapSmallIcon[strPath] = fileInfo.hIcon;
				return fileInfo.hIcon;
			}
		}
		return NULL;
	}

	HICON GetWndIcon(KAppWndListItemData ItemData)
	{
		HICON hIcon = NULL;
		if (ItemData.GetItemState() == 1)//对于已卡死的窗口获取他的图标会失败，就直接获取exe的图标
		{
			hIcon = GetSmallIcon(ItemData.GetItemExePath());
			return hIcon;
		}
// 		CString strKey;
// 		strKey.Format(_T("%d%s"), ItemData.GetItemHwnd(), ItemData.GetItemWndTitle());
// 		CAtlMap<CString, HICON>::CPair* ptr = m_mapWndIcon.Lookup(strKey);
// 		if (NULL != ptr)
// 			return ptr->m_value;
		hIcon = ItemData.GetItemWndIcon();
		if (hIcon == NULL)
			hIcon = GetSmallIcon(ItemData.GetItemExePath());

		return hIcon;
	}
private:
	CAtlArray<UINT>				m_arrColumState;//用来判读是否排序的时候使用需要排序0，升序1，降序
	BKSafeLog::CBKSafeLog		m_NetMonitorLog;
	DWORD						m_nProcID;
	KPerfMonListItemData		m_nSelListBoxItemData;
	int							m_nMaxHeightSel;
	IProcessLibQuery*			m_pKProcQuery;
	CString						m_strModulePath;
	HICON						m_hDefaultIcon;
	CAtlMap<CString, KSProcessInfo>				m_mapProcInfoFromLib;
	CAtlMap<CString, HICON>						m_mapLargeIcon;
	CAtlMap<CString, HICON>						m_mapSmallIcon;
	CAtlMap<CString, HICON>						m_mapWndIcon;
	KAppWndListItemData							m_AppWndData;

	CListBoxItemData(){
		m_arrColumState.SetCount(5);
		m_arrColumState[0] = -1;
		m_arrColumState[1] = -1;
		m_arrColumState[2] = -1;
		m_arrColumState[3] = -1;
		m_arrColumState[4] = -1;


		m_nProcID				= -1;
		m_nMaxHeightSel			= -1;
		m_pKProcQuery			= NULL;
		m_strModulePath			= L"";
		m_hDefaultIcon			= NULL;

		m_mapProcInfoFromLib.RemoveAll();
		m_mapSmallIcon.RemoveAll();
		m_mapLargeIcon.RemoveAll();
		m_mapWndIcon.RemoveAll();
		CAppPath::Instance().GetLeidianAppPath(m_strModulePath);
		//初始化进程库
		m_pKProcQuery = CProcessLibLoader::GetGlobalPtr(m_strModulePath)->GetQuery();
		m_NetMonitorLog.SetLogForModule(BKSafeLog::enumBKSafeNetMonitorLog);

		CString strIconPath;
		strIconPath.Format(L"%s\\webui\\splock\\floatwnd\\defaulticon.ico", m_strModulePath);
		//Default icon
		m_hDefaultIcon = (HICON)::LoadImage(
			NULL,
			strIconPath,
			IMAGE_ICON,
			0,
			0,
			LR_LOADFROMFILE
			);
	}
};