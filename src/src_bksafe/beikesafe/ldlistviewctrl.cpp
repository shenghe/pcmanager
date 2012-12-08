#include "StdAfx.h"
#include "beikesafe.h"
#include "LDListViewCtrl.h"
#include "bkmsgdefine.h"



CLDListViewCtrl::CLDListViewCtrl(void)
{
	m_uHeight = 30;
	m_bDrawBorder = TRUE;
	m_bDotFrameLine = FALSE;
	m_hCheckBitmp = BkBmpPool::GetBitmap(IDB_BITMAP_LISTCTRL_CHECK);
	m_hNotifyHwnd = NULL;
	m_bUrlHand = TRUE;
	m_bExTxtHand = TRUE;
	m_ListViewModel = MODEL_NORMAL;
	m_bSplitLines = TRUE;
	m_bItemUpdateCheckBox = FALSE;
	m_bSelState = TRUE;
	m_nSelItem = -1;
	m_dwCtrlID = 0;
	//m_arrImage.RemoveAll();
	m_nSubItemWidth = 0;
	m_hSubItemImage = NULL;
	m_pBtnSkin = NULL;
	m_pImageSkin = BkSkin::GetSkin("exam_stat15");
	m_pBtnSkin = BkSkin::GetSkin("normalbtn");
	m_btnState = BkWndState_Normal;
	m_bkColor = LDLISTVIEWCTRL_BK_COLOR;
	m_bCheckBoxHand = FALSE;
	m_bExpandRectHand = FALSE;
}

CLDListViewCtrl::~CLDListViewCtrl(void)
{
	//m_arrImage.RemoveAll();
}

void CLDListViewCtrl::DrawItem(LPDRAWITEMSTRUCT lpItemStruct)
{
	CDC dc;
	dc.Attach(lpItemStruct->hDC);
	int nCurItem = lpItemStruct->itemID;
	m_bSelected = FALSE;
	if (nCurItem < 0)
		return;//无效的行，返回

	m_bSelected = FALSE;
	m_bSelected = lpItemStruct->itemState & ODS_FOCUS || lpItemStruct->itemState & ODS_SELECTED;


	if (TRUE == m_bDrawBorder)
		_DrawBorder(dc);//绘制BOrder

	LPLDListViewCtrl_Item pItemStruct = _GetItemData(nCurItem);

	if (NULL == pItemStruct)
		return ;
	switch (pItemStruct->_enumItemType)
	{
	case ITEM_TYPE_NORMAL:
		{
			if (TRUE == pItemStruct->_bExpand)
				_DrawText(dc, nCurItem, pItemStruct);
		}
		break;
	case ITEM_TYPE_TITLE:
		{
			_DrawTitle(dc, nCurItem, pItemStruct);
		}
		break;
	case SUBITEM_TYPE_WITH_IMAGE:
		{//子项目中包含IMAGE
		}
		break;
	case ITEM_TYPE_UNKNOWN:
		{

		}
		break;
	default:
		break;
	}

	//释放
	dc.Detach();

}
//设置行高，如果给定的值,m_uHeight=0认为是一个无效的值，不会设置生效
void CLDListViewCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (0 == m_uHeight)
		return;
	lpMeasureItemStruct->itemHeight = m_uHeight;
}

int CLDListViewCtrl::CompareItem(LPCOMPAREITEMSTRUCT lpComplateItemStruct)
{
	return 0;
}
void CLDListViewCtrl::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{

}
//
int CLDListViewCtrl::DeleteAllItems()
{
	__super::DeleteAllItems();
	ReleaseBuf();

	SetMsgHandled(FALSE);

	return 0;
}

int CLDListViewCtrl::DeleteItem(int nItem)
{

	LPLDListViewCtrl_Item pData = _GetItemData(nItem);
	if (m_mapBtnRect.GetSize() > nItem)
		m_mapBtnRect.RemoveAt(nItem);	
	if (NULL != pData)
	{		
		if(ITEM_TYPE_TITLE == pData->_enumItemType)
		{
			__super::DeleteItem(nItem);//要先执行这个，否则就会出现删除错误
			_DeleteAnGroupData(pData->_uGroup);
			return 0;
		}
	}
	__super::DeleteItem(nItem);
	SetMsgHandled(FALSE);

	return 0;
}

//释放内存中的数据
void CLDListViewCtrl::ReleaseBuf()
{
	size_t count = m_arrListViewItem.GetSize();
	for (size_t i = 0; i < count; i++)
	{
		SAFE_DELETE_PTR(m_arrListViewItem[i]);
	}
	m_arrListViewItem.RemoveAll();
	m_mapBtnRect.RemoveAll();
}
//插入数据
int CLDListViewCtrl::InsertItem(int nItem, LPLDListViewCtrl_Item lpItemData)
{
	int nRet = -1;
	if (nItem < 0 )//如果传递过来一个-1，就默认认为插入到最后一行
	{
		nItem = CListViewCtrl::GetItemCount();
		lpItemData->_uItemID = nItem;
	}

	if (NULL == lpItemData)
		return nRet;

	m_arrListViewItem.Add(lpItemData);
	_InsertItemData(nItem, lpItemData);
	nRet = GetItemCount();

	if (TRUE == lpItemData->_bShowCheckBox)
		__super::SetCheckState(nItem, lpItemData->_bCheck);


	return nRet;
}
//高度设置为0，认为无效
int CLDListViewCtrl::SetItemHeight(ULONG uHeight /* = 0 */)
{
	if (0 == uHeight)
		return 0;
	ULONG u = m_uHeight;
	m_uHeight = uHeight;

	return u;
}

