/* -------------------------------------------------------------------------
//	FileName	：	CKavListViewEx.h
//	Brief		：	一个自绘的listctrl。
					父窗口需要添加 REFLECT_NOTIFICATIONS();
//
//	$Id: $
// -----------------------------------------------------------------------*/
#ifndef __CLOGLISTCTRL_H__
#define __CLOGLISTCTRL_H__

#include <vector>
#include "headerctrl.h"
#include "wtlhelper/whwindow.h"
// -------------------------------------------------------------------------
/////////////////////////////////////
#define WM_ITEM_BUTTON_CLICK		WM_USER+123		//当子项中存在按钮，被点击时会向父窗口发送此消息。WPARAM表示行，LPARAM表示列。




///////////////////////////////////////////////
class CKavListViewEx;
typedef CSortListViewCtrlImpl<CKavListViewEx> CLogListCtrlBase;
//////////////////////////////////////////////

//每个子项的格式化串
#define FORMAT_ALIGN_LEFT		0x00		//文字左对齐
#define FORMAT_ALIGN_CENTER		0x01		//文字居中
#define FORMAT_BUTTON			0x02		//子项是一个按钮项
#define FORMAT_BUTTON_TEXT		0x03		//子项是一个文字按钮项

//每个子项的数据
typedef struct tagSubItemData
{
	CString		strText;
	COLORREF	crText;
	BOOL		bTextBold;
	int			nFormat;
	tagSubItemData()
	{
		Reset();
	}
	void Reset()
	{
		strText = L"";
		crText = RGB(0,0,0);
		nFormat = FORMAT_ALIGN_LEFT;
		bTextBold = FALSE;
	}
}SubItemData;

//每一行的数据
typedef struct tagItemData
{
	std::vector<SubItemData> subItems;
}ItemData;

//一个list的数据
typedef std::vector<ItemData> ListItemData;

