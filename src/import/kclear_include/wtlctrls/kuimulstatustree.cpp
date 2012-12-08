
#include "stdafx.h"
#include "resource.h"
#include ".\kuimulstatustree.h"
#include <shellapi.h>

#undef __IStream_INTERFACE_DEFINED__

// #include "atlapp.h"
// #include "atlctrls.h"
// #include "atlctrlx.h"

KUIMulStatusTree::KUIMulStatusTree(void) :
m_uClickFlags(0), m_ICO_ID(MAKEINTRESOURCE(IDB_FOLDER_CHECKFLAG_BMP)), 
m_bEnableCheck(TRUE)
{

}

KUIMulStatusTree::~KUIMulStatusTree(void)
{

}


LRESULT KUIMulStatusTree::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lResult = DefWindowProc(uMsg, wParam, lParam);

    _Init();

    return 0;
}

BOOL KUIMulStatusTree::SubclassWindow(HWND hWnd)
{
    ATLASSERT(m_hWnd == NULL);
    ATLASSERT(::IsWindow(hWnd));

    BOOL bRet = ATL::CWindowImpl<KUIMulStatusTree, CTreeViewCtrl>::SubclassWindow(hWnd);
    if(bRet)
    {
        _Init();
    }

    return bRet;
}

HTREEITEM KUIMulStatusTree::InsertItem(LPCTSTR lpszItem, int nImage,
	 int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter,  int nCheck)
{
	HTREEITEM hResult = CTreeViewCtrl::InsertItem(lpszItem, nImage, nSelectedImage, hParent, hInsertAfter);
	
	SetCheck(hResult, nCheck);

	return hResult;
}


HTREEITEM KUIMulStatusTree::InsertItem(LPCTSTR lpszItem, HTREEITEM hParent, HTREEITEM hInsertAfter,  int nCheck)
{
	HTREEITEM hResult = CTreeViewCtrl::InsertItem(lpszItem, hParent, hInsertAfter);

	SetCheck(hResult, nCheck);

	return hResult;
}


BOOL KUIMulStatusTree::GetCheck(HTREEITEM hItem, int& nCheck)
{
    ATLASSERT(m_hWnd != NULL);

    TV_ITEM tvi;
    ZeroMemory(&tvi, sizeof(tvi));

    tvi.hItem = hItem;
    tvi.mask = TVIF_STATE;
    tvi.stateMask = TVIS_STATEIMAGEMASK;
    if (!TreeView_GetItem(m_hWnd, &tvi))
        return FALSE;

    nCheck = tvi.state >> 12;

    if (nCheck < 0)
        return FALSE;

    ATLASSERT(nCheck >= EM_TVIS_NONE && nCheck <= EM_TVIS_CHECK);

    return TRUE;
}

BOOL KUIMulStatusTree::SetCheck(
    HTREEITEM hItem, 
    int  nCheck, 
    BOOL bUpdate /* = TRUE */)
{
    ATLASSERT(hItem  != NULL);
    ATLASSERT(m_hWnd != NULL);

    TV_ITEM tvi;
    ZeroMemory(&tvi, sizeof(tvi));

    tvi.hItem = hItem;
    tvi.mask  = TVIF_STATE;
    tvi.stateMask = TVIS_STATEIMAGEMASK;

    ATLASSERT((nCheck >= EM_TVIS_NONE) && (nCheck <= EM_TVIS_CHECK));

    tvi.state = INDEXTOSTATEIMAGEMASK(nCheck);

    if (!TreeView_SetItem(m_hWnd, &tvi))
        return FALSE;

    if (nCheck != EM_TVIS_NONE)
    {    
        if (bUpdate)
            TravelChild(hItem, nCheck);

        TravelSiblingAndParent(hItem, nCheck);
    } // if (nCheck != EM_TVIS_NONE)    

    return TRUE;
}


void KUIMulStatusTree::_Init()
{
    const int		nicx = 16;
    const int		nicy = 16;
    const COLORREF	clrMask = RGB(255, 0, 255);
	
	// 设置系统图像列表
    WTL::CBitmap	bmCheck;
	bmCheck.LoadBitmap( m_ICO_ID );
	m_stateImageList.Create( nicx, nicy, ILC_COLOR32 | ILC_MASK, 4, 1 );
	m_stateImageList.Add( bmCheck, RGB( 255, 0, 255 ) );

    //m_stateImageList.Create(IDB_FOLDER_CHECKFLAG_BMP, nicx, 1, clrMask);
	ModifyStyle( 0, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT );

    ATLASSERT(m_stateImageList.m_hImageList != NULL);
    SetImageList(m_stateImageList, TVSIL_STATE);        
}