//设置border
BOOL CLDListViewCtrl::SetBorder(BOOL bBorder, COLORREF color /* = RGB */)
{
	m_colorBorder = color;
	BOOL bFlag = m_bDrawBorder;
	m_bDrawBorder =bBorder;

	return bFlag;
}
//绘制BORDER
void CLDListViewCtrl::_DrawBorder(CDC& memDC)
{
	CRect rc;
	GetClientRect(&rc);
	if (FALSE == rc.IsRectEmpty())
		return;

 	HBRUSH hBrush = NULL;
 	hBrush = CreateSolidBrush(m_colorBorder);
 	if (NULL == hBrush)
 		return ;
	FrameRect(memDC.m_hDC, &rc, hBrush);
	DeleteObject(hBrush);

}
//绘制背景色和每行的边线
void CLDListViewCtrl::_DrawItemBKColor(CDC& memDC, int nItem, BOOL bSelected /* = FALSE */)
{
	if (nItem < 0)
		return;

	CRect rcItem;
	GetItemRect(nItem, &rcItem, LVIR_BOUNDS);

	//背景
		if (_GetSelected())
		{
			memDC.FillSolidRect(&rcItem, LDLISTVIEWCTRL_SEL_COLOR);
		}
		else
		{
			memDC.FillSolidRect(&rcItem, m_bkColor);
		}

	//边线

	//DrawDotLine(&memDC, CPoint(rcItem.left, rcItem.bottom-1), CPoint(rcItem.right, rcItem.bottom-1), LDLISTVIEWCTRL_LINE_COLOR);
	DWORD dwType = PS_SOLID;
	if (TRUE == m_bDotFrameLine)
		dwType = PS_DOT;

	BOOL bDrawLine = FALSE;
	if (MODEL_NORMAL == m_ListViewModel)
	{
		bDrawLine = TRUE;
	}else if(MODEL_TITLE == m_ListViewModel)
	{
		LPLDListViewCtrl_Item pItemData = _GetItemData(nItem);
		if (NULL != pItemData)
		{
			if (ITEM_TYPE_TITLE == pItemData->_enumItemType && TRUE == m_bSplitLines)
			{
				int y = rcItem.top;//-m_uHeight+1;
				if (0 != nItem)//第一行不绘制行的顶部行线
					_DrawLine(memDC,  CPoint(rcItem.left, y), CPoint(rcItem.right, y), LDLISTVIEWCTRL_LINE_COLOR, dwType);

				if (TRUE == pItemData->_bExpand)
					bDrawLine = TRUE;
			}

		}
	}

	if (TRUE == bDrawLine && TRUE == m_bSplitLines)
		_DrawLine(memDC,  CPoint(rcItem.left, rcItem.bottom-1), CPoint(rcItem.right, rcItem.bottom-1), LDLISTVIEWCTRL_LINE_COLOR, dwType);

}
//设置每行的边线是否采用点线
BOOL CLDListViewCtrl::SetFrameLine(BOOL bDot)
{
	BOOL bFlag = m_bDotFrameLine;
	m_bDotFrameLine = bDot;

	return bFlag;
}
//绘制一条线
void CLDListViewCtrl::_DrawLine(CDC& memDC, CPoint ptStart, CPoint ptEnd, COLORREF color, DWORD dwPenTyple /*=PS_SOLID*/)
{

	HDC hDC = memDC.m_hDC;
	if (NULL == hDC)
		return ;

	HPEN hPen = ::CreatePen(dwPenTyple , 1, color), hOldPen = NULL;

	hOldPen = (HPEN)::SelectObject(hDC, hPen);

	::MoveToEx(hDC,ptStart.x, ptStart.y, NULL);
	::LineTo(hDC, ptEnd.x, ptEnd.y);

	::SelectObject(hDC, hOldPen);

	::DeleteObject(hPen);
}
//绘制文本
void CLDListViewCtrl::_DrawItemText(CDC& memDC, int nItem, int nSubItem, COLORREF colorText, LPCTSTR pszText, BOOL bTitle /*= FALSE*/)
{
	if (nItem < 0 && nSubItem < 0 || NULL == pszText)
		return;
	if (nSubItem < 0)
		nSubItem = 0;
	CRect rcText;
	
	GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rcText);
	_DrawItemText(memDC, rcText, pszText, colorText, BkFontPool::GetFont(BKF_DEFAULTFONT),bTitle);

}
//绘制CHECK BOX，返回CHECKBOX所占用的矩形区域，以便于其他的绘制操作向对于他的便宜
void CLDListViewCtrl::_DrawCheckBox(CDC& memDC , int nItem, BOOL bCheck, CRect &rcCheckBox)
{
	if (nItem < 0 || NULL == m_hCheckBitmp)
		return ;

	BITMAP bmp;
	GetObject( m_hCheckBitmp, sizeof(BITMAP), &bmp); 
	CDC dc;
	dc.CreateCompatibleDC(memDC.m_hDC);
	SelectObject(dc.m_hDC, m_hCheckBitmp);
	CRect rcItem;
	GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
	int nTop = rcItem.top;
	nTop = /*rcItem.bottom-bmp.bmHeight-3;*/rcItem.top + (rcItem.Height()-bmp.bmHeight)/2;
	//统一右便宜2个像素，这样显示出来会更好看些
	rcItem.OffsetRect(2, 0);

	if (TRUE == bCheck)
	{
		memDC.BitBlt(rcItem.left, nTop, bmp.bmWidth/2, bmp.bmHeight, dc.m_hDC, 0, 0, SRCCOPY);
	}
	else
	{
		memDC.BitBlt(rcItem.left, nTop, bmp.bmWidth/2, bmp.bmHeight, dc.m_hDC, 13, 0, SRCCOPY);
	}
	//返回CHECKBOX所占用的矩形区域
// 	if (TRUE == m_bItemUpdateCheckBox)
// 	{
// 		//rcCheckBox = rcItem;
// 	}
// 	else
	{
		rcCheckBox = rcItem;
		rcCheckBox.top = nTop;
		rcCheckBox.bottom = nTop + bmp.bmHeight;
		rcCheckBox.right = rcCheckBox.left + bmp.bmWidth/2;
	}

	ReleaseDC(dc.m_hDC);
}

void CLDListViewCtrl::_DrawItemText(CDC& memDC, CRect& rcItem, LPCTSTR pszText , COLORREF colorText , HFONT hFont, BOOL bTitle/* = FALSE*/)
{

	if (NULL == pszText)
		return;

	CRect rcText = rcItem;

	memDC.SetBkMode(TRANSPARENT);
	HFONT hftOld = memDC.SelectFont(hFont);
	COLORREF crOld = memDC.SetTextColor(colorText);
	DWORD dwFormat = DT_SINGLELINE | DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS;
	if (FALSE == bTitle)
	{

		memDC.DrawText(pszText, _tcslen(pszText),rcText, DT_CALCRECT|dwFormat);

		if (rcText.right > rcItem.right)
			rcText.right = rcItem.right-5;
	}
	else
	{
		dwFormat &= ~DT_CENTER;
		dwFormat |= DT_LEFT;
		
	}

	rcText.OffsetRect(0,4);
	memDC.DrawText(pszText, _tcslen(pszText),rcText, dwFormat);
	memDC.SelectFont(hftOld);
	memDC.SetTextColor(crOld);
	rcItem = rcText;
}


