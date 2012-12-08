#pragma once

#include <runoptimize/interface.h>
#include <wtl/atlctrls.h>

#define __USE_BASE_LIST_VIEW2	1

#define XX_ITEM_HEIGHT			26	//30
#define COL_STR_DEFAULT			RGB(0,0,0)
#define COL_CAPTION				RGB(243,243,243)
#define COL_ITEM_DEFAULT		RGB(255,255,255)
#define COL_ITEM_SELECT			RGB(235,245,255)
#define COL_ITEM_LINE			RGB(234,233,225)
#define COL_ITEM_SUCCESS		RGB(0xee,0xee,0xee)
#define COL_ITEM_FAIL			RGB(0xee,0xee,0xee)

#define COL_ITEM_TEXT_SUCCESS	RGB(0,128,0)
#define COL_ITEM_TEXT_FAILED	RGB(255,0,0)

#define LST_IGNORE_ITEM_TYPE_RUN		1
#define LST_IGNORE_ITEM_TYPE_SYSCFG		2

class IIgnoredOptLstCBack
{
public:
	virtual void ClickCheck() = 0;
};

struct _IGNORE_ITEM_DATA
{
public:
	_IGNORE_ITEM_DATA()
	{
		pInfo	= NULL;
		bCheck	= TRUE;
		rcDesc	= CRect(0,0,0,0);
		rcDisplay = CRect(0,0,0,0);
		pSysCfgInfo = NULL;
		nItemType = LST_IGNORE_ITEM_TYPE_RUN;
	}
	BOOL			bCheck;
	CKsafeRunInfo*	pInfo;
	CRect			rcDesc;
	CRect			rcDisplay;
	int				nItemType;
	CKSafeSysoptRegInfo* pSysCfgInfo;
};

class CIgnoredOptList : 
#if	__USE_BASE_LIST_VIEW2
	public CWindowImpl<CIgnoredOptList, CListViewCtrl>
#else
	public CWindowImpl<CIgnoredOptList, CListBox>
