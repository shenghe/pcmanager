
#pragma once

#include <runoptimize/interface.h>
#include <wtl/atlctrls.h>

#define __USE_BASE_LIST_VIEW2	1

#define ARUN_TYPE_ITEM					1
#define ARUN_TYPE_ITEM_SYSCFG			2

#define ARUN_TYPE_TITLE_BEGIN			10
#define ARUN_TYPE_TITLE_RUN				ARUN_TYPE_TITLE_BEGIN + 1	//可优化的启动项
#define ARUN_TYPE_TITLE_SERVE			ARUN_TYPE_TITLE_BEGIN + 2	//可优化的服务项
#define ARUN_TYPE_TITLE_TASK			ARUN_TYPE_TITLE_BEGIN + 3	//可优化的计划任务项
#define ARUN_TYPE_TITLE_RUN_DELAY		ARUN_TYPE_TITLE_BEGIN + 4	//可延迟的启动项

#define ARUN_TYPE_TITLE_SYSCFG_BEGIN	20
#define ARUN_TYPE_TITLE_SYSCFG_1		ARUN_TYPE_TITLE_SYSCFG_BEGIN + 1	//开关机加速优化项
#define ARUN_TYPE_TITLE_SYSCFG_2		ARUN_TYPE_TITLE_SYSCFG_BEGIN + 2	//系统运行加速优化项
#define ARUN_TYPE_TITLE_SYSCFG_3		ARUN_TYPE_TITLE_SYSCFG_BEGIN + 3	//系统稳定优化项
#define ARUN_TYPE_TITLE_SYSCFG_4		ARUN_TYPE_TITLE_SYSCFG_BEGIN + 4	//通用优化项
#define ARUN_TYPE_TITLE_SYSCFG_5		ARUN_TYPE_TITLE_SYSCFG_BEGIN + 5	//文件打开速度优化
#define ARUN_TYPE_TITLE_SYSCFG_6		ARUN_TYPE_TITLE_SYSCFG_BEGIN + 6	//网速优化

#define ARUN_STATUS_NULL		0
#define ARUN_STATUS_SUCCESS		1
#define ARUN_STATUS_FAILED		2
#define ARUN_STATUS_DOING		3

#define XX_ITEM_HEIGHT			26
#define COL_STR_DEFAULT			RGB(0,0,0)
#define COL_CAPTION				RGB(243,243,243)
#define COL_ITEM_DEFAULT		RGB(255,255,255)
#define COL_ITEM_SELECT			RGB(235,245,255)
#define COL_ITEM_LINE			RGB(234,233,225)
#define COL_ITEM_SUCCESS		RGB(0xee,0xee,0xee)
#define COL_ITEM_FAIL			RGB(0xee,0xee,0xee)

#define COL_ITEM_TEXT_SUCCESS	RGB(0,128,0)
#define COL_ITEM_TEXT_FAILED	RGB(255,0,0)
#define COL_ITEM_TEXT_ORANGE	RGB(204,102,0)

#define CLICK_INDEX_INVALID		1
#define CLICK_INDEX_DESC		2
#define CLICK_INDEX_CAPTION		3
#define CLICK_INDEX_NULL		4

#define CLICK_INDEX_IGNORE		10

#define AUTOOPT_ITEM_TYPE_RUN		101
#define AUTOOPT_ITEM_TYPE_SRV		102
#define AUTOOPT_ITEM_TYPE_TASK		103
#define AUTOOPT_ITEM_TYPE_DELAY		104

class IAutoOptLstCBack
{
public:
	virtual void ClickCheck() = 0;
	virtual BOOL OnClickAddIgnore(DWORD nIndex, DWORD dwID, int nItem) = 0;
};

struct _ARUN_ITEM_DATA
{
public:
	_ARUN_ITEM_DATA()
	{
		nType	= 0;
		nStatus	= ARUN_STATUS_NULL;
		pInfo	= NULL;
		bCheck	= TRUE;
		bDelay	= FALSE;
		rcDesc			= CRect(0,0,0,0);
		rcDisplay		= CRect(0,0,0,0);
		rcIgnoreLnk		= CRect(0,0,0,0);
	}
	DWORD			nType;
	DWORD			nStatus;
	BOOL			bCheck;
	BOOL			bDelay;
	CKsafeRunInfo*	pInfo;
	CRect			rcDesc;
	CRect			rcDisplay;
	CRect			rcIgnoreLnk;
	CKSafeSysoptRegInfo* pSysCfgInfo;
};