//获取每一行的关联数据
LPLDListViewCtrl_Item CLDListViewCtrl::_GetItemData(int nItem)
{
	LPLDListViewCtrl_Item pItem = NULL;
	if (nItem < 0)
		return pItem;
	size_t count = m_arrListViewItem.GetSize();


	pItem = (LPLDListViewCtrl_Item)GetItemData(nItem);

	return pItem;		
}

void CLDListViewCtrl::_DrawText(CDC& memDC, int nItem, LPLDListViewCtrl_Item pItemStruct)
{
	CRect rcItem;
	//rcItem = lpItemStruct->rcItem;
	GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
	if (TRUE == rcItem.IsRectEmpty())
		return;//无效矩形区域返回

	//绘制背景和边线
	_DrawItemBKColor(memDC, nItem, m_bSelected);


	//绘制checkbox
	CRect rcBox(0,0,0,0);
	if (TRUE == pItemStruct->_bShowCheckBox)
	{
		_DrawCheckBox(memDC, nItem, pItemStruct->_bCheck, rcBox);
		if (TRUE == m_bItemUpdateCheckBox)
		{
			pItemStruct->_rcCheckBox = rcItem;
		}
		else
		{
			pItemStruct->_rcCheckBox = rcBox;
		}
	}

	//绘制文本
	CRect rcText;
	size_t count = pItemStruct->_arrSubItem.GetSize();
	for (size_t i = 0; i < count; i++)
	{
		LPLDListViewCtrl_SubItem lpSubItem = pItemStruct->_arrSubItem[i];
		if (NULL == lpSubItem)
			continue;
		
		if (SUBITEM_TYPE_IMAGE == lpSubItem->_enumType)
		{//绘制image
			_DrawImageSubItem(memDC, nItem, i, lpSubItem);
		}else if (SUBITEM_TYPE_BUTTON == lpSubItem->_enumType)
		{//绘制button
			_DrawButtonSubItem(memDC, nItem, i, lpSubItem);
		}else if (SUBITEM_TYPE_WITH_IMAGE == lpSubItem->_enumType)
		{
			GetSubItemRect(nItem, i, LVIR_LABEL, rcText);
			CRect rcImage;
			_DrawImageSubItem2(memDC, rcText, rcImage, lpSubItem);
			rcText.left += rcImage.Width()+10 + lpSubItem->_OffsetLeft;
			if (0 == i)
			{
				pItemStruct->_rcCheckBox = rcImage;
			}
			_DrawItemText(memDC, rcText, lpSubItem);
		}
		else
		{//普通文本
			GetSubItemRect(nItem, i, LVIR_LABEL, rcText);
			if (0 == i && 1 == pItemStruct->_nTxtCheckBox)
			{
				CRect rc = rcText;
				::DrawText(memDC.m_hDC, pItemStruct->_pszGroupCaption, _tcslen(pItemStruct->_pszGroupCaption), &rc, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS|DT_LEFT|DT_CALCRECT);
				pItemStruct->_rcCheckBox = rc;
			}
			_DrawItemText(memDC, rcText, lpSubItem);
		}
		
	}

}

void CLDListViewCtrl::_DrawTitle(CDC& memDC, int nItem, LPLDListViewCtrl_Item pItemStruct)
{
	CRect rcItem;
	//rcItem = lpItemStruct->rcItem;
	GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
	if (TRUE == rcItem.IsRectEmpty())
		return;//无效矩形区域返回

	//绘制背景和边线
	_DrawItemBKColor(memDC, nItem, m_bSelected);

	//绘制checkbox
	CRect rcBox(0,0,0,0);
	if (TRUE == pItemStruct->_bShowCheckBox)
	{
		_DrawCheckBox(memDC, nItem, pItemStruct->_bCheck, rcBox);
		if (TRUE == m_bItemUpdateCheckBox)
		{
			pItemStruct->_rcCheckBox = rcItem;
		}
		else
		{
			pItemStruct->_rcCheckBox = rcBox;
		}
		
	}

	//绘制文本
 	CRect rcText = rcItem;
	rcText.OffsetRect(rcBox.Width()+5, 0);
	
	if (pItemStruct->_arrSubItem.GetSize() > 0)
	{
		pItemStruct->_rcExpand = rcText;
		LPLDListViewCtrl_SubItem pSubItemData = pItemStruct->_arrSubItem[0];
		rcText.left += pSubItemData->_OffsetLeft;
		if (NULL != pSubItemData && SUBITEM_TYPE_WITH_IMAGE == pSubItemData->_enumType)
		{
			CRect rcImage;
			_DrawImageSubItem2(memDC, rcText, rcImage, pSubItemData);
			pItemStruct->_rcExpand = rcImage;
			//pItemStruct->_rcCheckBox = rcImage;
			rcText.left += rcImage.Width()+10;
		}
		if (1 == pItemStruct->_nTxtCheckBox)
		{
			CRect rc = rcText;
			::DrawText(memDC.m_hDC, pSubItemData->_pszSubItemInfo, _tcslen(pSubItemData->_pszSubItemInfo), &rc, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS|DT_LEFT|DT_CALCRECT);
			pItemStruct->_rcCheckBox = rc;
			pItemStruct->_rcCheckBox.left = rcBox.left;
			pItemStruct->_rcCheckBox.right += 10;
			pItemStruct->_rcExpand = rc;
		}
		_DrawItemText(memDC, rcText, pItemStruct->_arrSubItem[0], TRUE);
	}
	else
	{
		_DrawItemText(memDC, rcText, pItemStruct->_pszGroupCaption, LDLISTVIEWCTRL_FONT_DEFAULT_COLOR, BkFontPool::GetFont(BKF_DEFAULTFONT), TRUE);	
	}

}

int CLDListViewCtrl::_HitTest(CPoint pt)
{
	LVHITTESTINFO hitinfo;
	hitinfo.pt = pt;
	int nItem = HitTest(&hitinfo); 

	return nItem;
}

void CLDListViewCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	int nItem = _HitTest(point);
	if (nItem < 0)
		return;

	LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
	if (NULL == lpItemData)
		return;

	if (TRUE == lpItemData->_rcCheckBox.PtInRect(point))
	{
		m_nSelItem= nItem;
	}

	SetMsgHandled(FALSE);

	return ;
}

