

#pragma once


//创建于现实列表相关的缓存数据
class CNetMonitorCacheData
{
public:
	CNetMonitorCacheData(){}
	CNetMonitorCacheData(KProcFluxItem procNetData, DWORD dwID, int nMonitorState)
	{
		_ProcNetData = procNetData;
		_dwCurSelID = dwID;
		_nMonitorState = nMonitorState;
		_bRemember = FALSE;
	}
	~CNetMonitorCacheData(){}
	KProcFluxItem& GetProcNetData()
	{
		return _ProcNetData;
	}

	CNetMonitorCacheData(CNetMonitorCacheData& NetData)
	{
		memcpy(this, &NetData, sizeof(CNetMonitorCacheData));
	}

	CNetMonitorCacheData&  operator = (CNetMonitorCacheData& Data)
	{
		this->_ProcNetData = Data.GetProcNetData();
		_bRemember = Data.IsRemember();

		return *this;
	}
	DWORD	SetIndexID(DWORD dwID){DWORD dw = _dwCurSelID; _dwCurSelID = dwID;return dw;}
	DWORD   GetIndexID(){return _dwCurSelID;}
	BOOL	MonitorIsOpen(){
		if (0 == _nMonitorState)
			return FALSE;
		else
			return TRUE;
	}
	BOOL	IsRemember(){return _bRemember;}
	BOOL    SetRemember(BOOL bSet){BOOL b = _bRemember; _bRemember = bSet; return b;}
private:
	KProcFluxItem	_ProcNetData;
	DWORD			_dwCurSelID;
	int				_nMonitorState;
	BOOL			_bRemember;
};

//创建于listbox相关的一些全局操作或者是数据状态
class CListBoxData
{
public:
	~CListBoxData(){
		if (NULL != m_hDefaultIcon)
			::DestroyIcon(m_hDefaultIcon);
		m_mapProcInfoFromLib.RemoveAll();
	}
	static CListBoxData* GetDataPtr()
	{
		static CListBoxData data;
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
	void	SetCurSelData(CNetMonitorCacheData& Data){m_nSelNetMonitorData = Data;}
	CNetMonitorCacheData& GetCurSelData(){return m_nSelNetMonitorData;}
	int		GetMaxHeightSel(){return m_nMaxHeightSel;}
	int		SetMaxHeightSel(int nSel){int sel = m_nMaxHeightSel; m_nMaxHeightSel = nSel; return sel;}
	HICON	GetDefaultIcon(){ return m_hDefaultIcon;}
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
	HICON GetIcon(CString strPath)
	{
		CAtlMap<CString, HICON>::CPair* ptr = m_mapIcon.Lookup(strPath);
		if (NULL != ptr)
			return ptr->m_value;

		if (TRUE == PathFileExists(strPath))
		{
			SHFILEINFO fileInfo = {0};
			if (::SHGetFileInfo(strPath, 0, &fileInfo, sizeof(fileInfo), SHGFI_ICON | SHGFI_LARGEICON |SHGFI_DISPLAYNAME) &&
				NULL != fileInfo.hIcon)
			{
				m_mapIcon[strPath] = fileInfo.hIcon;
				return fileInfo.hIcon;
			}
		}
		return NULL;
	}
private:
	CAtlArray<UINT>				m_arrColumState;//用来判读是否排序的时候使用需要排序0，升序1，降序
	BKSafeLog::CBKSafeLog		m_NetMonitorLog;
	DWORD						m_nProcID;
	CNetMonitorCacheData		m_nSelNetMonitorData;
	int							m_nMaxHeightSel;
	IProcessLibQuery*			m_pKProcQuery;
	CString						m_strModulePath;
	HICON						m_hDefaultIcon;
	CAtlMap<CString, KSProcessInfo>				m_mapProcInfoFromLib;
	CAtlMap<CString, HICON>						m_mapIcon;


	CListBoxData(){
		m_arrColumState.SetCount(10);
		m_arrColumState[0] = -1;
		m_arrColumState[1] = -1;
		m_arrColumState[2] = -1;
		m_arrColumState[3] = -1;
		m_arrColumState[4] = -1;
		m_arrColumState[5] = -1;
		m_arrColumState[6] = -1;
		m_arrColumState[7] = -1;
		m_arrColumState[8] = -1;
		m_arrColumState[9] = -1;


		m_nProcID				= -1;
		m_nMaxHeightSel			= -1;
		m_pKProcQuery			= NULL;
		m_strModulePath			= L"";
		m_hDefaultIcon			= NULL;

		m_mapProcInfoFromLib.RemoveAll();
		m_mapIcon.RemoveAll();
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