class CAutoRunOptList : 
	public CWindowImpl<CAutoRunOptList, CListViewCtrl>
	,public COwnerDraw<CAutoRunOptList>
{
	typedef CWindowImpl<CAutoRunOptList, CListViewCtrl> _super;
public:
	CAutoRunOptList(){
		m_rcCheck = CRect(9,(XX_ITEM_HEIGHT-13)/2,22,(XX_ITEM_HEIGHT+13)/2 );
		m_pCB = NULL;
		m_pHoverTip = 0;
	}
	virtual ~CAutoRunOptList(){}

public:
	BEGIN_MSG_MAP(CAutoRunOptList)   
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CAutoRunOptList>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()  

	void CreateToolTip()
	{
		m_wndToolTip.Create(m_hWnd);

		CToolInfo ti(0, m_hWnd);
		m_wndToolTip.AddTool(ti);
		m_wndToolTip.Activate(TRUE);
		m_wndToolTip.SetMaxTipWidth(500);
		m_wndToolTip.SetDelayTime(TTDT_AUTOPOP,5*1000);
		m_wndToolTip.SetDelayTime(TTDT_INITIAL,500);
	}

	void SetCallback(IAutoOptLstCBack* pCB)
	{
		m_pCB = pCB;
	}

	int GetCheckedCount()
	{
		m_arrayCheckedId.RemoveAll();
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA* pdata = GetItemParamData(i);

			if (pdata)
			{
				if (pdata->bCheck)
				{
					if (pdata->nType == ARUN_TYPE_ITEM)
					{
						m_arrayCheckedId.Add(pdata->pInfo->nID);
						nCount++;
					}
					else if (pdata->nType == ARUN_TYPE_ITEM_SYSCFG)
					{
						m_arrayCheckedId.Add(pdata->pSysCfgInfo->m_nID);
						nCount++;
					}
				}
			}
		}
		return nCount;
	}

	BOOL GetCheckState2(int nItem)
	{
		BOOL bRet = FALSE;
		_ARUN_ITEM_DATA*	pdata = GetItemParamData(nItem);
		
		if (pdata && (pdata->nType == ARUN_TYPE_ITEM || pdata->nType == ARUN_TYPE_ITEM_SYSCFG))
		{
			if (pdata->bCheck )
				bRet = TRUE;
		}

		return bRet;
	}

	int GetSuccessNumber()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata && (pdata->nType == ARUN_TYPE_ITEM || pdata->nType == ARUN_TYPE_ITEM_SYSCFG))
			{
				if ( pdata->nStatus == ARUN_STATUS_SUCCESS)
					nCount++;
			}
		}
		return nCount;
	}

	int GetFailedNumber()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata && (pdata->nType == ARUN_TYPE_ITEM || pdata->nType == ARUN_TYPE_ITEM_SYSCFG))
			{
				if ( pdata->nStatus == ARUN_STATUS_FAILED)
					nCount++;
			}
		}
		return nCount;
	}

	void OnLButtonDown(UINT uMsg, CPoint ptx)
	{
		BOOL	bHandle = FALSE;
		LVHITTESTINFO hti;
		hti.pt = ptx;
		SubItemHitTest( &hti );

		CRect	rcItem;
		CRect	rcButtons = m_rcCheck;
		GetItemRect(hti.iItem,&rcItem,LVIR_BOUNDS);
		rcButtons.OffsetRect(rcItem.left,rcItem.top);
		
		_ARUN_ITEM_DATA* pdata = GetItemParamData(hti.iItem);
		if ( hti.iSubItem==0 && rcButtons.PtInRect(ptx))
		{
			if (pdata 
				&& (pdata->nStatus == ARUN_STATUS_NULL || pdata->nStatus == ARUN_STATUS_DOING) 
				&& (pdata->nType == ARUN_TYPE_ITEM || pdata->nType == ARUN_TYPE_ITEM_SYSCFG))
			{
				pdata->bCheck = !pdata->bCheck;
				bHandle = TRUE;
				CListViewCtrl::RedrawItems(hti.iItem,hti.iItem);
				if (m_pCB)
					m_pCB->ClickCheck();
			}
		}
	
		if (pdata!=NULL)
		{
			int iItem = XBASE_HitTest(ptx);
			DWORD dwID = 0;
			if (pdata->nType == ARUN_TYPE_ITEM && pdata->pInfo)
			{
				CKsafeRunInfo*	pInfo = pdata->pInfo;
				dwID = pInfo->nID;
			}
			else if (pdata->nType == ARUN_TYPE_ITEM_SYSCFG && pdata->pSysCfgInfo)
			{
				CKSafeSysoptRegInfo* pSysCfgInfo = pdata->pSysCfgInfo;
				dwID = pSysCfgInfo->m_nID;
			}
			int iIndex = LabelHitTest(ptx);
			if (iIndex == CLICK_INDEX_IGNORE)
			{
				if (m_pCB)
				{
					m_pCB->OnClickAddIgnore(iIndex, dwID, hti.iItem);
					m_pCB->ClickCheck();
				}
				SetRedraw(TRUE);
				InvalidateRect(NULL);
			}
		}

		SetMsgHandled(bHandle);
	}

	BOOL IsCheckAll()
	{
		BOOL bCheckAll = FALSE;
		int nChecked = 0;
		int nTitle = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata && (pdata->nType == ARUN_TYPE_ITEM || pdata->nType == ARUN_TYPE_ITEM_SYSCFG))
			{
				if (pdata->bCheck)
					nChecked++;
			}
			else
				nTitle++;
		}
		return ((nChecked + nTitle) == CListViewCtrl::GetItemCount());
	}

	void AppendTitle(DWORD nType)
	{
		_ARUN_ITEM_DATA*	px = new _ARUN_ITEM_DATA;
		px->nType = nType;
		InsertItemX(px);
	}

	void InsertItemX(_ARUN_ITEM_DATA* param)
	{
		int i = CListViewCtrl::InsertItem( CListViewCtrl::GetItemCount(), _T(" "));
		CListViewCtrl::SetItemData(i,(DWORD_PTR)param);
	}

	DWORD GetTitleType(DWORD nRunType)
	{
		return nRunType;
	}

	DWORD GetSysCfgTitleType(DWORD nSysCfgType)
	{
		return nSysCfgType + 20;
	}

	DWORD GetRunInfoType(DWORD nType)
	{
		return nType - ARUN_TYPE_TITLE_BEGIN;
	}

	void _InitArrayType(CSimpleArray<CKsafeRunInfo*>& arrayx,DWORD nType, DWORD & nCount)
	{
		nCount = 0;
		BOOL	bAppendTitle = FALSE;
		for ( int i=0; i < arrayx.GetSize(); i++)
		{
			CKsafeRunInfo*	pInfo = arrayx[i];
			if (pInfo->nType == GetRunInfoType(nType) && pInfo->bEnable && pInfo->IsAdviseStop() )
			{
				if (!bAppendTitle)
				{
					bAppendTitle=TRUE;
					AppendTitle(GetTitleType(nType));		
				}
				AppendItem(pInfo);
				nCount++;
			}
		}
	}

	void _InitArrayType_Delay(CSimpleArray<CKsafeRunInfo*>& arrayx, DWORD nType, int nCount)
	{
		nCount = 0;
		BOOL	bAppendTitle = FALSE;
		for ( int i=0; i < arrayx.GetSize(); i++)
		{
			CKsafeRunInfo*	pInfo = arrayx[i];
			if (pInfo->IsCanDelay())
			{
				if (!bAppendTitle)
				{
					bAppendTitle=TRUE;
					AppendTitle(GetTitleType(nType));		
				}
				AppendItem2(pInfo, FALSE);
				nCount++;
			}
		}
	}

	BOOL InitArray(CSimpleArray<CKsafeRunInfo*>& arrayx, BOOL bOptDelayRun)
	{
		_InitArrayType(arrayx, ARUN_TYPE_TITLE_RUN,   m_nCountRun);
		_InitArrayType(arrayx, ARUN_TYPE_TITLE_SERVE, m_nCountServ);
		_InitArrayType(arrayx, ARUN_TYPE_TITLE_TASK,  m_nCountTask);
		if (bOptDelayRun == TRUE)
			_InitArrayType_Delay(arrayx, ARUN_TYPE_TITLE_RUN_DELAY, m_nCountDelay);
		m_pCB->ClickCheck();
		return (GetItemCount()>0);
	}

	void _InitArrayType_SysCfg(CSimpleArray<CKSafeSysoptRegInfo*>& arrayx, DWORD nType)
	{
		int nCount = 0;
		BOOL	bAppendTitle = FALSE;
		CSimpleArray<int> arrayID;
		
		for ( int i=0; i < arrayx.GetSize(); i++)
		{
			BOOL bFind = FALSE;
			CKSafeSysoptRegInfo*	pInfo = arrayx[i];
			if (pInfo->m_nType == nType)
			{
				if (!bAppendTitle)
				{
					bAppendTitle=TRUE;
					AppendTitle(GetSysCfgTitleType(nType));
				}
				
				for (int i = 0; i < arrayID.GetSize(); i++)
				{
					if (pInfo->m_nID == arrayID[i])
					{
						bFind = TRUE;
					}
				}
				if (!bFind)
				{
					AppendItem3(pInfo);
					arrayID.Add(pInfo->m_nID);
					nCount++;
				}
			}
		}
	}

	BOOL InitSyscfgArray(CSimpleArray<CKSafeSysoptRegInfo*>& arrayx)
	{
		_InitArrayType_SysCfg(arrayx, 1);
		_InitArrayType_SysCfg(arrayx, 2);
		_InitArrayType_SysCfg(arrayx, 3);
		_InitArrayType_SysCfg(arrayx, 4);
		m_pCB->ClickCheck();
		return (GetItemCount() > 0);
	}
	//建议禁止的项目
	void AppendItem(CKsafeRunInfo* pInfo)
	{
		_ARUN_ITEM_DATA*	px = new _ARUN_ITEM_DATA;
		px->nStatus	= ARUN_STATUS_NULL;
		px->nType	= ARUN_TYPE_ITEM;
		px->pInfo	= pInfo;

		InsertItemX(px);
	}
	//可以延迟启动的项目
	void AppendItem2(CKsafeRunInfo* pInfo, BOOL bCheck)
	{
		_ARUN_ITEM_DATA*	px = new _ARUN_ITEM_DATA;
		px->bCheck	= bCheck;
		px->nStatus	= ARUN_STATUS_NULL;
		px->nType	= ARUN_TYPE_ITEM;
		px->pInfo	= pInfo;
		px->bDelay  = TRUE;

		InsertItemX(px);
	}
	//系统设置优化相关项目
	
	void AppendItem3(CKSafeSysoptRegInfo* pInfo)
	{
		_ARUN_ITEM_DATA*	px = new _ARUN_ITEM_DATA;
		px->nStatus	= ARUN_STATUS_NULL;
		px->nType	= ARUN_TYPE_ITEM_SYSCFG;
		px->pSysCfgInfo = pInfo;
		DWORD array_dwNotCheckID[] = {524292,524293,524294};
		for(int i=0;i < sizeof(array_dwNotCheckID)/sizeof(array_dwNotCheckID[0]);i++)
		{
			if (pInfo->m_nID == array_dwNotCheckID[i])
				px->bCheck = FALSE;
		}

		InsertItemX(px);
	}

	_ARUN_ITEM_DATA* GetItemParamData(int index)
	{
		return (_ARUN_ITEM_DATA*)CListViewCtrl::GetItemData(index);
	}

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		_ARUN_ITEM_DATA*	pData = GetItemParamData(lpDrawItemStruct->itemID);
		CDCHandle			dcx;
		CRect				rcItem = lpDrawItemStruct->rcItem;
		CRect				rcItemTemp;
		GetClientRect(rcItemTemp);
		rcItem.right = rcItem.left + rcItemTemp.Width();
		DWORD				nOldClr;
		BOOL				bSelect = FALSE ;
		DWORD				nTformat = DT_VCENTER|DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS;
		DWORD				nTformat2 = DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_END_ELLIPSIS;
		
		if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
			bSelect = TRUE;

		dcx.Attach(lpDrawItemStruct->hDC);
		HFONT fntTmp = dcx.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		nOldClr = dcx.SetTextColor(COL_STR_DEFAULT);
		if (pData)
		{
			if (pData->nType > ARUN_TYPE_TITLE_BEGIN)
			{
				if (TRUE)
				{
					CDC dcTmp;
					dcTmp.CreateCompatibleDC(dcx);
					HBITMAP hBmpCheck	= BkBmpPool::GetBitmap(IDB_RUNOPT_LIST_HEADER2);
					HBITMAP hBmpOld		= dcTmp.SelectBitmap(hBmpCheck);
					dcx.StretchBlt( rcItem.left, rcItem.top, rcItem.Width(),rcItem.Height(), dcTmp, 0,0,1,25,SRCCOPY);
					dcTmp.SelectBitmap(hBmpOld);
				}
				_DrawTitle(dcx,&lpDrawItemStruct->rcItem,pData->nType);
			}
			else
			{
				if ( bSelect)
					dcx.FillSolidRect(&rcItem,COL_ITEM_SELECT);
				else
					dcx.FillSolidRect(&rcItem,COL_ITEM_DEFAULT);

				CRect	rcFirst;
				CRect	rcSecond;
				CRect	rcThird;
				CRect	rcFourth;
				GetSubItemRect(lpDrawItemStruct->itemID,0,LVIR_LABEL,&rcFirst);
				GetSubItemRect(lpDrawItemStruct->itemID,1,LVIR_LABEL,&rcSecond);
				GetSubItemRect(lpDrawItemStruct->itemID,2,LVIR_LABEL,&rcThird);
				GetSubItemRect(lpDrawItemStruct->itemID,3,LVIR_LABEL,&rcFourth);

				if (TRUE)//画Checkbox：
				{
					CRect			rcCheckbox = m_rcCheck;
					CDC				dcTmp;
					dcTmp.CreateCompatibleDC(dcx);

					HBITMAP hBmpCheck	= BkBmpPool::GetBitmap(IDB_BITMAP_LISTCTRL_CHECK);
					HBITMAP hBmpOld		= dcTmp.SelectBitmap(hBmpCheck);

					rcCheckbox.OffsetRect(rcFirst.left,rcFirst.top);
					if (pData->nStatus == ARUN_STATUS_SUCCESS)
						dcx.BitBlt( rcCheckbox.left, 
						rcCheckbox.top, 
						rcCheckbox.Width(),
						rcCheckbox.Height(), 
						dcTmp, 
						26,
						0,SRCCOPY);
					else if (pData->nStatus == ARUN_STATUS_FAILED )
						dcx.BitBlt( rcCheckbox.left, 
						rcCheckbox.top, 
						rcCheckbox.Width(),
						rcCheckbox.Height(), 
						dcTmp, 
						39,
						0,SRCCOPY);
					else
						dcx.BitBlt( rcCheckbox.left, 
						rcCheckbox.top, 
						rcCheckbox.Width(),
						rcCheckbox.Height(), 
						dcTmp, 
						pData->bCheck?0:13,
						0,SRCCOPY);

					dcTmp.SelectBitmap(hBmpOld);
				}

				if (pData->nType == ARUN_TYPE_ITEM && pData->pInfo != NULL)
				{
					CKsafeRunInfo*	pInfo = pData->pInfo;
					if (TRUE)//画标题：
					{
						CRect	rcText = rcFirst;
						rcText.left += m_rcCheck.right+10;
						CRect rcProbe;
						dcx.DrawText(pInfo->strDisplay,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(pInfo->strDisplay, -1, &rcText, nTformat);
						rcProbe.bottom = rcProbe.top + 30;
						calcLeftRECT(rcText,rcProbe,pData->rcDisplay);
					}

					if (TRUE)//画描述：
					{
						CRect rcText = rcSecond;
						CRect rcProbe;
						rcText.left += 10;
						dcx.DrawText(pInfo->strDesc,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(pInfo->strDesc, -1, &rcText, nTformat);
						rcProbe.bottom = rcProbe.top + 30;
						calcLeftRECT(rcText,rcProbe,pData->rcDesc);
					}

					if (pData->nStatus==ARUN_STATUS_SUCCESS)
					{
						CRect	rcText = rcThird;
						CString	strTip;
						if (pData->bDelay == TRUE)
							strTip = BkString::Get(IDS_SYSOPT_4099);
						else
							strTip = BkString::Get(IDS_SYSOPT_4019);

						dcx.SetTextColor(COL_ITEM_TEXT_SUCCESS);
						dcx.DrawText(strTip, -1, &rcText, nTformat2);
					}
					else if (pData->nStatus==ARUN_STATUS_FAILED)
					{
						CRect	rcText = rcThird;
						CString	strTip;
						strTip = BkString::Get(IDS_SYSOPT_4021);
						dcx.SetTextColor(COL_ITEM_TEXT_FAILED);
						dcx.DrawText(strTip, -1, &rcText, nTformat2);
					}
					else if (pData->nStatus == ARUN_STATUS_DOING)
					{
						CString strTip;
						if (pData->bDelay == TRUE)
							strTip = BkString::Get(IDS_SYSOPT_4098);
						else
							strTip = BkString::Get(IDS_SYSOPT_4022);

						CRect	rcText = rcThird;
						dcx.SetTextColor(COL_ITEM_TEXT_ORANGE);
						dcx.DrawText(strTip, -1, &rcText, nTformat2);
					}
					else if (pData->nStatus==ARUN_STATUS_NULL)
					{
						if (TRUE)
						{
							CString strTip;
							if (pData->bDelay == TRUE)
								strTip = BkString::Get(IDS_SYSOPT_4098);
							else
								strTip = BkString::Get(IDS_SYSOPT_4022);

							CRect	rcText = rcThird;
							dcx.SetTextColor(COL_ITEM_TEXT_ORANGE);
							dcx.DrawText(strTip, -1, &rcText, nTformat2);
						}
						CRect	rcText = rcFourth;
						CString	strTip = BkString::Get(IDS_SYSOPT_4090);
						dcx.SetTextColor(COL_DEFAULT_LINK);
						HFONT hfnt = BkFontPool::GetFont(FALSE,TRUE,FALSE);
						HFONT hTmp = dcx.SelectFont(hfnt);
						CRect rcProbe;
						dcx.DrawText(strTip,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_CENTER);
						dcx.DrawText(strTip, -1, &rcText, DT_VCENTER|DT_SINGLELINE|DT_CENTER|DT_END_ELLIPSIS);
						dcx.SelectFont(hTmp);
						rcProbe.bottom = rcProbe.top + 12;
						calcCenterRECT(rcText,rcProbe,pData->rcIgnoreLnk);
					}

				}
				else if (pData->nType == ARUN_TYPE_ITEM_SYSCFG && pData->pSysCfgInfo != NULL)
				{
					CKSafeSysoptRegInfo* pCfgInfo = pData->pSysCfgInfo;
					if (TRUE)//画标题：
					{
						CRect	rcText = rcFirst;
						rcText.left += m_rcCheck.right+10;
						rcText.right = rcFirst.Width() + rcSecond.Width();
						CRect rcProbe;
						dcx.DrawText(pCfgInfo->m_strDesc, -1, &rcProbe, DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(pCfgInfo->m_strDesc, -1, &rcText, nTformat);
						rcProbe.bottom = rcProbe.top + 30;
						calcLeftRECT(rcText,rcProbe,pData->rcDisplay);
					}

					if (pData->nStatus==ARUN_STATUS_SUCCESS)
					{
						CRect	rcText = rcThird;
						CString	strTip;
						strTip = BkString::Get(IDS_SYSOPT_4109);
						dcx.SetTextColor(COL_ITEM_TEXT_SUCCESS);
						dcx.DrawText(strTip, -1, &rcText, nTformat2);
					}
					else if (pData->nStatus==ARUN_STATUS_FAILED)
					{
						CRect	rcText = rcThird;
						CString	strTip;
						strTip = BkString::Get(IDS_SYSOPT_4021);
						dcx.SetTextColor(COL_ITEM_TEXT_FAILED);
						dcx.DrawText(strTip, -1, &rcText, nTformat2);
					}
					else if (pData->nStatus == ARUN_STATUS_DOING)
					{
						CString strTip;
						strTip = BkString::Get(IDS_SYSOPT_4108);
						CRect	rcText = rcThird;
						dcx.SetTextColor(COL_ITEM_TEXT_ORANGE);
						dcx.DrawText(strTip, -1, &rcText, nTformat2);
					}
					else if (pData->nStatus==ARUN_STATUS_NULL)
					{
						if (TRUE)
						{
							CString strTip;
							strTip = BkString::Get(IDS_SYSOPT_4108);
							CRect	rcText = rcThird;
							dcx.SetTextColor(COL_ITEM_TEXT_ORANGE);
							dcx.DrawText(strTip, -1, &rcText, nTformat2);
						}

						CRect	rcText = rcFourth;
						CString	strTip = BkString::Get(IDS_SYSOPT_4090);
						dcx.SetTextColor(COL_DEFAULT_LINK);
						HFONT hfnt = BkFontPool::GetFont(FALSE,TRUE,FALSE);
						HFONT hTmp = dcx.SelectFont(hfnt);
						CRect rcProbe;
						dcx.DrawText(strTip,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_CENTER);
						dcx.DrawText(strTip, -1, &rcText, DT_VCENTER|DT_SINGLELINE|DT_CENTER|DT_END_ELLIPSIS);
						dcx.SelectFont(hTmp);
						rcProbe.bottom = rcProbe.top + 12;
						calcCenterRECT(rcText,rcProbe,pData->rcIgnoreLnk);
					}
				}
			}
		}
		dcx.SetTextColor(nOldClr);
		dcx.SelectFont(fntTmp);
		dcx.Detach();
	}

	void calcCenterRECT(CRect rcArea, CRect rcProbe, CRect& rcOut)
	{
		rcOut.left		= rcArea.left + (rcArea.Width()-rcProbe.Width())/2;
		rcOut.right		= rcOut.left + rcProbe.Width();
		rcOut.top		= rcArea.top + (rcArea.Height()-rcProbe.Height())/2;
		rcOut.bottom	= rcOut.top + rcProbe.Height();
	}

	void calcLeftRECT(CRect rcArea, CRect rcProbe, CRect& rcOut)
	{
		rcOut.left		= rcArea.left;
		if (rcProbe.Width() <= rcArea.Width())
			rcOut.right	= rcOut.left + rcProbe.Width();
		else
			rcOut.right = rcArea.right;
		rcOut.top		= rcArea.top;
		rcOut.bottom	= rcOut.top + rcProbe.Height();
	}

	void MeasureItem(LPMEASUREITEMSTRUCT lParam)
	{
		lParam->itemHeight = XX_ITEM_HEIGHT;
	}

	BOOL DeleteItem(int nItem)
	{
		return _super::DeleteItem(nItem);
	}

	void DeleteItem(LPDELETEITEMSTRUCT)
	{
		return;
	}

	void ClearAll()
	{
		_super::DeleteAllItems();
	};

	void CheckAll(BOOL bCheck)
	{
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata && (pdata->nType == ARUN_TYPE_ITEM || pdata->nType == ARUN_TYPE_ITEM_SYSCFG))
			{
				pdata->bCheck = bCheck;
			}
		}
		CListViewCtrl::InvalidateRect(NULL);
	}

	void ResetStatus(DWORD dwStatus)
	{
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata 
				&& (pdata->nType == ARUN_TYPE_ITEM || pdata->nType == ARUN_TYPE_ITEM_SYSCFG)
				&& pdata->nStatus == ARUN_STATUS_NULL 
				&& pdata->bCheck == TRUE)
			{
				pdata->nStatus = dwStatus;
			}
		}
		CListViewCtrl::InvalidateRect(NULL);
	}

	DWORD GetTotalCount()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA* pdata = GetItemParamData(i);

			if (pdata && (pdata->nType == ARUN_TYPE_ITEM || pdata->nType == ARUN_TYPE_ITEM_SYSCFG))
				nCount++;
		}
		return nCount;
	}

	DWORD GetAdviceStopCount()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata 
				&& pdata->nType == ARUN_TYPE_ITEM 
				&& pdata->bDelay == FALSE
				&& pdata->pInfo->IsAdviseStop()
				&& pdata->nStatus != ARUN_STATUS_SUCCESS)
			{
				nCount++;
			}
		}
		return nCount;
	}

	int GetRunCount()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata 
				&& pdata->nType == ARUN_TYPE_ITEM 
				&& pdata->pInfo->nType == KSRUN_TYPE_STARTUP
				&& pdata->bDelay == FALSE)
				nCount++;
		}
		return nCount;
	}

	int GetDelayCount()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata 
				&& pdata->nType == ARUN_TYPE_ITEM 
				&& pdata->bDelay == TRUE)
				nCount++;
		}
		return nCount;
	}

	int GetServiceCount()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata 
				&& pdata->nType==ARUN_TYPE_ITEM 
				&& pdata->pInfo->nType == KSRUN_TYPE_SERVICE
				&& pdata->bDelay == FALSE)
				nCount++;
		}
		return nCount;
	}

	int GetTaskCount()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata 
				&& pdata->nType == ARUN_TYPE_ITEM 
				&& pdata->pInfo->nType == KSRUN_TYPE_TASK
				&& pdata->bDelay == FALSE)
				nCount++;
		}
		return nCount;
	}

	int GetSysCfgCount(int nType)
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA* pdata = GetItemParamData(i);

			if (pdata 
				&& pdata->nType == ARUN_TYPE_ITEM_SYSCFG 
				&& pdata->pSysCfgInfo
				&& pdata->pSysCfgInfo->m_nType == nType)
				nCount++;
		}
		return nCount;
	}

	void DeleteTitle(int nType)
	{
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	pdata = GetItemParamData(i);

			if (pdata && pdata->nType == nType)
			{
				DeleteItem(i);
				return;
			}
		}
	}

	void OnMouseMove(UINT uMsg, CPoint ptx)
	{
		BOOL	bShowTip = FALSE;
		int	iItem = XBASE_HitTest(ptx);

		CKsafeRunInfo*	kinfo = GetDataInfo(iItem);
		CKSafeSysoptRegInfo* pSysCfgInfo = GetSysCfgDataInfo(iItem);
		_ARUN_ITEM_DATA* pItemData = (_ARUN_ITEM_DATA*)XBASE_GetItemData(iItem);

		if (pItemData && (kinfo || pSysCfgInfo))
		{
			int iIndex = LabelHitTest(ptx);		

			if (iIndex == CLICK_INDEX_IGNORE)
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
			else if (iIndex==CLICK_INDEX_DESC || iIndex==CLICK_INDEX_CAPTION)
			{
				if (pItemData->nType == ARUN_TYPE_ITEM)
				{
					if ( ((DWORD)kinfo!=m_pHoverTip) )
					{
						CString	strInfo;

						if (iIndex==CLICK_INDEX_DESC)
							strInfo.Append( kinfo->strDesc.IsEmpty()?BkString::Get(IDS_SYSOPT_4033):kinfo->strDesc );
						else if (iIndex==CLICK_INDEX_CAPTION )
							strInfo.Append( kinfo->strDisplay );

						m_wndToolTip.SetMaxTipWidth(500);
						m_wndToolTip.UpdateTipText((LPCTSTR)strInfo,m_hWnd);
						m_pHoverTip = (DWORD)kinfo;
					}
				}
				else if (pItemData->nType == ARUN_TYPE_ITEM_SYSCFG)
				{
					if ( ((DWORD)pSysCfgInfo!=m_pHoverTip) )
					{
						CString	strInfo;
						if (iIndex==CLICK_INDEX_CAPTION )
							strInfo.Append( pSysCfgInfo->m_strDesc);

						m_wndToolTip.SetMaxTipWidth(500);
						m_wndToolTip.UpdateTipText((LPCTSTR)strInfo,m_hWnd);
						m_pHoverTip = (DWORD)pSysCfgInfo;
					}
				}
				
				bShowTip = TRUE;
			}
		}
		if (!bShowTip)
		{
			if (m_pHoverTip!=0)
			{
				m_pHoverTip = 0;
				m_wndToolTip.UpdateTipText((LPCTSTR)NULL,m_hWnd);
				m_wndToolTip.Pop();
			}
		}
	}

	LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if(m_wndToolTip.IsWindow())
		{
			MSG msg = { m_hWnd, uMsg, wParam, lParam };

			m_wndToolTip.RelayEvent(&msg);
		}

		SetMsgHandled(FALSE);

		return 0;
	}

	int LabelHitTest(CPoint ptx)
	{
		int		iItem = XBASE_HitTest(ptx);

		CRect	rcItem;
		_ARUN_ITEM_DATA* pdata = (_ARUN_ITEM_DATA*)XBASE_GetItemData(iItem);

		if (pdata==NULL)
			return CLICK_INDEX_INVALID;
		
		if (pdata->rcDesc.PtInRect(ptx))
			return CLICK_INDEX_DESC;
		else if (pdata->rcDisplay.PtInRect(ptx))
			return CLICK_INDEX_CAPTION;
		else if (pdata->rcIgnoreLnk.PtInRect(ptx) && pdata->nStatus == ARUN_STATUS_NULL)
			return CLICK_INDEX_IGNORE;
		else
			return CLICK_INDEX_NULL;
	}

	DWORD_PTR XBASE_GetItemData(int nItem)
	{
#if __USE_BASE_LIST_VIEW2
		return CListViewCtrl::GetItemData(nItem);
#else
		DWORD_PTR pvoid = CListBox::GetItemData(nItem);

		if ((DWORD)pvoid==-1)
			return NULL;
		return pvoid;
#endif
	}

	BOOL XBASE_GetItemRect(int i, LPRECT lprc)
	{
#if __USE_BASE_LIST_VIEW2
		return CListViewCtrl::GetItemRect(i,lprc,LVIR_BOUNDS);
#else
		return CListBox::GetItemRect(i,lprc);
#endif
	}

	int XBASE_HitTest(CPoint ptx)
	{
#if __USE_BASE_LIST_VIEW2
		LVHITTESTINFO hti;
		hti.pt = ptx;
		SubItemHitTest( &hti );
		return hti.iItem;
#else
		BOOL	bOut = FALSE;
		int i = (int)CListBox::ItemFromPoint(ptx,bOut);

		if (!bOut)
			return i;
		else
			return -1;
#endif
	}

	CKsafeRunInfo* GetDataInfo(int nItem)
	{
		_ARUN_ITEM_DATA*	pParam = (_ARUN_ITEM_DATA*)XBASE_GetItemData(nItem);

		if (pParam!=NULL && (DWORD)pParam != -1 )
			return pParam->pInfo;
		else
			return NULL;
	}
	
	CKSafeSysoptRegInfo* GetSysCfgDataInfo(int nItem)
	{
		_ARUN_ITEM_DATA*	pParam = (_ARUN_ITEM_DATA*)XBASE_GetItemData(nItem);

		if (pParam!=NULL && (DWORD)pParam != -1 )
			return pParam->pSysCfgInfo;
		else
			return NULL;
	}

	int GetIgnoredId(int nItem)
	{
		return m_arrayCheckedId[nItem];
	}

	int GetIgnoredCount()
	{
		return m_arrayIgnoredId.GetSize();
	}