class CKavListViewEx
	: public CLogListCtrlBase
	, public COwnerDraw<CKavListViewEx>
{

public:

	DECLARE_WND_CLASS(L"CLogListCtrl")

	CKavListViewEx()
		:m_nRowHeight(-1)
		,m_nDwonBtn(-1)
		,m_nBtnWidth(0)
		,m_nBtnHeight(0)
	{
		Init();
	};

	~CKavListViewEx()
	{
		Uninit();
	};

	BEGIN_MSG_MAP(CKavListViewEx)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		//MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		//MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CKavListViewEx>,1)
		CHAIN_MSG_MAP(CLogListCtrlBase)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	static DWORD GetWndStyle(DWORD dwStyle)
	{
		return dwStyle;
	}
	static DWORD GetWndExStyle(DWORD dwExStyle)
	{
		// 取消3D扩展风格。
		return dwExStyle &~WS_EX_CLIENTEDGE;
	}

/////////////////////////////////
//list的头部控制

	//在需要控制头部外观时，先调用这个函数。
	void HeaderSubclassWindow()
	{
		if(m_wndHeader.m_hWnd != NULL)
		{
			m_wndHeader.DestroyWindow();
			m_wndHeader.m_hWnd = NULL;
		}
		CHeaderCtrl wndHeader = GetHeader();
		if(wndHeader.m_hWnd != NULL)
		{
			m_wndHeader.SubclassWindow(wndHeader.m_hWnd);
		}
	}

	void SetHeaderHeight(int nHeight)
	{
		if(m_wndHeader.m_hWnd)
		{
			//m_wndHeader.SetHeaderHeight(nHeight);
		}
	}

	void SetHeaderFont( HFONT hFont )
	{
		if( hFont )
		{
			//m_wndHeader.SetHeaderFont( hFont );
		}
		else
		{
			LOGFONT lf;
			memset( &lf, 0, sizeof(lf) );
			_tcscpy( lf.lfFaceName, TEXT("宋体") );
			lf.lfHeight = 12;
			lf.lfUnderline = FALSE;
			lf.lfWeight = 400;
			CFont font;
			font.CreateFontIndirect( &lf );
			//m_wndHeader.SetHeaderFont( font.Detach() );
		}
	}

	void SetHeaderTextColor(COLORREF clr)
	{
		//m_wndHeader.SetHeaderTextColor(clr);
	}

	void SetHeaderBg(HBITMAP hBmp,COLORREF clrDiv)
	{
		//m_wndHeader.SetHeaderBg(hBmp,clrDiv,RGB(0,0,0));
	}
/////////////////////////////////

	LRESULT OnDrawItem(UINT , WPARAM , LPARAM lParam, BOOL& bHandled)
	{
		LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
		return 0;
	};

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItem)
	{
		CDCHandle dc(lpDrawItem->hDC);
		CRect rc(lpDrawItem->rcItem);
		int nSelected = GetSelectedIndex();
		int nColumnCount = GetColumnCount();
		int nItemID = lpDrawItem->itemID;
		HICON hIcon = NULL;
		COLORREF dwTextColor = RGB(0, 0, 0);

		// 绘制背景
		if ((UINT)nSelected == lpDrawItem->itemID)
			dc.FillSolidRect(rc, RGB(226,248,202));
		else
			dc.FillSolidRect(rc, RGB(0xFF, 0xFF, 0xFF));
		
		//每行底部的分隔线
		CRect rcDivRow = rc;
		rcDivRow.top = rcDivRow.bottom - 1;
		dc.FillSolidRect(rcDivRow, RGB(246, 246, 246));

		if (nItemID >= 0 && nItemID < m_Data.size())
		{
			// 防止m_Data被意外删除就先复制一份。by ZC. 2010-10-14.
			ItemData _kItemData = m_Data[nItemID];

			int nLeft = rc.left;
			int nWidth = 0;
			for(int i=0; i< _kItemData.subItems.size(); i++)
			{
				nWidth = GetColumnWidth(i);
				RECT rcSubItem = { nLeft, rc.top, nLeft+nWidth, rc.bottom };

				SubItemData&  subItem = _kItemData.subItems[i];

				if((subItem.nFormat&FORMAT_BUTTON)!=0 ) //子项是一个按钮
				{
					if (subItem.nFormat == FORMAT_BUTTON_TEXT)
					{
						// 这里不处理。
					}
					else
					{
						CRect rect;
						rect.left = nLeft + (nWidth - m_nBtnWidth)/2;
						rect.right = rect.left + m_nBtnWidth;
						rect.top = rc.top + (rc.Height() - m_nBtnHeight)/2;
						rect.bottom = rect.top + m_nBtnHeight;

						DrawBmp(dc,rect,m_bmpBtnNormal);
					}
				}

				//文字
				UINT nFormat = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
				if((subItem.nFormat&FORMAT_ALIGN_CENTER) == 1)
					nFormat |= DT_CENTER;
				else
					nFormat |= DT_LEFT;

				if(subItem.strText.GetLength() > 0)
				{
					BOOL lfUnderline = FALSE;
					COLORREF crOldText = dc.SetTextColor(subItem.crText);

					if (FORMAT_BUTTON_TEXT == subItem.nFormat)
						lfUnderline = TRUE;

					HFONT hOldFont = dc.SelectFont(GetFont(subItem.bTextBold, lfUnderline));

					dc.DrawText(subItem.strText,
								subItem.strText.GetLength(),
								&rcSubItem,
								nFormat);

					dc.SetTextColor(crOldText);
					::DeleteObject(dc.SelectFont(hOldFont));

				}
				nLeft += nWidth;
			}
		}
	}

	void MeasureItem(LPMEASUREITEMSTRUCT pMeasure)
	{
		if(m_nRowHeight > 0)
		{
			pMeasure->itemHeight = m_nRowHeight;
		}
		else
		{
			COwnerDraw<CKavListViewEx>::MeasureItem(pMeasure);
		}
	}

	int SetRowHeight(int nHeight)
	{
		int nOldHeight = m_nRowHeight;
		m_nRowHeight = nHeight;
		return nOldHeight;
	}

	int InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat = LVCFMT_LEFT, 
		int nWidth = -1, int nSubItem = -1, int iImage = -1, int iOrder = -1)
	{
		int nRetCode = CLogListCtrlBase::InsertColumn(nCol,lpszColumnHeading,nFormat,nWidth,
			nSubItem,iImage,iOrder);

		if(nRetCode != -1)
		{
			HeaderItem headerItem;
			headerItem.strText = lpszColumnHeading;
			headerItem.nFlag = DT_CENTER;
			//m_wndHeader.AddHDItem(headerItem);
		}

		return nRetCode;
	}

	int InsertColumn(int nCol, const LVCOLUMN* pColumn)
	{
		int nRetCode = CLogListCtrlBase::InsertColumn(nCol,pColumn);

		if(nRetCode != -1)
		{
			HeaderItem headerItem;
			headerItem.strText = pColumn->pszText;
			headerItem.nFlag = DT_CENTER;
			//m_wndHeader.AddHDItem(headerItem);
		}

		return nRetCode;
	}

	int InsertItem(ItemData& pItem)
	{
		m_Data.push_back(pItem);
		CLogListCtrlBase::InsertItem(m_Data.size()-1,L"");
		return m_Data.size();
	}

	int InsertItem(int nCount = -1)
	{
		ItemData data;
		for (int i = 0; i < GetColumnCount(); i++)
		{
			SubItemData dataName;
			data.subItems.push_back(dataName);
		}

		m_Data.push_back(data);
		CLogListCtrlBase::InsertItem(m_Data.size()-1, L"");
		return m_Data.size();
	}

	//注：基类的GetItemText(...)不可用。因为数据并没有设置到基类中去。

	BOOL SetSubItem(int nRow, int nSub, SubItemData& subItem)
	{
		if(m_Data.size() > nRow)
		{
			if(m_Data[nRow].subItems.size() > nSub)
			{
				m_Data[nRow].subItems[nSub] = subItem;
				return TRUE;
			}
		}

		return FALSE;
	}

	BOOL SetSubItemText(int nRow,int nSub,const CString& strText, int nFormat = FORMAT_ALIGN_CENTER)
	{
		if(m_Data.size() > nRow)
		{
			if(m_Data[nRow].subItems.size() > nSub)
			{
				m_Data[nRow].subItems[nSub].strText = strText;
				m_Data[nRow].subItems[nSub].nFormat = nFormat;
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL GetSubItemText(int nRow,int nSub,CString& strText)
	{
		if(m_Data.size() > nRow)
		{
			if(m_Data[nRow].subItems.size() > nSub)
			{
				strText = m_Data[nRow].subItems[nSub].strText;
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL SetSubItemFormat(int nRow,int nSub,int nFormat)
	{
		if(m_Data.size() > nRow)
		{
			if(m_Data[nRow].subItems.size() > nSub)
			{
				m_Data[nRow].subItems[nSub].nFormat = nFormat;
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL SetSubItemColor(int nRow,int nSub,COLORREF crText)
	{
		if(m_Data.size() > nRow)
		{
			if(m_Data[nRow].subItems.size() > nSub)
			{
				m_Data[nRow].subItems[nSub].crText = crText;
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL SetSubItemTextBold(int nRow,int nSub,BOOL bBold)
	{
		if(m_Data.size() > nRow)
		{
			if(m_Data[nRow].subItems.size() > nSub)
			{
				m_Data[nRow].subItems[nSub].bTextBold = bBold;
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL DeleteItem(LPDELETEITEMSTRUCT lpItem)
	{
		return TRUE;
	}

	BOOL DeleteItem(int nItem)
	{
		if (nItem >= 0 && nItem < m_Data.size())
		{
			// 先删除实际项，再删除内存中数据。
			__super::DeleteItem(nItem);

			m_Data.erase(m_Data.begin() + nItem);

			return TRUE;
		}

		return FALSE;
	}

	BOOL DeleteAllItems()
	{
		m_Data.clear();
		return CLogListCtrlBase::DeleteAllItems();
	}

	DWORD_PTR GetItemDataEx(int nRow)
	{
		if(m_Data.size() > nRow)
		{
			return (DWORD_PTR)&m_Data[nRow];
		}

		return NULL;
	}
	
	HFONT GetFont(BOOL bBold, BOOL lfUnderline = FALSE)
	{
		CFont destFont;

		CFontHandle srcFont(m_font);
		LOGFONT lf;
		srcFont.GetLogFont(&lf);
		lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
		lf.lfUnderline = lfUnderline;

		destFont.CreateFontIndirect(&lf);
		return (HFONT)destFont.Detach();
	}

	LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
	{
		int nItem,nSubItem;
		RECT rc;
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		if (IsMouseInButton(pt, nItem,nSubItem))
		{
			SetCursor(LoadCursor(NULL, IDC_HAND));
			//InvalidateRect(&rc);
		}
		bHandled = TRUE;
		return 0;
	}

	LRESULT OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
	{
		int nItem,nSubItem;
		RECT rc;
		GetWindowRect(&rc);
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		if (IsMouseInButton(pt, nItem,nSubItem))
		{
			//::PostMessage(GetParent(), WM_ITEM_BUTTON_CLICK, (WPARAM)nItem,(LPARAM)nSubItem);
			// 金山卫士界面中间层有个view窗体！
			::PostMessage(GetParent().GetParent(), WM_ITEM_BUTTON_CLICK, (WPARAM)nItem,(LPARAM)nSubItem);
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnLButtonUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
	{
		int nItem,nSubItem;
		RECT rc;
		GetWindowRect(&rc);
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		if (IsMouseInButton(pt, nItem,nSubItem))
		{
			
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		bHandled = FALSE;
		return 0;
	}


	BOOL IsMouseInButton(CPoint pt, int& nItem ,int & nSubItem)
	{
		BOOL retval = FALSE;
		nItem = -1;
		int nColumnCount = GetColumnCount();


		int nItemT = HitTest(pt, NULL);
		int nSubItemT = 0;
		BOOL bFind = FALSE;
		if (nItemT != -1)
		{
			int nLeft = 0;
			int nWidth = 0;
			SCROLLINFO hInfo;
			hInfo.cbSize = sizeof(SCROLLINFO);
			hInfo.fMask = SIF_ALL;
			GetScrollInfo(SB_HORZ,&hInfo);
			if(hInfo.nPos > 0)
			{
				//这里，一个水平滚动条的位置，正好是一个像素。
				nLeft -= hInfo.nPos;
			}
			for(nSubItemT=0; nSubItemT<m_Data[nItemT].subItems.size(); nSubItemT++)
			{
				nWidth = GetColumnWidth(nSubItemT);
				if(m_Data[nItemT].subItems[nSubItemT].nFormat&FORMAT_BUTTON)
				{
					//只要x轴在范围内就可以了，以后再考虑y轴吧
					int nMin = nWidth>m_nBtnWidth ? nLeft+(nWidth-m_nBtnWidth)/2 : nLeft;
					int nMax = nWidth>m_nBtnWidth ? nMin + m_nBtnWidth : nMin+nWidth;
					if(pt.x>=nMin && pt.x<=nMax)
					{
						bFind = TRUE;
						break;
					}
				}
				nLeft += nWidth;
			}
		}
		if(bFind)
		{
			nItem = nItemT;
			nSubItem = nSubItemT;
		}

		return bFind;
	}

	int GetItemCount()
	{
		return m_Data.size();
	}

protected:
	void Init()
	{
		LOGFONT lf;
		CFontHandle font = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
		font.GetLogFont(&lf);
		_tcscpy( lf.lfFaceName, TEXT("宋体") );
		lf.lfWeight = FW_NORMAL;
		lf.lfHeight = -12;

		m_font.CreateFontIndirect(&lf);
		font.DeleteObject();

//		m_bmpBtnNormal.LoadBitmap(IDB_SUBITEM_NORAML);
		//m_bpmBtnDwon.LoadBitmap(IDB_SUBITEM_DOWN);
//		BITMAP bmpInfo;
//		m_bmpBtnNormal.GetBitmap(&bmpInfo);
//		m_nBtnWidth = bmpInfo.bmWidth;
//		m_nBtnHeight = bmpInfo.bmHeight;
	}
	void Uninit()
	{
		m_Data.clear();
	}

protected:
	ListItemData	m_Data;
	CFont			m_font;
	CBitmap			m_bmpBtnNormal;
	//CBitmap			m_bpmBtnDwon;
	int				m_nDwonBtn;
	int				m_nBtnWidth;
	int				m_nBtnHeight;
	int				m_nRowHeight;
	CWHHeaderCtrl	m_wndHeader;
};

// -------------------------------------------------------------------------
// $Log: $

#endif /* __CREPARILISTCTRL_H__ */
