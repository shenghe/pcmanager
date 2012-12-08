
#pragma once

//

/////////////////////////////////////////////////////////////////////////////
// CDirectoryListCtrl window
#include "wtlctrls/listheader.h"


typedef struct  {

    
    CString strCaption;
    ULONGLONG ulSize;
    int nIndex;
    CString strDesc;

} DIR_LIST_ITEM;

class CDirectoryListCtrl : public CWindowImpl<CDirectoryListCtrl,CListViewCtrl>,
    public COwnerDraw<CDirectoryListCtrl>
{
// Construction
public:
	CDirectoryListCtrl();
    virtual ~CDirectoryListCtrl();

    typedef std::vector<DIR_LIST_ITEM> CDirItemArray;
// Attributes

public:

    COLORREF m_clrSelect;
    COLORREF m_clrSelectBorder;
    COLORREF m_clrTextColor;
    COLORREF m_clrSelectTextColor;
    COLORREF m_clrBorder;
    CImageList m_imagelist;
    HICON m_hIconRegedit;
    CDirItemArray m_itemArray;
    HWND m_hwParent;

    void AutoColumnWidth();
    void AddItem(LPCTSTR pszName, ULONGLONG ulSize, int nIndex,CString strDesc);
    BOOL ItemExists(int ndirID)
    {
        BOOL bFind = FALSE;
        for(CDirItemArray::iterator iter = m_itemArray.begin() ;iter!=m_itemArray.end();iter++)
        {
            if(iter->nIndex == ndirID)
            {
               bFind = TRUE;
               break;
            }
        }
        return bFind;
    }

    void ChangeItem(int dirIndex,ULONGLONG ulSize)
    {
        int nIndex = 0;
        for(CDirItemArray::iterator iter = m_itemArray.begin() ;iter!=m_itemArray.end();iter++,nIndex++)
        {
            if(iter->nIndex == dirIndex)
            {
                if (ulSize == 0 && dirIndex != 0)
                {
                    m_itemArray.erase(iter);
                    DeleteItem(nIndex);
                    RedrawItems(0,(int)m_itemArray.size()-1);
                }
                else
                {
                    iter->ulSize = ulSize;
                    RedrawItems(nIndex,nIndex);
                }
                    
                break;
            }
        }

    }
   
  
    void InitListCtrl();
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
    {
        lpMeasureItemStruct->itemHeight  = 34;
    }
    void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
    {
       
    }

    int DeleteItem(int nItem)
    {
        __super::DeleteItem(nItem);
        return 0;
    }

    BEGIN_MSG_MAP(CDirectoryListCtrl)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_NCPAINT(OnNcPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MESSAGE_HANDLER(LVM_HITTEST, OnHitTest)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KEYUP(OnKeyUp)
        MSG_WM_CHAR(OnKey)
        CHAIN_MSG_MAP_ALT(COwnerDraw<CDirectoryListCtrl>, 1)
    END_MSG_MAP()



private:

    CListHeader m_wndHeaderCtrl;
    HIMAGELIST  m_hImageList;
    ULONGLONG   m_currentTSize;
    int         m_nCurrentIndex;

protected:
	
     LRESULT OnHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	 void OnPaint(HDC rgn);
     void OnNcPaint(HRGN rgn);
	 void OnTimer(UINT nIDEvent);
	 void OnLButtonDown(UINT nFlags, CPoint point);
	 void OnDestroy();
	 void OnRButtonDown(UINT nFlags, CPoint point);
     LRESULT OnEraseBkgnd(HDC pdc)
     {
         
          return 1;
     }

     LRESULT OnKeyUp( WPARAM wParam, LPARAM lParam, UINT uMsg)
     {
         SetMsgHandled(TRUE);
         return 1;
     }

     LRESULT OnKey( WPARAM wParam, LPARAM lParam, UINT uMsg)
     {
         SetMsgHandled(TRUE);
         return 1;

     }

     LRESULT OnKeyDown( WPARAM wParam, LPARAM lParam, UINT uMsg)
     {
         SetMsgHandled(TRUE);
         return 1;

     }

     

};

LPCTSTR GetFileSizeText(__int64 fNum);