LRESULT CLDListViewCtrl::OnNMClick(LPNMHDR pnmh)
{
	LPNMLISTVIEW pnms = (LPNMLISTVIEW)pnmh;
	LVHITTESTINFO hitinfo;

	hitinfo.pt = pnms->ptAction;

	int nItem = HitTest(&hitinfo); 
	m_bSelected = FALSE;

	if(nItem != -1)
	{
		LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
		if (NULL == lpItemData)
			return -1;	
		if (TRUE == lpItemData->_rcExpand.PtInRect(hitinfo.pt))
			_ExpandItem(nItem, lpItemData);

		OnLButtonUp(0, hitinfo.pt);//不知道为什么每次响应LBUTTONUP消息很迟钝，都是双击的时候，响应，先吧消息处理放到这里了
	}
	SetMsgHandled(FALSE);
	return 0;
}

void CLDListViewCtrl::_UpdateCheckBoxState(CPoint pt, LPLDListViewCtrl_Item pItemStruct)
{
	if (NULL == pItemStruct || FALSE == pItemStruct->_bShowCheckBox)
		return;

	if (TRUE == pItemStruct->_rcCheckBox.PtInRect(pt))
	{//修改现在的选中状态
		pItemStruct->_bCheck = !pItemStruct->_bCheck;
		InvalidateRect(&pItemStruct->_rcCheckBox);
	}

}

LRESULT CLDListViewCtrl::OnNMDClick(LPNMHDR pnmh)
{
	LPNMLISTVIEW pnms = (LPNMLISTVIEW)pnmh;
	LVHITTESTINFO hitinfo;

	hitinfo.pt = pnms->ptAction;

	int nItem = HitTest(&hitinfo); 

	if(nItem != -1)
	{
		LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
		if (NULL == lpItemData)
			return -1;
		//_ExpandItem(nItem, lpItemData);
		//_UpdateCheckBoxState(hitinfo.pt, lpItemData);
	}
	SetMsgHandled(FALSE);
	return 0;
}

void CLDListViewCtrl::_ExpandItem(int nItem, LPLDListViewCtrl_Item lpItemData)
{
	if (nItem < 0 || NULL == lpItemData)
		return;
	
	if (ITEM_TYPE_TITLE == lpItemData->_enumItemType)
	{//双击点中了一行
		int nCount = GetItemCount();
		lpItemData->_bExpand = !lpItemData->_bExpand;//取反
		_Expand(lpItemData->_uGroup, nItem, lpItemData->_bExpand);
		DWORD dwValue = MAKELPARAM(lpItemData->_bExpand, m_dwCtrlID);
		SendMessage(m_hNotifyHwnd, MSG_USER_LDLISTVIEWCTRL_EXPAND, nItem, dwValue);//展开操作
	}
}
void CLDListViewCtrl::_Expand(int nGroup, int nCurItem, BOOL bExpand)
{
	if (nCurItem < 0 || nGroup < 0)
		return;
	int nCount = GetItemCount();
	
	if (FALSE == bExpand)
	{//收起操作
		_DeleteAnGroupData(nGroup);
	}
	else
	{//展开操作
		size_t count = m_arrListViewItem.GetSize();
		int nIndex = nCurItem;
		for (size_t i = 0; i < count; i++)
		{
			LPLDListViewCtrl_Item pItemData = m_arrListViewItem[i];
			if (NULL != pItemData)
			{
				if (ITEM_TYPE_TITLE != pItemData->_enumItemType && nGroup == pItemData->_uGroup)
				{
					pItemData->_bExpand = bExpand;
					_InsertItemData(++nIndex, pItemData);//重新插入需要展开的数据
				}
			}
		}
	}

}
void CLDListViewCtrl::_InsertItemData(int nIndex, LPLDListViewCtrl_Item lpItemData)
{
	if (nIndex < 0 || NULL == lpItemData)
		return;

	size_t count = lpItemData->_arrSubItem.GetSize();

	for (size_t i = 0; i < count; i++)
	{
		LPLDListViewCtrl_SubItem lpSubItem = lpItemData->_arrSubItem[i];
		if (NULL == lpSubItem)
			continue;

		if ( lpItemData->_enumItemType == ITEM_TYPE_NORMAL && FALSE == lpItemData->_bExpand)
			continue;
		//插入每一行数据 
		if (0 == i)
		{
			CListViewCtrl::InsertItem(nIndex, lpSubItem->_pszSubItemInfo);
		}
		else
		{
			CListViewCtrl::SetItemText(nIndex, i, lpSubItem->_pszSubItemInfo);
		}
	}
	//关联数据
	SetItemData(nIndex, (DWORD_PTR)(lpItemData));
}