LRESULT KUIMulStatusTree::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DefWindowProc();

	if( !m_stateImageList.IsNull() )
	{
		m_stateImageList.Destroy();
	}

	return 0;
}


LRESULT KUIMulStatusTree::OnNMRClick(int, LPNMHDR pnmh, BOOL&)
{
    POINT pt = { 0, 0 };
    ::GetCursorPos(&pt);
    POINT ptClient = pt;
    if(pnmh->hwndFrom != NULL)
        ::ScreenToClient(pnmh->hwndFrom, &ptClient);

    if(pnmh->hwndFrom == m_hWnd)
    {
        TVHITTESTINFO tvhti = { 0 };
        tvhti.pt = ptClient;
        HitTest(&tvhti);
        if ((tvhti.flags & TVHT_ONITEMLABEL) != 0)
        {
            TVITEM tvi = { 0 };
            tvi.mask = TVIF_PARAM;
            tvi.hItem = tvhti.hItem;
            if (GetItem(&tvi) != FALSE)
            {
                //LPTVITEMDATA lptvid = (LPTVITEMDATA)tvi.lParam;
                //if (lptvid != NULL)
                    //m_ShellMgr.DoContextMenu(::GetParent(m_hWnd), lptvid->spParentFolder, lptvid->lpi, pt);
            }
        }
    }

    return 0L;
}


LRESULT KUIMulStatusTree::OnStateIconClick(int nCode, LPNMHDR pnmh, BOOL& bHandled) 
{
    if (m_uClickFlags & TVHT_ONITEMSTATEICON) 
        bHandled = TRUE;
    else
        bHandled = FALSE;

    return bHandled;
}


LRESULT KUIMulStatusTree::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == 0x20) //处理空格键
    {
        HTREEITEM hItem = GetSelectedItem();

		if ( !m_bEnableCheck )
			return TRUE;

        int nCheck = 0;

        GetCheck(hItem, nCheck);
        if(nCheck != 0)
        {
            nCheck = // 取反
                (nCheck == EM_TVIS_CHECK) ? EM_TVIS_UNCHECK : EM_TVIS_CHECK;

            SetCheck(hItem, nCheck);
        }

    }
    else bHandled = FALSE;

    return TRUE;
}


LRESULT KUIMulStatusTree::OnTvnKeydown(int nCode, LPNMHDR pnmh, BOOL& bHandled)
{
    LPNMTVKEYDOWN ptvkd = (LPNMTVKEYDOWN)pnmh;

    if (ptvkd->wVKey == 32)
    {
        bHandled = TRUE;
    }
    else
        bHandled = FALSE;

    bHandled = TRUE;

    return FALSE;
}


LRESULT KUIMulStatusTree::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int nRetCode = 0;

    POINT pt = {LOWORD(lParam), HIWORD(lParam)};

    HTREEITEM hTreeItem = HitTest(pt, &m_uClickFlags);

	bHandled = TRUE;
	

    if (hTreeItem != NULL && m_uClickFlags & TVHT_ONITEMSTATEICON)
    {
        int nCheck = FALSE;

		if ( !m_bEnableCheck )
			 goto Exit0;

        nRetCode = GetCheck(hTreeItem, nCheck);

        if (!nRetCode)
            goto Exit0;

        nCheck = // 取反
            (nCheck == EM_TVIS_CHECK) 
            ? EM_TVIS_UNCHECK 
            : EM_TVIS_CHECK;

        SetCheck(hTreeItem, nCheck);

        HWND hParent = GetParent();
        if (hParent != NULL)
        {
            ::SendMessage(hParent, WM_KAN_NOTIFY_TREE_CHECKED_CHANGE, (WPARAM)hTreeItem, (LPARAM)nCheck);
        }

		SelectItem(hTreeItem);


    } // if (hTreeItem != NULL && m_uClickFlags & TVIS_STATEIMAGEMASK)
	
	if ( m_uClickFlags & TVHT_ONITEMSTATEICON )
	{
	}
	else if(hTreeItem != NULL && m_uClickFlags & TVIS_CUT)
    {
        //::SendMessage(GetParent(),WM_KAN_NOTIFY_TREE_SELECT_CHANGE,(WPARAM)hTreeItem,0);
        DefWindowProc();
    }
    else
    {
        DefWindowProc();
    }
    
    