#endif
	,public COwnerDraw<CIgnoredOptList>
{
	typedef CWindowImpl<CIgnoredOptList, CListViewCtrl> _super;

public:

	CIgnoredOptList()
	{
		m_rcCheck = CRect(12,(XX_ITEM_HEIGHT-13)/2,25,(XX_ITEM_HEIGHT+13)/2 );
		m_pCB = NULL;
		m_pHoverTip = 0;
	}
	virtual ~CIgnoredOptList(){}

public:
	BEGIN_MSG_MAP(CIgnoredOptList)   
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CIgnoredOptList>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()  

	HWND Create( HWND hWndParent, _U_RECT rect /*= NULL*/, LPCTSTR szWindowName /*= NULL*/, DWORD dwStyle /*= 0*/, DWORD dwExStyle /*= 0*/, _U_MENUorID MenuOrID /*= 0U*/, LPVOID lpCreateParam /*= NULL*/ )
	{
		HWND hWnd = _super::Create(hWndParent, rect, szWindowName, dwStyle|LVS_OWNERDRAWFIXED, dwExStyle, MenuOrID, lpCreateParam);
		_super::SetExtendedListViewStyle( LVS_EX_FULLROWSELECT|_super::GetExtendedListViewStyle() );

// 		m_ctlHeader.SubclassWindow( GetHeader().m_hWnd );
// 		m_ctlHeader.SetHeight(25);
// 		m_ctlHeader.ModifyStyle(HDS_FULLDRAG, 0, 0);

		_super::SetBkColor(BACKGROUND_COLOR);
		return hWnd;
	}

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

	void SetCallback(IIgnoredOptLstCBack* pCB)
	{
		m_pCB = pCB;
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

		if ( hti.iSubItem==0 && rcButtons.PtInRect(ptx))
		{
			_IGNORE_ITEM_DATA* pdata = GetItemParamData(hti.iItem);
			if (pdata )
			{
				pdata->bCheck = !pdata->bCheck;
				bHandle = TRUE;
				CListViewCtrl::RedrawItems(hti.iItem,hti.iItem);
				if (m_pCB)
					m_pCB->ClickCheck();
			}
		}

		SetMsgHandled(bHandle);
	}

	BOOL IsCheckAll()
	{
		return GetCheckedCount() == GetItemCount();
	}

	void InsertItemX(_IGNORE_ITEM_DATA* param)
	{
		int i = CListViewCtrl::InsertItem( CListViewCtrl::GetItemCount(), _T(" "));
		CListViewCtrl::SetItemData(i,(DWORD_PTR)param);
	}

	BOOL InitArray(CSimpleArray<CKsafeRunInfo*>& arrayx)
	{
		int nCount = 0;
		BOOL bAppendTitle = FALSE;
		for ( int i=0; i < arrayx.GetSize(); i++)
		{
			CKsafeRunInfo*	pInfo = arrayx[i];
			AppendItem(pInfo);
			nCount ++;
		}
		return nCount > 0;
	}

	BOOL InitArray2(CSimpleArray<CKSafeSysoptRegInfo*>& arrayX)
	{
		int nCount = 0;
		BOOL bAppendTitle = FALSE;
		for ( int i=0; i < arrayX.GetSize(); i++)
		{
			CKSafeSysoptRegInfo* pSysCfgInfo = arrayX[i];
			AppendItem2(pSysCfgInfo);
			nCount ++;
		}
		return nCount > 0;
	}

	void AppendItem(CKsafeRunInfo* pInfo)
	{
		_IGNORE_ITEM_DATA*	px = new _IGNORE_ITEM_DATA;
		px->bCheck	= TRUE;
		px->pInfo	= pInfo;

		InsertItemX(px);
	}

	void AppendItem2(CKSafeSysoptRegInfo* pInfo)
	{
		_IGNORE_ITEM_DATA*	px = new _IGNORE_ITEM_DATA;
		px->bCheck	= TRUE;
		px->pSysCfgInfo	= pInfo;
		px->nItemType = LST_IGNORE_ITEM_TYPE_SYSCFG;

		InsertItemX(px);
	}

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		_IGNORE_ITEM_DATA*	pData = GetItemParamData(lpDrawItemStruct->itemID);
		CDCHandle			dcx;
		CRect				rcItem = lpDrawItemStruct->rcItem;
		DWORD				nOldClr;
		BOOL				bSelect = FALSE ;
		DWORD				nTformat= DT_VCENTER|DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS;

		if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
			bSelect = TRUE;

		dcx.Attach(lpDrawItemStruct->hDC);
		HFONT fntTmp = dcx.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		nOldClr = dcx.SetTextColor(COL_STR_DEFAULT);
		if (pData)
		{
			if ( bSelect )
				dcx.FillSolidRect(&rcItem,COL_ITEM_SELECT);
			else
				dcx.FillSolidRect(&rcItem,COL_ITEM_DEFAULT);

			CRect	rcFirst;
			CRect	rcSecond;
			CRect	rcThird;
			CRect	rcItem = lpDrawItemStruct->rcItem;
			GetSubItemRect(lpDrawItemStruct->itemID,0,LVIR_LABEL,&rcFirst);
			GetSubItemRect(lpDrawItemStruct->itemID,1,LVIR_LABEL,&rcSecond);
			GetSubItemRect(lpDrawItemStruct->itemID,2,LVIR_LABEL,&rcThird);

			if (TRUE)
			{
				CRect			rcCheckbox = m_rcCheck;
				CDC				dcTmp;
				dcTmp.CreateCompatibleDC(dcx);

				HBITMAP hBmpCheck	= BkBmpPool::GetBitmap(IDB_BITMAP_LISTCTRL_CHECK);
				HBITMAP hBmpOld		= dcTmp.SelectBitmap(hBmpCheck);

				rcCheckbox.OffsetRect(rcFirst.left,rcFirst.top);
				dcx.BitBlt( rcCheckbox.left, 
					rcCheckbox.top, 
					rcCheckbox.Width(),
					rcCheckbox.Height(), 
					dcTmp, 
					pData->bCheck?0:13,
					0,SRCCOPY);
				dcTmp.SelectBitmap(hBmpOld);
			}

			if (pData->nItemType == LST_IGNORE_ITEM_TYPE_RUN)
			{
				CKsafeRunInfo*	pInfo = pData->pInfo;
				
				if (TRUE)
				{
					CRect	rcText = rcFirst;
					rcText.left += m_rcCheck.right+10;
					CRect rcProbe;
					dcx.DrawText(pInfo->strDisplay,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
					dcx.DrawText(pInfo->strDisplay, -1, &rcText, nTformat);
					rcProbe.bottom = rcProbe.top + 30;
					calcLeftRECT(rcText,rcProbe,pData->rcDisplay);
				}

				if (TRUE)
				{
					CRect rcText = rcSecond;
					CRect rcProbe;
					rcText.left += 10;
					dcx.DrawText(pInfo->strDesc,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
					dcx.DrawText(pInfo->strDesc, -1, &rcText, nTformat);
					rcProbe.bottom = rcProbe.top+30;
					calcLeftRECT(rcText,rcProbe,pData->rcDesc);
				}

				if (TRUE)
				{
					CRect	rcText = rcThird;
					rcText.left += 10;
					CString	strTip;
					if (pInfo->nType==KSRUN_TYPE_STARTUP)
						strTip = BkString::Get(IDS_SYSOPT_4002);
					if(pInfo->nType==KSRUN_TYPE_SERVICE)
						strTip = BkString::Get(IDS_SYSOPT_4003);
					if(pInfo->nType==KSRUN_TYPE_TASK)
						strTip = BkString::Get(IDS_SYSOPT_4004);
					dcx.DrawText(strTip, -1, &rcText, nTformat);
				}
			}
			else if (pData->nItemType == LST_IGNORE_ITEM_TYPE_SYSCFG)
			{
				CKSafeSysoptRegInfo* pInfo = pData->pSysCfgInfo;
				if (TRUE)
				{
					CRect	rcText = rcFirst;
					rcText.left += m_rcCheck.right+10;
					CRect rcProbe;
					dcx.DrawText(pInfo->GetTitleStr(), -1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
					dcx.DrawText(pInfo->GetTitleStr(), -1, &rcText, nTformat);
					rcProbe.bottom = rcProbe.top + 30;
					calcLeftRECT(rcText,rcProbe,pData->rcDisplay);
				}

				if (TRUE)
				{
					CRect rcText = rcSecond;
					CRect rcProbe;
					rcText.left += 10;
					dcx.DrawText(pInfo->m_strDesc,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
					dcx.DrawText(pInfo->m_strDesc, -1, &rcText, nTformat);
					rcProbe.bottom = rcProbe.top+30;
					calcLeftRECT(rcText,rcProbe,pData->rcDesc);
				}
			}
		}
		dcx.SetTextColor(nOldClr);
		dcx.SelectFont(fntTmp);
		dcx.Detach();
	}

	void calcLeftRECT(CRect rcArea, CRect rcProbe, CRect& rcOut)
	{
		rcOut.left		= rcArea.left;
		rcOut.right		= rcOut.left + rcProbe.Width();
		rcOut.top		= rcArea.top;
		rcOut.bottom	= rcOut.top + rcProbe.Height();
	}

	BOOL DeleteItem(int nItem)
	{
		return _super::DeleteItem(nItem);
	}

	void DeleteItem(LPDELETEITEMSTRUCT lParam)
	{
		return;
	}

	void DeleteCheckedItem()
	{
		for(int i = GetItemCount(); i >0 ;i--)
		{
			if (GetCheckState2(i-1) == TRUE)
				DeleteItem(i-1);
		}
	}

	void MeasureItem(LPMEASUREITEMSTRUCT lParam)
	{
		lParam->itemHeight = XX_ITEM_HEIGHT;
	}

	void CheckAll(BOOL bCheck)
	{
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_IGNORE_ITEM_DATA*	pdata = GetItemParamData(i);
				pdata->bCheck = bCheck;
		}
		CListViewCtrl::InvalidateRect(NULL);
	}

	int GetCheckedCount()
	{
		int nCount = 0;
		for ( int i=0; i < CListViewCtrl::GetItemCount(); i++)
		{
			_IGNORE_ITEM_DATA*	pdata = GetItemParamData(i);
			if (pdata->bCheck == TRUE)
				nCount++;
		}
		return nCount;
	}

	BOOL GetCheckState2(int nItem)
	{
		BOOL bRet = FALSE;
		_IGNORE_ITEM_DATA*	pdata = GetItemParamData(nItem);

		if (pdata && pdata->bCheck )
				bRet = TRUE;

		return bRet;
	}


	_IGNORE_ITEM_DATA* GetItemParamData(int index)
	{
		return (_IGNORE_ITEM_DATA*)CListViewCtrl::GetItemData(index);
	}

	void OnMouseMove(UINT uMsg, CPoint ptx)
	{
		BOOL	bShowTip = FALSE;
		int	iItem = XBASE_HitTest(ptx);

		CKsafeRunInfo*	kinfo = GetDataInfo(iItem);
		CKSafeSysoptRegInfo* pSysCfgInfo = GetSysCfgDataInfo(iItem);
		_IGNORE_ITEM_DATA* pItemData = (_IGNORE_ITEM_DATA*)XBASE_GetItemData(iItem);

		if (pItemData && (kinfo || pSysCfgInfo))
		{
			int iIndex = LabelHitTest(ptx);		

			if (iIndex==CLICK_INDEX_DESC || iIndex==CLICK_INDEX_CAPTION)
			{
				if (pItemData->nItemType == LST_IGNORE_ITEM_TYPE_RUN)
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
				else if (pItemData->nItemType == LST_IGNORE_ITEM_TYPE_SYSCFG)
				{
					if ( ((DWORD)pSysCfgInfo!=m_pHoverTip) )
					{
						CString	strInfo;

						if (iIndex==CLICK_INDEX_DESC)
							strInfo.Append( pSysCfgInfo->m_strDesc);
						else if (iIndex==CLICK_INDEX_CAPTION )
							strInfo.Append( pSysCfgInfo->GetTitleStr());

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

	CKsafeRunInfo* GetDataInfo(int nItem)
	{
		_IGNORE_ITEM_DATA*	pParam = (_IGNORE_ITEM_DATA*)XBASE_GetItemData(nItem);

		if (pParam!=NULL && (DWORD)pParam != -1 )
			return pParam->pInfo;
		else
			return NULL;
	}

	CKSafeSysoptRegInfo* GetSysCfgDataInfo(int nItem)
	{
		_IGNORE_ITEM_DATA*	pParam = (_IGNORE_ITEM_DATA*)XBASE_GetItemData(nItem);

		if (pParam!=NULL && (DWORD)pParam != -1 )
			return pParam->pSysCfgInfo;
		else
			return NULL;
	}

	int LabelHitTest(CPoint ptx)
	{
		int		iItem = XBASE_HitTest(ptx);

		CRect	rcItem;
		_IGNORE_ITEM_DATA* pdata = (_IGNORE_ITEM_DATA*)XBASE_GetItemData(iItem);

		if (pdata==NULL)
			return CLICK_INDEX_INVALID;

		if (pdata->rcDesc.PtInRect(ptx))
			return CLICK_INDEX_DESC;
		else if (pdata->rcDisplay.PtInRect(ptx))
			return CLICK_INDEX_CAPTION;
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

protected:
	DWORD	m_nCountRun;
	DWORD	m_nCountServ;
	DWORD	m_nCountTask;
	CRect	m_rcCheck;
	IIgnoredOptLstCBack*	m_pCB;
	CToolTipCtrl		m_wndToolTip;
	DWORD				m_pHoverTip;
	CSimpleArray<int>	m_arrayCheckedId;
	CSimpleArray<int>	m_arrayIgnoredId;
/*	CHeaderCtrlEx		m_ctlHeader;*/
};