void CLDListViewCtrl::_DrawItemText(CDC& memDC, CRect rcItem, LPLDListViewCtrl_SubItem pSubItem, BOOL bTitle/* = FALSE*/)
{
	if (TRUE == rcItem.IsRectEmpty() || NULL == pSubItem)
		return;
	COLORREF colortxt = LDLISTVIEWCTRL_FONT_DEFAULT_COLOR;
	HFONT hFont = pSubItem->_enumFont == FONT_NORMAT ? BkFontPool::GetFont(BKF_DEFAULTFONT):BkFontPool::GetFont(BKF_BOLDFONT);
	if (ITEM_TYPE_NORMAL == pSubItem->_enumType || 
		SUBITEM_TYPE_WITH_IMAGE == pSubItem->_enumType)
	{
		LPTSTR pos = NULL,
			   pos2 = NULL;
		pos = StrStrI(pSubItem->_pszSubItemInfo,STR_SUBITEM_EX_TXT_MARK_BEGIN);
		pos2 = StrStrI(pSubItem->_pszSubItemInfo,STR_SUBITEM_EX_TXT_MARK_END);
		BOOL bExTxtFlag = FALSE;
		if (NULL != pos && NULL != pos2)
			bExTxtFlag = TRUE;

		BOOL bUrlTxtFlag = FALSE;
		pos = StrStrI(pSubItem->_pszSubItemInfo,STR_SUBITEM_URL_TXT_MARK_BEGIN);
		pos2 = StrStrI(pSubItem->_pszSubItemInfo,STR_SUBITEM_URL_TXT_MARK_END);
		if (NULL != pos && NULL != pos2)
			bUrlTxtFlag = TRUE;
		//ITEM中没有需要显示的特殊标签,就直接绘制整个文字吧
		if (FALSE == bExTxtFlag && FALSE == bUrlTxtFlag)
		{
			if (TRUE == bTitle)
				colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : LDLISTVIEWCTRL_FONT_DEFAULT_COLOR;
			else if(MODEL_TITLE == m_ListViewModel)
				colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : LDLISTVIEWCTRL_SUBITEMFONT_COLOR;
			else
				colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : LDLISTVIEWCTRL_FONT_DEFAULT_COLOR;
			

			_DrawItemText(memDC, rcItem, pSubItem->_pszSubItemInfo, colortxt, hFont);

			return;
		}
		LPTSTR pszBuf = StrDup(pSubItem->_pszSubItemInfo);
		LPTSTR pszBak = pszBuf;
		TCHAR pszValue[MAX_PATH] = {0};
		int nIndex = 0;
		CRect rcTxt = rcItem;
		pSubItem->_arrUrlRect.RemoveAll();
		pSubItem->_arrExTxtRect.RemoveAll();
		while(*pszBuf != 0)
		{//金山卫士网络安全公司<EX>金山下属子公司</EX>专注于互联网安全<URL>www.keniu.com</URL>
			
			rcTxt.top = rcItem.top;
			rcTxt.bottom = rcItem.bottom;
			colortxt = LDLISTVIEWCTRL_FONT_DEFAULT_COLOR;
			if (0 == StrCmpNI(pszBuf, STR_SUBITEM_EX_TXT_MARK_BEGIN, _tcslen(STR_SUBITEM_EX_TXT_MARK_BEGIN)))
			{
				LPTSTR posStart = pszBuf + _tcslen(STR_SUBITEM_EX_TXT_MARK_BEGIN);
				
				LPTSTR posEnd = StrStrI(posStart, STR_SUBITEM_EX_TXT_MARK_END);
				*posEnd = 0;
				posEnd += _tcslen(STR_SUBITEM_EX_TXT_MARK_END);
				pszBuf = posEnd;
				if (FALSE == bTitle)
				{
					if (0 == pSubItem->_colorExTxt)
						colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : LDLISTVIEWCTRL_SUBITEMFONT_COLOR;
					else
						colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : pSubItem->_colorExTxt;
				}
				else
					colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : pSubItem->_colorExTxt;

				_DrawItemText(memDC, rcTxt, posStart, colortxt, hFont);
				pSubItem->_arrExTxtRect.Add(rcTxt);
				rcTxt.left = rcTxt.right;
				rcTxt.right = rcItem.right;
				continue;
			}

			if (0 == StrCmpNI(pszBuf, STR_SUBITEM_URL_TXT_MARK_BEGIN, _tcslen(STR_SUBITEM_URL_TXT_MARK_BEGIN)))
			{
				LPTSTR posStart = pszBuf + _tcslen(STR_SUBITEM_URL_TXT_MARK_BEGIN);

				LPTSTR posEnd = StrStrI(posStart, STR_SUBITEM_URL_TXT_MARK_END);
				*posEnd = 0;
				posEnd += _tcslen(STR_SUBITEM_URL_TXT_MARK_END);
				pszBuf = posEnd;
// 				if (FALSE == bTitle)
// 					colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : LDLISTVIEWCTRL_SUBITEMFONT_COLOR;
// 				else
					colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : pSubItem->_colorUrlTxt;

				_DrawItemText(memDC, rcTxt, posStart, colortxt, BkFontPool::GetFont(BKF_UNDERLINE));
				pSubItem->_arrUrlRect.Add(rcTxt);
				rcTxt.left = rcTxt.right;
				rcTxt.right = rcItem.right;
				continue;
			}
			pszValue[0] = *pszBuf;
			rcTxt.top = rcItem.top;
			rcTxt.bottom = rcItem.bottom;

			if (FALSE == bTitle)
			{
				if (MODEL_TITLE == m_ListViewModel)
					colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : LDLISTVIEWCTRL_SUBITEMFONT_COLOR;
				else
					colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : LDLISTVIEWCTRL_FONT_DEFAULT_COLOR;
			}
			else 
				colortxt = TRUE == _GetSelected() ? LDLISTVIEWCTRL_FONT_SEL_COLOR : LDLISTVIEWCTRL_FONT_DEFAULT_COLOR;;

			_DrawItemText(memDC, rcTxt, pszValue, colortxt, hFont);
			rcTxt.left = rcTxt.right;
			rcTxt.right = rcItem.right;
			//下一个
			++pszBuf;
			++nIndex;
		}
		LocalFree(pszBak);//释放


	}
}


void CLDListViewCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	int nSelect = -1;
	LVHITTESTINFO hitinfo;
	hitinfo.pt = point;
	nSelect = HitTest(&hitinfo);
	int nSubItem = -1;
	if (nSelect < 0)
		return;

	if (_PointInUrlRect(nSelect, point, nSubItem) >= 0 ||
		_PointInExTxtRect(nSelect, point) >= 0 ||
		_PointInButtonRect(nSelect, point) >= 0 ||
		_PointInCheckBox(nSelect, point) >= 0 ||
		_PointInExPandRect(nSelect, point) >= 0)
	{
		SetCursor(::LoadCursor(NULL, IDC_HAND));
	}
	else
	{
		m_btnState = BkWndState_Normal;
		SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}
	SetMsgHandled(FALSE);
	return ;
}

int CLDListViewCtrl::_PointInUrlRect(int nItem, CPoint pt, int& nSubItem)
{
	int nRet = -1;

	if (FALSE == m_bUrlHand)
		return nRet;

	LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
	if (NULL == lpItemData)
		return nRet;
	size_t count = lpItemData->_arrSubItem.GetSize();
	for (size_t i = 0; i < count; i++)
	{
		LPLDListViewCtrl_SubItem lpSubItemData = lpItemData->_arrSubItem[i];
		if (NULL == lpSubItemData)
			continue;
		size_t countUrl = lpSubItemData->_arrUrlRect.GetSize();
		for (size_t url = 0; url < countUrl; url++)
		{
			if (lpSubItemData->_arrUrlRect[url].PtInRect(pt))
			{
				nSubItem = i;
				return url;
			}
		}
	}
	return nRet;
}
int CLDListViewCtrl::_PointInExTxtRect(int nItem, CPoint pt)
{
	int nRet = -1;

	if (FALSE == m_bExTxtHand)
		return nRet;
	
	LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
	if (NULL == lpItemData)
		return nRet;
	size_t count = lpItemData->_arrSubItem.GetSize();
	for (size_t i = 0; i < count; i++)
	{
		LPLDListViewCtrl_SubItem lpSubItemData = lpItemData->_arrSubItem[i];
		if (NULL == lpSubItemData)
			continue;

		size_t countTxt = lpSubItemData->_arrExTxtRect.GetSize();
		for (size_t txt = 0; txt < countTxt; txt++)
		{
			if (lpSubItemData->_arrExTxtRect[txt].PtInRect(pt))
			{
				return txt;
			}
		}
	}
	return nRet;
}

void CLDListViewCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	int nSelect = -1;
	LVHITTESTINFO hitinfo;
	hitinfo.pt = point;
	nSelect = HitTest(&hitinfo);
	if (nSelect < 0)
		return;

	LPLDListViewCtrl_Item lpItemData = _GetItemData(nSelect);
	if (NULL == lpItemData)
		return;
	if ( nSelect == m_nSelItem)
	{//当鼠标按下和抬起是在同一行的时候，才改变CHECKBOX的状态
		_UpdateCheckBoxState(point, lpItemData);
		__super::SetCheckState(nSelect, lpItemData->_bCheck);
	}

	
	int nSubItem = -1;
	int nUrlRect = _PointInUrlRect(nSelect, point, nSubItem),
		nExTxtRect = _PointInExTxtRect(nSelect, point),
		nBtnRect = _PointInButtonRect(nSelect, point);
	if (nUrlRect >= 0 ||
		nExTxtRect >= 0 ||
		nBtnRect >= 0)
	{
		if (nUrlRect >= 0)
		{
			_OnLButtonUpOnAnUrl(nSelect, nUrlRect, nSubItem);
		}else if (nExTxtRect >= 0)
		{
			_OnLButtonUpOnAnExTxt(nSelect, nExTxtRect);
		}else if (nBtnRect >= 0)
		{
			_OnLButtonUpOnAnButton(nSelect, nBtnRect);
		}
		SetCursor(::LoadCursor(NULL, IDC_HAND));
	}
	else
	{
		SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	SetMsgHandled(FALSE);

}

void CLDListViewCtrl::_OnLButtonUpOnAnUrl(int nItem, int nIndexUrl, int nSubItem)
{
	if (nItem < 0 || nIndexUrl < 0)
		return;

	LPTSTR pszUrl = _GetAnUrl(nItem, nIndexUrl);
	if (NULL != pszUrl)
	{
		ShellExecute(NULL, TEXT("open"), pszUrl, NULL, NULL, SW_SHOW);
	}
	if (TRUE == ::IsWindow(m_hNotifyHwnd))
	{
		DWORD dwValue = MAKELPARAM(nIndexUrl, m_dwCtrlID);
		DWORD dwItem = MAKELPARAM(nItem,nSubItem);
		SendMessage(m_hNotifyHwnd, MSG_USER_LDLISTVIEWCTRL_LBP_URL, dwItem, dwValue);
	}
}

void CLDListViewCtrl::_OnLButtonUpOnAnExTxt(int nItem, int nIndexExTxt)
{
	if (nItem < 0 || nIndexExTxt < 0)
		return;

	if (TRUE == ::IsWindow(m_hNotifyHwnd))
		SendMessage(m_hNotifyHwnd, MSG_USER_LDLISTVIEWCTRL_LBP_EXTXT, nItem, nIndexExTxt);
}

LPTSTR CLDListViewCtrl::_GetAnUrl(int nItem, int nIndexUrl)
{
	LPTSTR pszValue = NULL;
	if (nItem < 0 || nIndexUrl < 0)
		return pszValue;

	LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
	
	if (NULL == lpItemData)
		return pszValue;

	int count = lpItemData->_arrSubItem.GetSize();
	for (int i = 0; i < count; i++)
	{
		LPLDListViewCtrl_SubItem lpSubItem = lpItemData->_arrSubItem[i];
		if (NULL == lpItemData)
			continue;
		int countUrl = lpSubItem->_arrUrl.GetSize();
		if (nIndexUrl >= countUrl)
			return pszValue;
		return const_cast<LPTSTR>(lpSubItem->_arrUrl[nIndexUrl]);
	}

	return pszValue;
}

HWND CLDListViewCtrl::SetNotifyHwnd(HWND hWnd)
{
	HWND wnd = m_hNotifyHwnd;
	m_hNotifyHwnd = hWnd;

	return wnd;
}

HWND CLDListViewCtrl::Create(HWND hWndParent, _U_RECT rect /* = NULL */, LPCTSTR szWindowName /* = NULL */, DWORD dwStyle /* = 0 */, DWORD dwExStyle /* = 0 */, _U_MENUorID MenuOrID /* = 0U */, LPVOID lpCreateParam /* = NULL */)
{
	HWND hWnd = __super::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
	__super::SetExtendedListViewStyle( LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|__super::GetExtendedListViewStyle() );
	
	if (NULL == hWnd)
		return NULL;

	m_dwCtrlID = reinterpret_cast<DWORD&>(MenuOrID);

 	if(FALSE == (dwStyle & LVS_NOCOLUMNHEADER) && NULL != GetHeader().m_hWnd)
	{
 		m_ListViewHeader.SubclassWindow(GetHeader().m_hWnd);
		m_ListViewHeader.SetHeight(25);
	}

	return hWnd;
}

//设置列标题高度
ULONG CLDListViewCtrl::SetHeaderHeight(ULONG uHeight)
{
	//ULONG uHeight = m_ListViewHeader.GetWindowContextHelpId()
	m_ListViewHeader.SetHeight(uHeight);
	return 0;
}

void CLDListViewCtrl::ExpandItem(int nItem, BOOL bExpand)
{
	if (nItem < 0)
		return;

	LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
	if (NULL == lpItemData)
		return ;
	
	if (bExpand == lpItemData->_bExpand)//如果当前展开状态已经是需要的状态了，就不用再管了
		return;

	_ExpandItem(nItem, lpItemData);
}