Exit0:
    


    return TRUE;
}


LRESULT KUIMulStatusTree::OnLDBCLK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int nRetCode = 0;

    POINT pt = {LOWORD(lParam), HIWORD(lParam)};

    HTREEITEM hTreeItem = HitTest(pt, &m_uClickFlags);

    bHandled = TRUE;


    

    if ( m_uClickFlags & TVHT_ONITEMSTATEICON )
    {
    }
    else if(hTreeItem != NULL && m_uClickFlags & TVIS_CUT)
    {
        ::SendMessage(GetParent(),WM_KAN_NOTIFY_TREE_SELECT_CHANGE,(WPARAM)hTreeItem,0);
        DefWindowProc();
    }
    else
    {
        DefWindowProc();
    }





    return TRUE;
}


HTREEITEM KUIMulStatusTree::GetFirstCheck(HTREEITEM hFirstCheck /* = NULL */)
{
    int nRetCode;
    int nCheckMode;

    HTREEITEM hItem = (hFirstCheck != NULL) ? hFirstCheck : 
    (GetNextItem(NULL, TVGN_ROOT));

    while (hItem != NULL)
    {
        nRetCode = GetCheck(hItem, nCheckMode);
        ATLASSERT(nRetCode);

        switch (nCheckMode)
        {
        case EM_TVIS_CHECK:
            return hItem;
        case EM_TVIS_INDETERMINING:
            hFirstCheck = GetNextItem(hItem, TVGN_CHILD);
            if (hFirstCheck != NULL)
            {
                hItem = hFirstCheck;
                break;
            }

        default:
            do
            {
                hFirstCheck = GetNextItem(hItem, TVGN_NEXT);
                if (hFirstCheck != NULL)
                {
                    hItem = hFirstCheck;
                    break;
                }
                hFirstCheck = GetNextItem(hItem, TVGN_PARENT);
                if (NULL == hFirstCheck)
                    return NULL;

                hItem = hFirstCheck;
            } while (hItem != NULL);

            break;
        }
    }

    return NULL;

}

HTREEITEM KUIMulStatusTree::GetNextCheck(HTREEITEM hPrevCheck)
{
    int nRetCode;
    int nCheckMode;

    HTREEITEM hChild = NULL;
    HTREEITEM hItem  = hPrevCheck;

    while (hItem != NULL)
    {
        if (hChild != NULL)
            hChild = NULL;
        else
        {
            do
            {
                hPrevCheck = GetNextItem(hItem, TVGN_NEXT);
                if (hPrevCheck != NULL)
                {
                    hItem = hPrevCheck;
                    break;
                } // if (hPrevCheck != NULL)

                hPrevCheck = GetNextItem(hItem, TVGN_PARENT);
                if (NULL == hPrevCheck)
                    return NULL;

                hItem = hPrevCheck;

            } while (hItem != NULL);
        }


        nRetCode = GetCheck(hItem, nCheckMode);
        ATLASSERT(nRetCode);

        switch (nCheckMode)
        {
        case EM_TVIS_CHECK:
            return hItem;

        case EM_TVIS_INDETERMINING:
            hChild = GetNextItem(hItem, TVGN_CHILD);
            if (hChild != NULL)
            {
                hItem = hChild;
                break;
            }
        default:
            break;
        }
    }


    return NULL;
}