protected:
	void _DrawTitle(CDCHandle& dcx, LPRECT lpRc, DWORD nType)
	{
		HFONT fntTmp = dcx.SelectFont(BkFontPool::GetFont(FALSE,FALSE,FALSE));
		dcx.SetTextColor(COL_DEFAULT_LINK);

		CRect	rcItem = *lpRc;
		rcItem.left += 13;

		DWORD	nFormat = DT_VCENTER|DT_LEFT|DT_SINGLELINE;
		CString	strShow;
		if (nType==ARUN_TYPE_TITLE_RUN)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4024), GetRunCount());
		}
		else if (nType==ARUN_TYPE_TITLE_SERVE)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4025), GetServiceCount());
		}
		else if (nType==ARUN_TYPE_TITLE_TASK)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4026), GetTaskCount());
		}
		else if (nType == ARUN_TYPE_TITLE_RUN_DELAY)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4097), GetDelayCount());
		}
		else if (nType == ARUN_TYPE_TITLE_SYSCFG_1)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4102), GetSysCfgCount(1));
		}
		else if (nType == ARUN_TYPE_TITLE_SYSCFG_2)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4103), GetSysCfgCount(2));
		}
		else if (nType == ARUN_TYPE_TITLE_SYSCFG_3)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4104), GetSysCfgCount(3));
		}
		else if (nType == ARUN_TYPE_TITLE_SYSCFG_4)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4105), GetSysCfgCount(4));
		}
		else if (nType == ARUN_TYPE_TITLE_SYSCFG_5)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4106), GetSysCfgCount(5));
		}
		else if (nType == ARUN_TYPE_TITLE_SYSCFG_6)
		{
			strShow.Format(BkString::Get(IDS_SYSOPT_4107), GetSysCfgCount(6));
		}
			
		dcx.DrawText(strShow,-1,&rcItem,nFormat);

		dcx.SelectFont(fntTmp);
	}

protected:
	DWORD	m_nCountRun;
	DWORD	m_nCountServ;
	DWORD	m_nCountTask;
	DWORD   m_nCountDelay;
	DWORD	m_nCountSyscfg;
	CRect	m_rcCheck;
	IAutoOptLstCBack*	m_pCB;
	CToolTipCtrl		m_wndToolTip;
	DWORD				m_pHoverTip;
	CSimpleArray<int>	m_arrayCheckedId;
	CSimpleArray<int>	m_arrayIgnoredId;
};