//设置选中状态
BOOL CLDListViewCtrl::SetCheckState(int nItem, BOOL bCheck /* = TRUE */)
{
	BOOL bFlag = FALSE;
	LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
	if (NULL == lpItemData)
		return bFlag;

	bFlag = lpItemData->_bCheck;

	lpItemData->_bCheck = bCheck;

	__super::SetCheckState(nItem, bCheck);
	
	return bFlag;
}

BOOL CLDListViewCtrl::GetCheckState(int nIndex)
{
	BOOL bFlag = FALSE;
	if (nIndex < 0)
		return bFlag;

	LPLDListViewCtrl_Item lpItemData = _GetItemData(nIndex);
	if (NULL == lpItemData)
		return bFlag;
	
	bFlag = lpItemData->_bCheck;
	return bFlag;
}

void CLDListViewCtrl::_DeleteAnGroupData(int nGroup)
{
	if (nGroup < 0)
		return;

	int nCount = GetItemCount();

	for (int i = nCount-1; i >= 0; i--)
	{
		LPLDListViewCtrl_Item pItemData = _GetItemData(i);
		if (NULL == pItemData)
		{
			continue;
			//DeleteItem(nCurItem);
		}
		if (ITEM_TYPE_TITLE != pItemData->_enumItemType && nGroup == pItemData->_uGroup)
		{
			__super::DeleteItem(i);;
		}
	}
}

BOOL CLDListViewCtrl::SetUrlHand(BOOL bHand)
{
	BOOL bFlag = m_bUrlHand;
	m_bUrlHand = bHand;
	
	return bFlag;
}


BOOL CLDListViewCtrl::SetExTxtHand(BOOL bHand)
{
	BOOL bFlag = m_bExTxtHand;
	m_bExTxtHand = bHand;

	return bFlag;
}

ENUM_LISTVIEW_MODEL CLDListViewCtrl::SetListViewModel(ENUM_LISTVIEW_MODEL model)
{
	ENUM_LISTVIEW_MODEL mdl = m_ListViewModel;
	m_ListViewModel = model;

	return mdl;
}

BOOL CLDListViewCtrl::SetSplitLines(BOOL bSplitLines)
{
	BOOL bFlag = m_bSplitLines;
	m_bSplitLines = bSplitLines;

	return bFlag;
}

BOOL CLDListViewCtrl::SetFullRowUpdateCheckBox(BOOL bFullRow /* = FALSE */)
{
	BOOL bFlag = m_bItemUpdateCheckBox;
	m_bItemUpdateCheckBox = bFullRow;

	return bFlag;
}

BOOL CLDListViewCtrl::SetSelState(BOOL bSet /* = TRUE */)
{
	BOOL bFlag = m_bSelState;
	m_bSelState = bSet;

	return bFlag;
}

int CLDListViewCtrl::PushSubItemImage(DWORD dwImageID)
{
// 	HBITMAP hBmp = NULL;
// 	 hBmp = BkBmpPool::GetBitmap(dwImageID);
// 	 if (NULL != hBmp)
// 		 m_arrImage.Add(hBmp);
// 
//	 return m_arrImage.GetSize();

	return 0;
}

int CLDListViewCtrl::_DrawImageSubItem(CDC& memDC, int nItem, int nSubItem, LPLDListViewCtrl_SubItem pItemStruct)
{
	int nRet = -1;
	CRect rcSubItem;
	GetSubItemRect(nItem, nSubItem,LVIR_BOUNDS/*LVIR_LABEL*/, rcSubItem);
	if (TRUE == rcSubItem.IsRectEmpty() || NULL == pItemStruct)
		return nRet;

/*
	BITMAP bitmap;
	m_nSubItemWidth = 1;
	if (0 == m_nSubItemWidth)
	{//
		if (pItemStruct->_ImageID >= m_arrImage.GetSize())
			pItemStruct->_ImageID = 0;


		GetObject(m_hSubItemImage, sizeof(BITMAP), &bitmap);
		CDC tmpDC;
		tmpDC.CreateCompatibleDC(memDC.m_hDC);
		SelectObject(tmpDC.m_hDC, m_arrImage[pItemStruct->_ImageID]);
		memDC.BitBlt(rcSubItem.left+5, rcSubItem.top+3, m_nSubItemWidth, m_nSubItemWidth, tmpDC.m_hDC, m_nSubItemWidth, bitmap.bmHeight, SRCCOPY);

		nRet = 0;
	}
	else
	{
		
		if (NULL == m_hSubItemImage)
			return nRet;

		GetObject(m_hSubItemImage, sizeof(BITMAP), &bitmap);
		if (pItemStruct->_ImageID > bitmap.bmWidth/m_nSubItemWidth)//如果给出的ID超出了范围，就是用第一个
			pItemStruct->_ImageID = 0;

		CDC tmpDC;
		tmpDC.CreateCompatibleDC(memDC.m_hDC);
		SelectObject(tmpDC.m_hDC, m_hSubItemImage);
		memDC.StretchBlt(rcSubItem.left+5, rcSubItem.top+3, m_nSubItemWidth, m_nSubItemWidth, tmpDC.m_hDC, pItemStruct->_ImageID*m_nSubItemWidth, 0, m_nSubItemWidth, bitmap.bmHeight, SRCCOPY);
		
		nRet = 0;
	}
*/
	if (pItemStruct->_ImageID < 0)
		return nRet;

	if (NULL == m_pImageSkin)
		m_pImageSkin = BkSkin::GetSkin("exam_stat15");

	CRect rc(rcSubItem.left+5, rcSubItem.top+5, m_nSubItemWidth, m_nSubItemWidth);
	m_pImageSkin->Draw(memDC.m_hDC, rc, pItemStruct->_ImageID);
	nRet = 0;
	

	return nRet;
}

int CLDListViewCtrl::_DrawButtonSubItem(CDC& memDC, int nItem, int nSubItem, LPLDListViewCtrl_SubItem pItemStruct)
{
	int nRet = -1;
	CRect rcSubItem;
	GetSubItemRect(nItem, nSubItem,LVIR_BOUNDS/*LVIR_LABEL*/, rcSubItem);
	if (TRUE == rcSubItem.IsRectEmpty() || NULL == pItemStruct)
		return nRet;

	CRect rcTxt = rcSubItem;

	::DrawText(memDC.m_hDC, pItemStruct->_pszSubItemInfo, _tcslen(pItemStruct->_pszSubItemInfo), &rcTxt, DT_VCENTER|DT_SINGLELINE|DT_CENTER|DT_CALCRECT);

	if (NULL == m_pBtnSkin)
		m_pBtnSkin = BkSkin::GetSkin("normalbtn");

	CRect rc = rcTxt;
	rc.InflateRect(3, 0, 3, 0);
	//draw button
	m_pBtnSkin->Draw(memDC.m_hDC, rc, m_btnState/*(FALSE) ? BkWndState_Normal : BkWndState_Hover*/);

	if (FALSE == m_mapBtnRect.SetAt(nItem, rc))
		m_mapBtnRect.Add(nItem, rc);

	::DrawText(memDC.m_hDC, pItemStruct->_pszSubItemInfo, _tcslen(pItemStruct->_pszSubItemInfo), &rcTxt, DT_VCENTER|DT_SINGLELINE|DT_CENTER);

	return nRet;
}