void KUIMulStatusTree::TravelChild(HTREEITEM hItem, int nCheck)
{
    HTREEITEM hChildItem   = NULL;
    HTREEITEM hBrotherItem = NULL;;
    HWND hParent = GetParent();
    UINT nState  = 0;


    //查找子节点，没有就结束
    hChildItem = GetChildItem(hItem);

    int nChildCheck = nCheck;
    GetCheck(hChildItem,nChildCheck);

    if(hChildItem != NULL && nChildCheck != KUIMulStatusTree::EM_TVIS_NONE)
    {
        //设置子节点的状态与当前节点的状态一致
        nState = GetItemState(hChildItem, TVIS_STATEIMAGEMASK);
        if (nState != INDEXTOSTATEIMAGEMASK(nCheck))
        {        
            SetItemState(hChildItem, INDEXTOSTATEIMAGEMASK(nCheck), TVIS_STATEIMAGEMASK);
//             if (hParent != NULL)
//             {
//                 ::SendMessage(hParent, WM_KAN_NOTIFY_TREE_CHECKED_CHANGE, (WPARAM)hChildItem, (LPARAM)nCheck);
//             }
        }

        //再递归处理子节点的子节点和兄弟节点
        TravelChild(hChildItem, nCheck);

        //处理子节点的兄弟节点和其子节点
        hBrotherItem = GetNextSiblingItem(hChildItem);

        while (hBrotherItem)
        {
            //设置子节点的兄弟节点状态与当前节点的状态一致
            nState = GetItemState(hBrotherItem, TVIS_STATEIMAGEMASK);
            int nBrotherCheck = nCheck;
            GetCheck(hBrotherItem,nBrotherCheck);
            if (nState != INDEXTOSTATEIMAGEMASK(nCheck) && nBrotherCheck != KUIMulStatusTree::EM_TVIS_NONE)
            {        
                SetItemState(hBrotherItem, INDEXTOSTATEIMAGEMASK(nCheck), TVIS_STATEIMAGEMASK);
//                 if (hParent != NULL)
//                 {
//                     ::SendMessage(hParent, WM_KAN_NOTIFY_TREE_CHECKED_CHANGE, (WPARAM)hBrotherItem, (LPARAM)nCheck);
//                 }
            }

            //再递归处理子节点的兄弟节点的子节点和兄弟节点
            TravelChild(hBrotherItem, nCheck);

            hBrotherItem = GetNextSiblingItem(hBrotherItem);
        }
    }
}

void KUIMulStatusTree::TravelSiblingAndParent(HTREEITEM hItem, int nCheck)
{
    HTREEITEM hNextSiblingItem = NULL;
    HTREEITEM hPrevSiblingItem = NULL;
    HTREEITEM hParentItem = NULL;

    HWND hParent = GetParent();
    UINT nState  = 0;

    //查找父节点，没有就结束
    hParentItem = GetParentItem(hItem);

    int nRetCode = false;

    if(hParentItem != NULL)
    {
        int nState = nCheck;//设初始值，防止没有兄弟节点时出错

        //查找当前节点下面的兄弟节点的状态
        hNextSiblingItem = GetNextSiblingItem(hItem);

        while(hNextSiblingItem != NULL)
        {
            nRetCode = GetCheck(hNextSiblingItem, nState);

            if(nState != nCheck && nRetCode)
                break;
            else 
                hNextSiblingItem = GetNextSiblingItem(hNextSiblingItem);
        }

        if( nCheck == nState)
        {
            //查找当前节点上面的兄弟节点的状态
            hPrevSiblingItem = GetPrevSiblingItem(hItem);

            while(hPrevSiblingItem != NULL)
            {
                nRetCode = GetCheck(hPrevSiblingItem, nState);

                if(nState != nCheck && nRetCode)
                    break;
                else 
                    hPrevSiblingItem = GetPrevSiblingItem(hPrevSiblingItem);
            }
        }

        if( nCheck == nState || nState == 0)
        {
            nRetCode = GetCheck( hParentItem, nState);

            if( nState != 0)
            {
                //如果状态一致，则父节点的状态与当前节点的状态一致
                nState = GetItemState(hParentItem, TVIS_STATEIMAGEMASK);
                if (nState != INDEXTOSTATEIMAGEMASK(nCheck))
                {        
                    SetItemState(hParentItem, INDEXTOSTATEIMAGEMASK(nCheck), TVIS_STATEIMAGEMASK);
                    if (hParent != NULL)
                    {
                       // ::SendMessage(hParent, WM_KAN_NOTIFY_TREE_CHECKED_CHANGE, (WPARAM)hParentItem, (LPARAM)nCheck);
                    }
                }

            }
            //再递归处理父节点的兄弟节点和其父节点

            TravelSiblingAndParent(hParentItem, nCheck);
        }
        else
        {
            //状态不一致，则当前节点的父节点、父节点的父节点……状态均为第三态
            hParentItem = GetParentItem(hItem);

            while(hParentItem != NULL)
            {
                nRetCode = GetCheck(hParentItem, nState);

                if(nState !=0)
                {
                    nState = GetItemState(hParentItem, TVIS_STATEIMAGEMASK);
                    SetItemState(hParentItem, INDEXTOSTATEIMAGEMASK(EM_TVIS_INDETERMINING), TVIS_STATEIMAGEMASK);
                    if (hParent != NULL)
                    {
                        //::SendMessage(hParent, WM_KAN_NOTIFY_TREE_CHECKED_CHANGE, (WPARAM)hParentItem, (LPARAM)EM_TVIS_INDETERMINING);
                    }
                }

                hParentItem = GetParentItem(hParentItem);
            }
        }
    }	
}