HBITMAP CLDListViewCtrl::SetSubItemImage(DWORD dwImageID, int subItemImageWidth)
{
	m_hSubItemImage = BkBmpPool::GetBitmap(dwImageID);
	if (NULL == m_hSubItemImage)
		return m_hSubItemImage;
	m_nSubItemWidth = subItemImageWidth;

	return m_hSubItemImage;
}

int CLDListViewCtrl::_PointInButtonRect(int nItem, CPoint point)
{
	int nRet = -1;
	CRect rc = m_mapBtnRect.Lookup(nItem);

	if (TRUE == rc.PtInRect(point))
	{
		nRet = nItem;
		m_btnState = BkWndState_Hover;
	}

	return nRet;
}

void CLDListViewCtrl::_OnLButtonUpOnAnButton(int nItem, int nIndexRext)
{
	if (nItem < 0 || nIndexRext < 0)
		return;
//	::MessageBox(NULL, TEXT("测试：_OnLButtonUpOnAnButton"), NULL, 1);

}

int CLDListViewCtrl::_DrawImageSubItem2(CDC& memDC, CRect rc, CRect& ImageRc, LPLDListViewCtrl_SubItem pSubItem)
{
	int nRet = -1;

	if (NULL == pSubItem || pSubItem->_ImageID < 0 || TRUE == rc.IsRectEmpty())
		return nRet;

	if (NULL == m_pImageSkin)
		m_pImageSkin = BkSkin::GetSkin("exam_stat15");
	if (NULL == m_pImageSkin)
		return nRet;

	CSize size = m_pImageSkin->GetSkinSize();
	CRect rcImage(rc.left+5, rc.top+5, 0, 0);
	rcImage.right = rcImage.left + size.cx;
	rcImage.bottom = rcImage.top + size.cy;
	m_pImageSkin->Draw(memDC.m_hDC, rcImage, pSubItem->_ImageID);
	ImageRc = rcImage;
	nRet = 0;

	return nRet;
}

int CLDListViewCtrl::MoveTo(int nOldID, int nNewID)
{
	int nRet = -1;
	LPLDListViewCtrl_Item pItemData = (LPLDListViewCtrl_Item)GetItemData(nOldID);
	if (NULL == pItemData)
		return nRet;

	__super::DeleteItem(nOldID);
	 _InsertItemData(nNewID, pItemData);
	 nRet = 0;

	return nRet;
}
COLORREF CLDListViewCtrl::SetBkColor(COLORREF bkColor)
{
	COLORREF color = m_bkColor;

	m_bkColor = bkColor;
	__super::SetBkColor(bkColor);

	return color;
}

BOOL CLDListViewCtrl::SetCheckBoxHand(BOOL bHand)
{
	BOOL bFlag = m_bCheckBoxHand;
	m_bCheckBoxHand = bHand;

	return bFlag;
}
BOOL CLDListViewCtrl::SetExpandRectHand(BOOL bHand)
{
	BOOL bFlag = m_bExpandRectHand;
	m_bExpandRectHand = bHand;

	return bFlag;
}

int CLDListViewCtrl::_PointInCheckBox(int nItem, CPoint point)
{
	int nRet = -1;
	if (FALSE == m_bCheckBoxHand)
		return nRet;

	LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
	if (NULL == lpItemData)
		return nRet;

	if (TRUE == lpItemData->_rcCheckBox.PtInRect(point))
		nRet = 0;

	return nRet;
}
int CLDListViewCtrl::_PointInExPandRect(int nItem, CPoint point)
{
	int nRet = -1;
	if (FALSE == m_bExpandRectHand)
		return nRet;

	LPLDListViewCtrl_Item lpItemData = _GetItemData(nItem);
	if (NULL == lpItemData || FALSE == lpItemData->_bUserExpand)
		return nRet;

	if (TRUE == lpItemData->_rcExpand.PtInRect(point))
		nRet = 0;

	return nRet;
}

BOOL CLDListViewCtrl::OnEraseBkgnd(CDCHandle dc)
{
	int nTop = GetTopIndex();
	int nBottom = nTop + GetCountPerPage();

	nBottom = min(nBottom, GetItemCount() - 1);

	CRect rcClient, rcItemTop, rcItemBottom;
	COLORREF crBg = IsWindowEnabled() ? m_bkColor : RGB(0xDD, 0xDD, 0xDD);

	GetClientRect(rcClient);
	GetItemRect(nTop, rcItemTop, LVIR_BOUNDS);
	GetItemRect(nBottom, rcItemBottom, LVIR_BOUNDS);

	dc.FillSolidRect(rcItemBottom.right, rcItemTop.top, rcClient.right - rcItemBottom.right, rcClient.bottom - rcItemTop.top, crBg);
	dc.FillSolidRect(rcClient.left, rcItemBottom.bottom, rcItemBottom.Width(), rcClient.bottom - rcItemBottom.bottom, crBg);

	SetMsgHandled(FALSE);
	return TRUE;
}

void CLDListViewCtrl::OnVScroll(UINT nSBCode, UINT nPos, HWND hWnd)
{
	if (SB_TOP == nSBCode ||
		SB_ENDSCROLL == nSBCode)
	{
		Invalidate();
	}
	SetMsgHandled(FALSE);
}
LRESULT CLDListViewCtrl::OnMouseWhell(UINT fwKey, short zDela,CPoint pt)
{
// 	INT ptMin, ptMax;
// 	::GetScrollRange(m_hWnd, SB_VERT, &ptMin, &ptMax);
// 	if (ptMax == pt.y || ptMin == pt.y)
	{
		Invalidate();
	}
	SetMsgHandled(FALSE);

	return 0;
}